/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"

extern Scope file_scp;

static int trace_ast_down_up_gen_tac(vector<Tac> &tacs, Ast *p)
{
	if (!p || p->sem_stamp == SEM_VAR_DECLARE)
		return -1;

	int b = trace_ast_down_up_gen_tac(tacs, p->right);
	int a = trace_ast_down_up_gen_tac(tacs, p->left);
	Tac t;

	switch (p->sem_stamp)
	{
	// leaf node
	case SEM_VAR:
		assert(p->symb);
		return p->vr;

	case SEM_IMMEDIATE:
		assert(p->symb);
		return p->vr;

	case SEM_OPERATOR:
		if (p->op == OP_LOGIC_AND || p->op == OP_LOGIC_OR)
			ERR("op: %d", p->op);

		if (p->op == OP_ASSIGN)
		{
			if (a == b) // not put into tac
				return a;
			// x = y : return x
			t = Tac(p);
			t.dst = a;
			t.s1 = b;
			tacs.push_back(t);
			return a;
		}
		if (p->op == OP_SAVE_RET_VALUE)
		{
			t = Tac(p);
			t.s1 = a;
			tacs.push_back(t);
			break;
		}
		if (p->op == OP_TEST)
		{
			t = Tac(p);
			t.s1 = a;
			tacs.push_back(t);
			return -1;
		}

		t = Tac(p);
		t.dst = p->vr;
		t.s1 = a;
		t.s2 = b;
		tacs.push_back(t);
		return p->vr;

//	case SEM_SAVE_RET_VALUE:
//		t = Tac(p);
//		t.s1 = a;
//		tacs.push_back(t);
//		break;

	case SEM_VAR_DECLARE:
		return -1;

		// todo
	case SEM_FUNC_DECLARE:
		case SEM_FUNC_DEFINE:
		case SEM_FUNC_CALL:
		printf("todo sem_func* semty %d \n", p->sem_stamp);
		return -1;

	default:
		ERR("%d \n", p->sem_stamp);
		break;
	}

	return -1;
}

static void replace_ssa(Scope *scp, int a, int b)
{
	LOG();
	vector<Tac> &tacs = scp->bb->tacO0;
	for(auto &r : tacs)
	{
		if(r.dst == a) r.dst = b;
		if(r.s1 == a) r.s1 = b;
		if(r.s2 == a) r.s2 = b;
	}

	for (Scope *p : scp->clds)
		replace_ssa(p, a, b);
}
static void insert_copy_ssa(Scope *scp, Scope *tail)
{
	vector<Symbol*> &outer_symb_table = scp->outer_symb_table;
	for (int i = 0; i < outer_symb_table.size(); i++)
	{
		Symbol *symb = outer_symb_table[i];
#if 1
		Tac t;
		t.sem_stamp = SEM_OPERATOR;
		t.op = OP_COPY_SSA;
		t.dst = symb->out_base_vr;
		t.s1 = symb->ssa.back();

		LOG("new_ssa_scp %s", symb->ssa_scp.back()->name.c_str());
		vector<Tac> &tacs = tail->bb->tacO0;
		tacs.push_back(t);
#else
		int a = symb->ssa.back();
		int b = symb->out_base_vr;
		replace_ssa(scp, a, b);
#endif
	}
}

