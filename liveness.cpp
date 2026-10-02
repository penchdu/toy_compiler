/*
 * liveness.cpp
 *
 *  Created on: 2026年9月29日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

void clear_vr_consume_cnt(Scope *scp)
{
	for (auto *p : scp->scope_symb_table)
	{
		p->consume_cnt = 0;
//		p->cld_consume_cnt = 0;
	}

	for (Scope *p : scp->clds)
		clear_vr_consume_cnt(p);
}
static bool is_private_symb(Scope *scp, int vr)
{
	Ast *declare_at = vr_declare_manager.declare_at[vr];
	SymbolVariable *symb = declare_at->symb;

	for (auto p : *(scp->symb_table))
	{
		if (p->vr == vr)
		{
			assert(p == symb);
			return true;
		}
	}

	return false;
//	LOG("%p %p, %s", declare_at->this_scp, declare_at->home_scp, declare_at->tk.src.c_str());
}
static void _update_vr_consume_cnt(Scope *scp, int vr,
    int target_symb_stamp, int usage = VR_USAGE_READ)
{
	assert(usage & VR_USAGE_READ);
	Ast *declare_at = vr_declare_manager.declare_at[vr];
	Scope *scp_declare_at = declare_at->this_scp;
//	LOG("%p %p, %s", declare_at->this_scp, declare_at->home_scp, declare_at->tk.src.c_str());
//	assert(declare_at->this_scp == declare_at->home_scp);

	SymbolVariable *symb = declare_at->symb;
	bool is_private = is_private_symb(scp, vr);

	if ((target_symb_stamp & SYMB_PRIVATE) || (target_symb_stamp & SYMB_PRIVATE_transient))
	{
		if (is_private)
			symb->consume_cnt++;	// private
	}

	if (target_symb_stamp & SYMB_OUTER)
	{
		// outer
		if (!is_private)
		{
			assert(declare_at->symb_stamp != SYMB_PRIVATE_transient);
			symb->consume_cnt++;
		}
	}

	scp->consume_cnt_in_bb[vr]++;
}
void update_vr_consume_cnt(Scope *scp, const X64mc &mc, int target_symb_stamp)
{
	MachineCodeStamp mc_stamp = mc.mc_stamp;

	switch (mc_stamp)
	{
	case MC_LI:
		break;

	case MC_LD:
		break;

	case MC_ST:
		_update_vr_consume_cnt(scp, mc.s1, target_symb_stamp);
		break;

	case MC_ASSIGN:
		_update_vr_consume_cnt(scp, mc.s2, target_symb_stamp);
		break;

	case MC_ADD:
		case MC_SUB:
		case MC_IMUL:
		case MC_DIV:
		_update_vr_consume_cnt(scp, mc.s1, target_symb_stamp);
		_update_vr_consume_cnt(scp, mc.s2, target_symb_stamp);
		break;

	case MC_CMP_E:
		case MC_CMP_NE:
		case MC_CMP_L:
		case MC_CMP_LE:
		case MC_CMP_G:
		case MC_CMP_GE:
		_update_vr_consume_cnt(scp, mc.s1, target_symb_stamp);
		_update_vr_consume_cnt(scp, mc.s2, target_symb_stamp);
		break;

	case MC_SAVE_RET:
		_update_vr_consume_cnt(scp, mc.s1, target_symb_stamp);
		break;

	default:
		ERR("%d \n", mc_stamp);
		break;
	}
}

static void gen_vr_use_cnt(Scope *scp, int vr, int usage)
{
	Ast *declare_at = vr_declare_manager.declare_at[vr];
	Scope *scp_declare_at = declare_at->this_scp;
//	LOG("%p %p, %s", declare_at->this_scp, declare_at->home_scp, declare_at->tk.src.c_str());
//	assert(declare_at->this_scp == declare_at->home_scp);

	SymbolVariable *symb = declare_at->symb;

	if (scp->symb_table != scp_declare_at->symb_table)	// outer
	{
		assert(declare_at->symb_stamp != SYMB_PRIVATE_transient);

		if (usage & VR_USAGE_READ)
		{
			symb->use_cnt++;
//			symb->cld_use_cnt++;
			symb->appear_cnt++;
			scp->use_cnt_in_bb[vr]++;
			scp->outer_symb_used.push_back(vr);
		}

		if (usage & VR_USAGE_WRITE)
			symb->appear_cnt++;

		scp->appear_cnt_in_bb[vr]++;
	}
	else
	{	// private
		if (usage & VR_USAGE_READ)
		{
			symb->use_cnt++;
			symb->appear_cnt++;
			scp->use_cnt_in_bb[vr]++;
		}
		if (usage & VR_USAGE_WRITE)
			symb->appear_cnt++;

		scp->appear_cnt_in_bb[vr]++;
	}
}

static void _gen_liveness(Scope *scp)
{
	BasicBlock *bb = (BasicBlock*) scp->basic_block;
	scp->use_cnt_in_bb.resize(vr_declare_manager.size());
	scp->appear_cnt_in_bb.resize(vr_declare_manager.size());

	for (auto &mc : bb->x64mc)
	{
		MachineCodeStamp mc_stamp = mc.mc_stamp;

		switch (mc_stamp)
		{
		case MC_LI:
			gen_vr_use_cnt(scp, mc.s1, VR_USAGE_WRITE);
			break;

		case MC_LD:
			gen_vr_use_cnt(scp, mc.s1, VR_USAGE_WRITE);
			break;

		case MC_ST:
			gen_vr_use_cnt(scp, mc.s1, VR_USAGE_READ);
			PRINT_MORE
			break;

		case MC_ASSIGN:
			gen_vr_use_cnt(scp, mc.s1, VR_USAGE_WRITE);
			gen_vr_use_cnt(scp, mc.s2, VR_USAGE_READ);
			break;

		case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			case MC_DIV:
			gen_vr_use_cnt(scp, mc.s1, VR_USAGE_READ_WRITE);
			gen_vr_use_cnt(scp, mc.s2, VR_USAGE_READ);
			break;

		case MC_CMP_E:
			case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:
			gen_vr_use_cnt(scp, mc.s1, VR_USAGE_READ);
			gen_vr_use_cnt(scp, mc.s2, VR_USAGE_READ);
			gen_vr_use_cnt(scp, mc.dst, VR_USAGE_WRITE);
			break;

		case MC_SAVE_RET:
			gen_vr_use_cnt(scp, mc.s1, VR_USAGE_READ);
			break;

		default:
			ERR("%d \n", mc_stamp);
			break;
		}
	}

	for (Scope *p : scp->clds)
		_gen_liveness(p);
}

void gen_liveness()
{
	_gen_liveness(&file_scp);

//	mc_list_name = "x64mc";
//	PRINT_ASM_HEAD("========== mc ==========");
//	dump_mc(&file_scp);
}

