/*
 * liveness.cpp
 *
 *  Created on: 2026年9月29日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

vector<int> ssa_use_cnt;
vector<int> ssa_consume_cnt;
vector<int> ssa_reference_cnt;

struct ScopeUsedOuterSymb {
	Scope *while_scp;
};
vector<ScopeUsedOuterSymb> while_use_outer_symb;
vector<ScopeUsedOuterSymb> while_cond_body_use_outer_symb;

void reset_scope_symb_consume_cnt(Scope *scp, int value)
{
	BasicBlock *bb = scp->bb;
	for (int i = 0; i < bb->mc_schedu.size(); i++)
	{
		const X64mc &mc = bb->mc_schedu[i];
		update_mc_consume_cnt(scp, mc, value);
		check_pr_vr_consistency();
	}

	for (Scope *p : scp->clds)
		reset_scope_symb_consume_cnt(p, value);
}
void clear_all_vr_consume_cnt(Scope *scp)
{
	ssa_consume_cnt.assign(vr_size, 0);

//	for (Symbol *p : scp->symb_table_ptr)
//		p->consume_cnt = 0;
//
//	for (Scope *p : scp->clds)
//		clear_all_vr_consume_cnt(p);
}
void check_vr_consume_cnt(Scope *scp)
{
	assert(ssa_use_cnt.size() == ssa_consume_cnt.size());

	for(int i = 0; i < ssa_use_cnt.size(); i++)
	{
		if(ssa_use_cnt[i] != ssa_consume_cnt[i])
			ERR("%s %%%d %d %d", vr_name(i), i, ssa_use_cnt[i], ssa_consume_cnt[i]);
	}
//	for (Symbol *p : scp->symb_table_ptr)
//	{
//		if (p->consume_cnt != p->use_cnt)
//			ERR("%s %%%d %d %d", p->src.c_str(), p->vr, p->use_cnt, p->consume_cnt);
//	}
//
//	for (Scope *p : scp->clds)
//		check_vr_consume_cnt(p);
}
bool is_private_symb(Scope *scp, int vr)
{
	Ast *ast = declare_record.ast[vr];
	Symbol *symb = ast->symb;

	if(symb->depth == scp->depth)
		return true;
	else if(symb->depth < scp->depth)
		return false;
	else
		ERR("%d > %d", symb->depth, scp->depth);

//	for (Symbol *p : *(scp->symb_table))
//	{
//		if (p->vr == vr)
//		{
//			assert(p == symb);
//			return true;
//		}
//	}

	return false;
}
void update_vr_consume_cnt(Scope *scp, int vr, int update_value)
{
//	assert(usage & VR_USAGE_READ);
	Ast *declare_at = declare_record.ast[vr];
	Scope *scp_declare_at = declare_at->scope;
//	LOG("%p %p, %s", declare_at->this_scp, declare_at->home_scp, declare_at->tk.src.c_str());
//	assert(declare_at->this_scp == declare_at->home_scp);

	Symbol *symb = declare_at->symb;
	bool is_private = is_private_symb(scp, vr);

	ssa_consume_cnt[vr] += update_value;
	scp->consume_cnt_in_bb[vr] += update_value;

	if (!is_private)
		assert(symb->stamp != SYMB_PRIVATE_transient);
}
void update_mc_consume_cnt(Scope *scp, const X64mc &mc, int update_value)
{
	switch (mc.stamp)
	{
	case MC_LD:
		ERR();
		break;

	case MC_ST:
		ERR();
		update_vr_consume_cnt(scp, mc.s1, update_value);
		break;

	case MC_ASSIGN:
		update_vr_consume_cnt(scp, mc.s1, update_value);
		break;

	case MC_ADD:
		case MC_SUB:
		case MC_IMUL:
		case MC_DIV:
		update_vr_consume_cnt(scp, mc.s1, update_value);
		update_vr_consume_cnt(scp, mc.s2, update_value);
		break;

	case MC_CMP_E:
		case MC_CMP_NE:
		case MC_CMP_L:
		case MC_CMP_LE:
		case MC_CMP_G:
		case MC_CMP_GE:
		update_vr_consume_cnt(scp, mc.s1, update_value);
		update_vr_consume_cnt(scp, mc.s2, update_value);
		break;

	case MC_TEST:
		case MC_SAVE_RET:
		update_vr_consume_cnt(scp, mc.s1, update_value);
		break;

	default:
		ERR("%d \n", mc.stamp);
		break;
	}
}

static void gen_vr_use_cnt(Scope *scp, int vr, int usage)
{
	Ast *p = declare_record.ast[vr];
	Symbol *symb = p->symb;

	if (scp->symb_table != p->scope->symb_table)	// outer
	{
		assert(p->symb->stamp != SYMB_PRIVATE_transient);
//		if (usage & VR_USAGE_READ)
//			scp->outer_symb_used.push_back(vr);
	}

	if (usage & VR_USAGE_READ)
	{
//		symb->use_cnt++;
		ssa_use_cnt[vr]++;
		scp->use_cnt_in_bb[vr]++;
	}

//	symb->reference_cnt++;
	ssa_reference_cnt[vr]++;
	scp->reference_cnt_in_bb[vr]++;
}

//static void _gen_use_cnt(Scope *scp)
//{
//	BasicBlock *bb = scp->bb;
//	scp->use_cnt_in_bb.clear();
//	scp->use_cnt_in_bb.resize(vr_size);
//	scp->reference_cnt_in_bb.clear();
//	scp->reference_cnt_in_bb.resize(vr_size);
//
//	for (auto &mc : bb->mcO1)
//	{
//		switch (mc.stamp)
//		{
//		case MC_LD:
//			gen_vr_use_cnt(scp, mc.s1, VR_USAGE_WRITE);
//			break;
//
//		case MC_ST:
//			gen_vr_use_cnt(scp, mc.s1, VR_USAGE_READ);
//			break;
//
//		case MC_ASSIGN:
//			gen_vr_use_cnt(scp, mc.dst, VR_USAGE_WRITE);
//			gen_vr_use_cnt(scp, mc.s1, VR_USAGE_READ);
//			break;
//
//		case MC_ADD:
//			case MC_SUB:
//			case MC_IMUL:
//			case MC_DIV:
//
//		case MC_CMP_E:
//			case MC_CMP_NE:
//			case MC_CMP_L:
//			case MC_CMP_LE:
//			case MC_CMP_G:
//			case MC_CMP_GE:
//			gen_vr_use_cnt(scp, mc.s1, VR_USAGE_READ);
//			gen_vr_use_cnt(scp, mc.s2, VR_USAGE_READ);
//			gen_vr_use_cnt(scp, mc.dst, VR_USAGE_WRITE);
//			break;
//
//		case MC_TEST:
//			case MC_SAVE_RET:
//			gen_vr_use_cnt(scp, mc.s1, VR_USAGE_READ);
//			break;
//
//		default:
//			ERR("%d \n", mc.stamp);
//			break;
//		}
//	}
//
//	for (Scope *p : scp->clds)
//		_gen_use_cnt(p);
//}
static void _gen_use_cnt(Scope *scp)
{
	BasicBlock *bb = scp->bb;
	scp->use_cnt_in_bb.clear();
	scp->use_cnt_in_bb.resize(vr_size);
	scp->reference_cnt_in_bb.clear();
	scp->reference_cnt_in_bb.resize(vr_size);

	for (auto &tac : bb->tacO1)
	{
		switch (tac.op)
		{
		case OP_ASSIGN:
		case OP_COPY_SSA:
			gen_vr_use_cnt(scp, tac.dst, VR_USAGE_WRITE);
			gen_vr_use_cnt(scp, tac.s1, VR_USAGE_READ);
			break;

		case OP_ADD:
			case OP_SUB:
			case OP_MUL:
			case OP_DIV:

		case OP_CMP_E:
			case OP_CMP_NE:
			case OP_CMP_L:
			case OP_CMP_LE:
			case OP_CMP_G:
			case OP_CMP_GE:
			gen_vr_use_cnt(scp, tac.s1, VR_USAGE_READ);
			gen_vr_use_cnt(scp, tac.s2, VR_USAGE_READ);
			gen_vr_use_cnt(scp, tac.dst, VR_USAGE_WRITE);
			break;

		case OP_TEST:
		case OP_SAVE_RET_VALUE:
			gen_vr_use_cnt(scp, tac.s1, VR_USAGE_READ);
			break;

		default:
			ERR("%d %d, %%%d %%%d %%%d \n", tac.sem_stamp ,tac.op, tac.dst, tac.s1, tac.s2);
			break;
		}
	}

	for (Scope *p : scp->clds)
		_gen_use_cnt(p);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
static void _check_while_used_outer_symb(int vr, int usage)
{
	Ast *p = declare_record.ast[vr];
	Symbol *symb = p->symb;

	// outer-while-scope should hold all outer-symb used in this while, include the outer-symb only used
	// in inner-while-scope
	for (auto &r : while_use_outer_symb)
	{
		if (symb->depth < r.while_scp->depth)
		{
			if (usage & VR_USAGE_READ)
				r.while_scp->outer_symb_read[vr]++;

			if (usage & VR_USAGE_WRITE)
			{
				r.while_scp->outer_symb_write[vr]++;
			}
		}
	}
}
static void check_while_used_outer_symb(Scope *scp)
{
	BasicBlock *bb = scp->bb;

	for (auto &mc : bb->mcO1)
	{
		switch (mc.stamp)
		{
		case MC_LD:
			_check_while_used_outer_symb(mc.s1, VR_USAGE_WRITE);
			break;

		case MC_ST:
			_check_while_used_outer_symb(mc.s1, VR_USAGE_READ);
			break;

		case MC_ASSIGN:
			_check_while_used_outer_symb(mc.dst, VR_USAGE_WRITE);
			_check_while_used_outer_symb(mc.s1, VR_USAGE_READ);
			break;

		case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			case MC_DIV:

		case MC_CMP_E:
			case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:
			_check_while_used_outer_symb(mc.s1, VR_USAGE_READ);
			_check_while_used_outer_symb(mc.s2, VR_USAGE_READ);
			_check_while_used_outer_symb(mc.dst, VR_USAGE_WRITE);
			break;

		case MC_TEST:
			case MC_SAVE_RET:
			_check_while_used_outer_symb(mc.s1, VR_USAGE_READ);
			break;

		default:
			ERR("%d \n", mc.stamp);
			break;
		}
	}
}
void find_while_used_outer_symb(Scope *scp)
{
	if (scp->sem_stamp == SEM_WHILE
	    || scp->sem_stamp == SEM_WHILE_COND
	    || scp->sem_stamp == SEM_WHILE_BODY
	    || scp->sem_stamp == SEM_IF
	    || scp->sem_stamp == SEM_IF_then
	    || scp->sem_stamp == SEM_IF_else)
	{
		while_use_outer_symb.push_back({scp});

		assert(scp->outer_symb_read.size() == 0);
		assert(scp->outer_symb_write.size() == 0);
		scp->outer_symb_read.resize(vr_size);
		scp->outer_symb_write.resize(vr_size);
	}

	if (while_use_outer_symb.size() > 0)
		check_while_used_outer_symb(scp);

	for (Scope *p : scp->clds)
		find_while_used_outer_symb(p);

	if (scp->sem_stamp == SEM_WHILE
	    || scp->sem_stamp == SEM_WHILE_COND
	    || scp->sem_stamp == SEM_WHILE_BODY
	    || scp->sem_stamp == SEM_IF
	    || scp->sem_stamp == SEM_IF_then
	    || scp->sem_stamp == SEM_IF_else)
	{
		while_use_outer_symb.pop_back();
	}
}
void dump_while_used_outer_symb(Scope *scp)
{
	if (scp->outer_symb_read.size() || scp->outer_symb_read.size())
		printf("%d %s \n", scp->id, scp->name.c_str());

	if (scp->outer_symb_read.size())
	{
		printf("	outer_symb_read: ");
		for (int vr = 0; vr < scp->outer_symb_read.size(); vr++)
		{
			int cnt = scp->outer_symb_read[vr];
			if (cnt > 0)
				printf(" %s=%%%d(%d)", declare_record.ast[vr]->tk.src.c_str(), vr, cnt);
		}
		printf("\n");
	}
	if (scp->outer_symb_write.size())
	{
		printf("	outer_symb_write: ");
		for (int vr = 0; vr < scp->outer_symb_write.size(); vr++)
		{
			int cnt = scp->outer_symb_write[vr];
			if (cnt > 0)
				printf(" %s=%%%d(%d)", declare_record.ast[vr]->tk.src.c_str(), vr, cnt);
		}
		printf("\n");
	}

	for (Scope *p : scp->clds)
		dump_while_used_outer_symb(p);
}

void gen_use_cnt()
{
	ssa_use_cnt.assign(vr_size, 0);
	ssa_reference_cnt.assign(vr_size, 0);

	_gen_use_cnt(&file_scp);

//	mc_list_name = "x64mc";
//	PRINT_ASM_HEAD("========== mc ==========");
//	dump_mc(&file_scp);
}