static bool gen_tac_O0(Scope *scp)
{
	assert(scp->bb);

//	LOG("scp %s", scp->name.c_str());
	if (scp->sem_stamp == SEM_bb_terminate)	// TODO if support goto, use jmp_in.size() == 0 ?
	{
//		insert_copy_ssa(scp->temp_outer_symb_table);

		printf("%s have a SEM_bb_terminate scope %s\n", scp->parent->name.c_str(), scp->name.c_str());
		return true;
	}

	for (auto *p : scp->asts)
		trace_ast_down_up_gen_tac(scp->bb->tacO0, p);

	bool bb_terminated = 0;
	for (Scope *p : scp->clds)
	{
		bb_terminated = gen_tac_O0(p);
		if (bb_terminated)
			break;
	}

	if (bb_terminated && scp->sem_stamp == SEM_INVALID)
	{
		//scp->jmp_in.size() == 0
//			scp->sem_stamp != SEM_COND_JMP
//			&& scp->sem_stamp != SEM_WHILE_BODY
//			&& scp->sem_stamp != SEM_JMP
		return true;
	}

	if (!scp->is_virtual && scp->depth > 1)
	{
		LOG("scp %s %lu", scp->name.c_str(), scp->outer_symb_table.size());
		Scope *tail = get_inner_tail_of_scope(scp);
		insert_copy_ssa(scp, tail);
	}

	return false;
}

static int _tac_len = 0;
#define PRINT_TAC(fmt, ...) _tac_len = printf(fmt, ##__VA_ARGS__)
#define PRINT_TAC_SEM(...) \
	do { \
	printf("%*s", (28 - _tac_len) > 1 ? (28 - _tac_len) : 2, ""); \
	printf(__VA_ARGS__); \
	} while (0)

static void dump_tac(Scope *scp, Optimize_level level)
{
	assert(scp->bb != 0);
	BasicBlock *bb = scp->bb;
	vector<Tac> *tac = 0;

	if (level == O0)
		tac = &bb->tacO0;
	else if (level == O1)
		tac = &bb->tacO1;
	else
		ERR();

	if (scp->sem_stamp == SEM_FUNC_DEFINE
	    || scp->sem_stamp == SEM_IF_cond
	    || scp->sem_stamp == SEM_IF_then
	    || scp->sem_stamp == SEM_IF_else
	    || scp->sem_stamp == SEM_WHILE_COND
	    || scp->sem_stamp == SEM_WHILE_BODY
	    || scp->sem_stamp == SEM_CONTINUE
	    || scp->sem_stamp == SEM_BREAK
	    || (scp->sem_stamp == SEM_UNNAMED_SCOPE && tac->size())
	    || scp->jmp_in.size() > 0
	    || scp->asts.size() > 0)
	{
		printf("\n%s: \n", scp->name.c_str());
	}

	for (const Tac &r : *tac)
	{
		Ast *p = r.ast;

		switch (r.sem_stamp)
		{
		case SEM_OPERATOR:
			if (r.op == OP_ASSIGN)
			{
				PRINT_TAC("%%%d %s %%%d", r.dst, p->tk.src.c_str(), r.s1);
				PRINT_TAC_SEM("#%s %s %s \n", vr_name(r.dst), p->tk.src.c_str(), vr_name(r.s1));
			}
			else if (r.op == OP_COPY_SSA)
			{
				PRINT_TAC("%%%d = %%%d", r.dst, r.s1);
				PRINT_TAC_SEM("#cp_ssa %s = %s \n", vr_name(r.dst), vr_name(r.s1));
			}
			else if (r.op == OP_SAVE_RET_VALUE)
			{
				PRINT_TAC("save_ret: %s %%%d", p->tk.src.c_str(), r.s1);
				printf("\n");
			}
			else if (r.op == OP_TEST)
			{
				PRINT_TAC("test %s %%%d", vr_name(r.s1), r.s1);
				printf("\n");
			}
			else
			{
				PRINT_TAC("%%%d = %%%d %s %%%d", r.dst, r.s1, p->tk.src.c_str(), r.s2);
				PRINT_TAC_SEM("#%s = %s %s %s \n",
				    r.dst >= 0 ? vr_name(r.dst) : "-1",
				    r.s1 >= 0 ? vr_name(r.s1) : "-1",
				    p->tk.src.c_str(),
				    r.s2 >= 0 ? vr_name(r.s2) : "-1");
			}
			break;

		case SEM_FUNC_CALL:
			PRINT_TAC("func call: %s", p->tk.src.c_str());
			printf("\n");
			break;

		case SEM_IMMEDIATE:
		case SEM_VAR_DECLARE:
		case SEM_VAR:
		default:
			ERR();
			break;
		}
	}

	if (scp->jmp_out != 0)
		printf("->%s:\n\n", scp->jmp_out->name.c_str());

	for (Scope *p : scp->clds)
		dump_tac(p, level);
}

