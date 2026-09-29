/*
 * liveness.cpp
 *
 *  Created on: 2026年9月29日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

static void gen_vr_liveness(int vrid, int usage)
{
	VR_USAGE_READ,
	    VR_USAGE_WRITE,
	    VR_USAGE_READ_WRITE;

	Ast *ast = vr_manager.ast[vrid];
	SymbolVariable *symb = ast->symb;
	Scope *scp = ast->this_scp;

	if (ast->symb_stamp == SYMB_OUTER)
	{
		if (usage & VR_USAGE_READ)
		{
			symb->cld_use_cnt++;
			scp->used_cnt_in_bb[vrid]++;
			scp->outer_symb_used.push_back(vrid);
			symb->appear_cnt++;
		}

		if (usage & VR_USAGE_WRITE)
			symb->appear_cnt++;

		return;
	}
	assert(ast->symb_stamp == SYMB_PRIVATE || ast->symb_stamp == SYMB_PRIVATE_transient);

	if (usage & VR_USAGE_READ)
	{
		symb->use_cnt++;
		scp->used_cnt_in_bb[vrid]++;
		symb->appear_cnt++;
	}
	if (usage & VR_USAGE_WRITE)
		symb->appear_cnt++;
}

static void _gen_liveness(Scope *scp)
{
	BasicBlock *bb = (BasicBlock*) scp->basic_block;
	scp->used_cnt_in_bb.resize(vr_manager.id + 1);

	for (auto &mc : bb->x64mc)
	{
		MachineCodeStamp mc_stamp = mc.mc_stamp;

		switch (mc_stamp)
		{
		case MC_LI:
			gen_vr_liveness(mc.s1, VR_USAGE_WRITE);
			break;

		case MC_LD:
			gen_vr_liveness(mc.s1, VR_USAGE_WRITE);
			break;

		case MC_ST:
			gen_vr_liveness(mc.s1, VR_USAGE_READ);
			PRINT_MORE
			break;

		case MC_ASSIGN:
			case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			case MC_DIV:
			gen_vr_liveness(mc.s1, VR_USAGE_READ_WRITE);
			gen_vr_liveness(mc.s2, VR_USAGE_READ);
			break;

		case MC_CMP_E:
			case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:
			gen_vr_liveness(mc.s1, VR_USAGE_READ);
			gen_vr_liveness(mc.s2, VR_USAGE_READ);
			gen_vr_liveness(mc.dst, VR_USAGE_WRITE);
			break;

		case MC_SAVE_RET:
			gen_vr_liveness(mc.s1, VR_USAGE_READ);
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

