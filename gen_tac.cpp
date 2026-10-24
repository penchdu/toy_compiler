/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"

extern Scope file_scp;

static int trace_ast_down_up_gen_3_address_code(vector<Tac> &tacs, Ast *p)
{
	if (!p || p->sem_stamp == SEM_VAR_DECLARE)
		return -1;

	int b = trace_ast_down_up_gen_3_address_code(tacs, p->right);
	int a = trace_ast_down_up_gen_3_address_code(tacs, p->left);
	Tac t;

	switch (p->sem_stamp)
	{
	// leaf node
	case SEM_VAR:
		case SEM_FUNC_ARG:
		assert(p->symb);

		// todo symb->vr to be defined in "new =" to gen ssa
		return p->symb->vr;

	case SEM_IMMEDIATE:
		assert(p->symb);
		return p->symb->vr;

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

	case SEM_SAVE_RET_VALUE:
		t = Tac(p);
		t.s1 = a;
		tacs.push_back(t);
		break;

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

static bool gen_tac(Scope *scp)
{
	assert(scp->bb);

//	LOG("scp %s", scp->name.c_str());
	if (scp->sem_stamp == SEM_bb_terminate)
	{
		printf("%s have a SEM_bb_terminate scope %s\n", scp->parent->name.c_str(), scp->name.c_str());
		return true;
	}

	for (auto *p : scp->asts)
		trace_ast_down_up_gen_3_address_code(scp->bb->tacO0, p);

	bool bb_terminated = 0;
	for (Scope *p : scp->clds)
	{
		bb_terminated = gen_tac(p);
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
	return false;
}

static void dump_tac(Scope *scp)
{
	if (scp->jmp_in.size() > 0)
		printf("\nlabel %d:\n", scp->id);

	for (Tac &r : scp->bb->tacO0)
	{
		Ast *p = r.ast;

		switch (p->sem_stamp)
		{
		case SEM_IMMEDIATE:
			printf("const:\t %%%d num %d\n", r.dst, p->const_value);
			break;

		case SEM_OPERATOR:
			if (p->op == OP_ASSIGN)
				printf("assign:\t %%%d %s %%%d\n", r.dst, p->tk.src.c_str(), r.s1);
			else
				printf("op:\t %%%d = %%%d %s %%%d\n", r.dst, r.s1, p->tk.src.c_str(), r.s2);
			break;

		case SEM_VAR_DECLARE:
			printf("del:\t %s[%s] %%%d\n", p->tk.src.c_str(), p->symb->unique_name.c_str(), p->symb->vr);
			break;

		case SEM_VAR:
			printf("var:\t %%%d %s\n", r.dst, p->tk.src.c_str());
			break;

		case SEM_FUNC_CALL:
			printf("func call:\t %s\n", p->tk.src.c_str());
			break;

		case SEM_SAVE_RET_VALUE:
			printf("save_ret:\t %s %%%d\n", p->tk.src.c_str(), r.s1);
			break;

		default:
			ERR();
			break;
		}
	}

	if (scp->jmp_out != 0)
		printf("%d->%d:\n\n", scp->id, scp->jmp_out->id);

	for (Scope *p : scp->clds)
		dump_tac(p);
}

static vector<int> vr_union;
static vector<int> target_vr_unioned;

static int find(int a)
{
	if(a < 0)
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
	if(target_vr_unioned[b] > 0)
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
	vector<Tac> &tac = bb->tacO1;

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
			if(dst < 0)
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

	for (int i = 0; i < (int)tac.size(); )
	{
		Tac &t = tac[i];
		if (t.ast->op == OP_ASSIGN && t.dst == t.s1)
		{
			tac.erase(tac.begin() + i);
			continue;
		}
		i++;
	}

	for (Scope *p : scp->clds)
		gen_tac_O1(p);
}

static void elimicate_transient_vr(Scope *scp)
{
	scp->bb->tacO1 = scp->bb->tacO0;
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

		Symbol *symb = declare_record.ast[a.dst]->symb;
		if (symb->stamp == SYMB_PRIVATE_transient
		    && b.ast->op == OP_ASSIGN
		    //			&& a.dst != a.s2
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

void gen_three_address_code()
{
	gen_tac(&file_scp);
	dump_ast();

	printf("\n========== tac ==========\n");
	dump_tac(&file_scp);

	gen_tac_O1(&file_scp);
	elimicate_transient_vr(&file_scp);

	printf("\n========== tac ==========\n");
	dump_tac(&file_scp);
}