static vector<int> vr_union;
static vector<int> target_vr_unioned;

static int find(int a)
{
	if (a < 0)
		return a;

	if (vr_union[a] == a)
		return a;

	int root = find(vr_union[a]);
	vr_union[a] = root;
	return root;
}
static void u(int a, int b)
{
	// b is target, each target vr can be unioned at most once
	if (target_vr_unioned[b] > 0)
		return;

	int root_a = find(a);
	int root_b = find(b);
	if (root_a != root_b)
	{
		vr_union[root_a] = root_b;
		target_vr_unioned[b]++;
	}
}
static void gen_tac_O1(Scope *scp)
{
	BasicBlock *bb = scp->bb;
	bb->tacO1 = bb->tacO0;

	for (Scope *p : scp->clds)
		gen_tac_O1(p);
}
static void union_transient_vr(Scope *scp)
{
	vector<Tac> &tac = scp->bb->tacO1;

	vr_union.clear();
	vr_union.resize(vr_size, -1);
	target_vr_unioned.clear();
	target_vr_unioned.resize(vr_size, 0);

#define VR_UNION(a) \
	if(a >= 0) vr_union[a] = a

	for (int i = 0; i < tac.size(); i++)
	{
		const Tac &t = tac[i];
		VR_UNION(t.dst);
		VR_UNION(t.s1);
		VR_UNION(t.s2);
	}

	for (int i = 0; i < tac.size(); i++)
	{
		int a = tac[i].dst;
		if (a < 0 || vr_symb(a)->stamp != SYMB_PRIVATE_transient)
		continue;

		// a transient that already received a merge (was a target) cannot be
		// merged further: this breaks union chains like %10->%11->%2 where the
		// final target %2 would be written at %10's def point and clobbered
		// before its original value is consumed.
		if (target_vr_unioned[a] > 0)
			continue;

		for (int k = i + 1; k < tac.size(); k++)
		{
			int dst = tac[k].dst;
			if (dst < 0)
				continue;

			int s1 = tac[k].s1;
			int s2 = tac[k].s2;

			int root_a = find(a);
			if (find(s1) != root_a && find(s2) != root_a)
				continue;

			// a is first used at instruction k; try to merge a into dst.
			// For correctness, dst must not be live (used as a source) between
			// a's def (i) and a's use (k), because after the merge a's def
			// writes dst, clobbering any original dst value still needed.
			int root_dst = find(dst);
			bool dst_live = false;
			for (int m = i + 1; m <= k && !dst_live; m++)
			{
				int ms1 = tac[m].s1;
				int ms2 = tac[m].s2;
				if (m == k)
				{
					// at the use instruction, the operand slot holding a is
					// conceptually replaced by dst, so it does not count as a
					// use of dst's original value.
					if (ms1 >= 0 && find(ms1) != root_a && find(ms1) == root_dst)
						dst_live = true;
					if (ms2 >= 0 && find(ms2) != root_a && find(ms2) == root_dst)
						dst_live = true;
				}
				else
				{
					if (ms1 >= 0 && find(ms1) == root_dst)
						dst_live = true;
					if (ms2 >= 0 && find(ms2) == root_dst)
						dst_live = true;
				}
			}
			if (dst_live)
				break;

			u(a, dst);
			break;
		}
	}

#define VR_FIND(a) \
	if(a >= 0) a = find(a)

	for (int i = 0; i < tac.size(); i++)
	{
		Tac &t = tac[i];
		VR_FIND(t.dst);
		VR_FIND(t.s1);
		VR_FIND(t.s2);
	}

	for (int i = 0; i < tac.size();)
	{
		Tac &t = tac[i];
		if ((t.op == OP_ASSIGN || t.op == OP_COPY_SSA) && t.dst == t.s1)
		{
			tac.erase(tac.begin() + i);
			continue;
		}

//		if (t.ast->op == OP_SUB && t.s1 == t.s2)
//		{
//			t. li_mc(MC_LI, mc.dst);
//			li_mc.pr1 = mc.pr_dst;
//			li_mc.const_num = 0;
//							li_mc.ori_sem = "li 0 (s1-s2)";
//		}

		i++;
	}

	for (Scope *p : scp->clds)
		union_transient_vr(p);
}
static void copy_coalesce(Scope *scp)
{
}
static void copy_propagation(Scope *scp)
{
	vector<Tac> &tac = scp->bb->tacO1;

	for (int i = tac.size() - 1; i >= 0; i--)
	{
		Tac &t = tac[i];
		if (t.op == OP_ASSIGN)
		{
			t.op = OP_ALL;
			int s1 = t.s1;
			int dst = t.dst;
			for (int k = i; k < tac.size(); k++)
			{

			}
		}

	}

	for (int i = 0; i < tac.size(); i++)
	{
		int a = tac[i].dst;
		if (a < 0 || vr_symb(a)->stamp != SYMB_PRIVATE_transient)
		continue;

		for (int k = i + 1; k < tac.size(); k++)
		{
			int dst = tac[k].dst;
			if (dst < 0)
				continue;

		}
	}

	for (Scope *p : scp->clds)
		copy_propagation(p);
}
static void elimicate_transient_vr(Scope *scp)
{
	vector<Tac> &tac = scp->bb->tacO1;

	for (int i = 0; i < tac.size();)
	{
		if (i + 1 >= tac.size())
			break;

		Tac &a = tac[i];
		Tac &b = tac[i + 1];

		if (a.dst < 0)
		{
			i++;
			continue;
		}

		if (vr_symb(a.dst)->stamp == SYMB_PRIVATE_transient
		&& b.op == OP_ASSIGN
		&& b.s1 == a.dst)
		{
			a.dst = b.dst;
			tac.erase(tac.begin() + i + 1);
			continue;
		}

		i++;
	}

	for (Scope *p : scp->clds)
		elimicate_transient_vr(p);
}
static void rm_dead_code(Scope *scp)
{
	for (int i = scp->clds.size() - 1; i >= 0; i--)
		rm_dead_code(scp->clds[i]);

	vector<Tac> &tacs = scp->bb->tacO1;

	for (int i = tacs.size() - 1; i >= 0; i--)
	{
		auto &tac = tacs[i];
		if (tac.dst < 0 || vr_use_cnt[tac.dst] > 0)
			continue;

		Operator op = tacs[i].op;
		if (op == OP_CMP_E
		    || op == OP_CMP_NE
		    || op == OP_CMP_L
		    || op == OP_CMP_LE
		    || op == OP_CMP_G
		    || op == OP_CMP_GE
		    || op == OP_TEST)
		{
			continue;
		}

		if (tac.s1 >= 0)
		{
//			declare_record.ast[mc.s1]->symb->use_cnt--;
			vr_use_cnt[tac.s1]--;
			scp->use_cnt_in_bb[tac.s1]--;
		}
		if (tac.s2 >= 0)
		{
//			declare_record.ast[mc.s2]->symb->use_cnt--;
			vr_use_cnt[tac.s2]--;
			scp->use_cnt_in_bb[tac.s2]--;
		}

		LOG("rm %s %%%d", vr_name(tac.dst), tac.dst);
		tacs.erase(tacs.begin() + i);
	}
}
static void collect_add_operands(vector<Tac> &tacs, vector<int> &def_idx, int vr,
    vector<int> &ops, vector<int> &idxs)
{
	int di = def_idx[vr];
	if (di < 0)
	{
		ops.push_back(vr);
		return;
	}
	Tac &t = tacs[di];
	if (t.op != OP_ADD || vr_symb(vr)->stamp != SYMB_PRIVATE_transient)
	{
		ops.push_back(vr);
		return;
	}
	idxs.push_back(di);

	if (vr_use_cnt[t.s1] == 1 && vr_symb(t.s1)->stamp == SYMB_PRIVATE_transient)
	collect_add_operands(tacs, def_idx, t.s1, ops, idxs);
	else
	ops.push_back(t.s1);

	if (vr_use_cnt[t.s2] == 1 && vr_symb(t.s2)->stamp == SYMB_PRIVATE_transient)
	collect_add_operands(tacs, def_idx, t.s2, ops, idxs);
	else
	ops.push_back(t.s2);
}
static void reassociate(Scope *scp)
{
	vector<Tac> &tacs = scp->bb->tacO1;

	if (tacs.size() > 0)
	{
		vector<int> def_idx(vr_size, -1);
		for (int i = 0; i < tacs.size(); i++)
			if (tacs[i].dst >= 0)
				def_idx[tacs[i].dst] = i;

		vector<int> use_in_add(vr_size, 0);
		for (auto &t : tacs)
		{
			if (t.op != OP_ADD || vr_symb(t.dst)->stamp != SYMB_PRIVATE_transient)
			continue;
			if (t.s1 >= 0 && vr_symb(t.s1)->stamp == SYMB_PRIVATE_transient)
			use_in_add[t.s1]++;
			if (t.s2 >= 0 && vr_symb(t.s2)->stamp == SYMB_PRIVATE_transient)
			use_in_add[t.s2]++;
		}

		for (int i = tacs.size() - 1; i >= 0; i--)
		{
			Tac &t = tacs[i];
			if (t.op != OP_ADD)
				continue;
			if (vr_symb(t.dst)->stamp != SYMB_PRIVATE_transient)
			continue;
			if (use_in_add[t.dst] > 0)
				continue;

			vector<int> ops;
			vector<int> idxs;
			collect_add_operands(tacs, def_idx, t.dst, ops, idxs);

			if (ops.size() <= 2)
				continue;

			bool all_user = true;
			for (int o : ops)
				if (vr_symb(o)->stamp == SYMB_PRIVATE_transient)
				{
					all_user = false;
					break;
				}
			if (all_user)
			{
				int max_cnt = 0;
				for (int o : ops)
					max_cnt = std::max(max_cnt, vr_use_cnt[o]);
				std::sort(ops.begin(), ops.end(), [&](int a, int b)
				{
					int ca = vr_use_cnt[a], cb = vr_use_cnt[b];
					if (ca != cb)
					return ca > cb;
					bool hi = (ca == max_cnt);
					return hi ? (a < b) : (a > b);
				});
			}
			else
			{
				std::stable_sort(ops.begin(), ops.end(), [&](int a, int b)
				{
					bool ta = vr_symb(a)->stamp == SYMB_PRIVATE_transient;
					bool tb = vr_symb(b)->stamp == SYMB_PRIVATE_transient;
					return ta && !tb;
				});
			}

			int acc = ops[0];
			for (int k = idxs.size() - 1; k >= 0; k--)
			{
				Tac &instr = tacs[idxs[k]];
				instr.s1 = acc;
				instr.s2 = ops[idxs.size() - k];
				acc = instr.dst;
			}
		}
	}

	for (Scope *p : scp->clds)
		reassociate(p);
}

void gen_three_address_code_O0()
{
	gen_tac_O0(&file_scp);

	printf("\n========== tac O0 ==========\n");
	dump_tac(&file_scp, O0);
}
void gen_three_address_code_O1()
{
	gen_tac_O1(&file_scp);
//	union_transient_vr(&file_scp);

	elimicate_transient_vr(&file_scp);
//	copy_propagation(&file_scp);
	gen_use_cnt();
	rm_dead_code(&file_scp);
//	reassociate(&file_scp);

	gen_use_cnt();

	printf("\n========== tac O1==========\n");
	dump_tac(&file_scp, O1);
}
