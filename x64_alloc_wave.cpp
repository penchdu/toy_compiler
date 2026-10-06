/*
 * asymmetric_peak_valley_regalloc.cpp
 *
 *  Created on: 2026年9月26日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"
#include <cmath>

vector<Wave> vrwave;
vector<int> vr_usage;

void init_wave(BasicBlock *bb)
{
	for (auto &r : vrwave)
	{
		r.score.clear();
		r.insts.clear();
	}

	auto &v = bb->vrids;
	std::sort(v.begin(), v.end());
	v.erase(std::unique(v.begin(), v.end()), v.end());

	int mc_size = bb->x64mc_schedu.size();
	for (int vr : bb->vrids)
	{
		vrwave[vr].score.resize(mc_size, 0);
		vrwave[vr].insts.resize(mc_size, 0);
	}

	for (int inst_idx = 0; inst_idx < mc_size; inst_idx++)
	{
		X64mc &mc = bb->x64mc_schedu[inst_idx];
		for (int i = 0; i < 3; i++)
		{
			int vr = mc.vr_list[i];
			if (vr >= 0)
			{
				vrwave[vr].insts[inst_idx]++;
			}
		}
	}
}

void spill_vr(vector<X64mc> &x64mc_alloced, int vr, const string &tag)
{
	int &pr = vr2pr[vr].pr;
	int &u = vr2pr[vr].u;

	assert(pr != X64PR_MAX);
	assert(u != VR_USEAGE_INVALID);
	assert(pr2vr[pr].vr == vr);

	if (u & VR_USAGE_WRITE)
	{
		X64mc mc(MC_ST, vr);
		mc.pr1 = (int) pr;
		mc.ori_sem = tag + "spill";
		x64mc_alloced.push_back(mc);
	}

	pr2vr[pr].vr = INVALID__VR;
	pr = X64PR_MAX;
	u = VR_USEAGE_INVALID;
}
void spill_all_pr(BasicBlock *bb)
{
	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		if (pr2vr[pr].vr != INVALID__VR)
		{
			spill_vr(bb->x64mc_alloc_wave, pr2vr[pr].vr);
//			ERR();
		}
	}
}
static vector<int> x64pr_state(X64PR_MAX, INVALID__VR);

int wave_get_pr(BasicBlock *bb, int mc_idx)
{
	X64mc &mc = bb->x64mc_schedu[mc_idx];

	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		if (pr2vr[pr].vr == INVALID__VR)
			return pr;
	}

	float min_score = 100000;
	int min_score_vr = INVALID__VR;

	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		int vr = pr2vr[pr].vr;
		if (vr == mc.dst
		    || vr == mc.s1
		    || vr == mc.s2)
			continue;

		float score = vrwave[vr].score[mc_idx];
		if (score < min_score)
		{
			min_score = score;
			min_score_vr = vr;
		}
	}
	assert(min_score_vr != INVALID__VR);

	int pr = vr2pr[min_score_vr].pr;
	spill_vr(bb->x64mc_alloc_wave, min_score_vr);
	return pr;
}
static int get_pr__load_vr(BasicBlock *bb, int vr, int u, int mc_idx)
{
	/*
	 * todo
	 * mc_assign %1, %1
	 * s1 == s2
	 */
	assert(u != VR_USEAGE_INVALID);
	bool need_load = 0;

	if (vr2pr[vr].pr == X64PR_MAX)
	{
		int pr = wave_get_pr(bb, mc_idx);
		vr2pr[vr].pr = pr;
		vr2pr[vr].u = u | vr_usage[vr];
		pr2vr[pr].vr = vr;
		need_load = (u & VR_USAGE_READ);
	}
	// todo, if (!(vr2pr[vr].u & VR_USAGE_WRITE) && (u & VR_USAGE_WRITE) ?
// 	// else: VR 已经在寄存器中，值有效 —— 永远不需要 MC_LD！
// 	else if (!(vr2pr[vr].u & VR_USAGE_READ)
// 	    && (u & VR_USAGE_READ))
// 	{
// 		need_load = 1;
// 	}
	else
	{
		vr2pr[vr].u |= u | vr_usage[vr];
	}

	if (need_load)
	{
		X64mc mc(MC_LD, vr);
		mc.pr1 = vr2pr[vr].pr;
		mc.ori_sem = "alloc";
		bb->x64mc_alloc_wave.push_back(mc);
	}

	return vr2pr[vr].pr;
}
void check_and_clear_vr(Scope *scp, int vr)
{
	if (vr < 0)
		return;

//	int bb_use = scp->use_cnt_in_bb[vr];
//	if (bb_use < 0)
//		return;

	Ast *declare_at = vr_declare_manager.declare_at[vr];
	Symbol *symb = declare_at->symb;

//	LOG("scp=%d, %s %%%d name=%s, bb: %d %d, symb: %d %d",
//	    scp->id, is_private_symb(scp, vr) ? "private" : "outer  ",
//	    vr, symb->src.c_str(),
//	    scp->use_cnt_in_bb[vr], scp->consume_cnt_in_bb[vr],
//	    symb->use_cnt, symb->consume_cnt);

	if (vr2pr[vr].pr != X64PR_MAX
	    && symb->consume_cnt >= symb->use_cnt)
	//	    && symb->cld_consume_cnt >= symb->cld_use_cnt
	{
		if (scp->consume_cnt_in_bb.size() <= vr
		    || scp->use_cnt_in_bb.size() <= vr
		    || scp->consume_cnt_in_bb[vr] != scp->use_cnt_in_bb[vr])
		{
			ERR("%s, %lu %lu %%%d",
				scp->name.c_str(),
				scp->consume_cnt_in_bb.size(),
				scp->use_cnt_in_bb.size(),
				vr);
		}

		// last use in global
		int pr = vr2pr[vr].pr;
//		LOG("%s clear %s %s", scp->name.c_str(), symb->src.c_str(), pr_name[pr]);

		vr2pr[vr].pr = X64PR_MAX;
		vr2pr[vr].u = VR_USEAGE_INVALID;

		if (pr2vr[pr].vr == INVALID__VR)
		{
			int vr = pr2vr[pr].vr;
			ERR("scp=%s %s=%%%d", scp->name.c_str(),
			    vr_declare_manager.declare_at[vr]->tk.src.c_str(), vr);
		}
		assert(pr2vr[pr].vr != INVALID__VR);
		pr2vr[pr].vr = INVALID__VR;
	}
}

static void check_bb_use_cnt(Scope *scp, int target_symb_stamp = SYMB_ALL)
{
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		Ast *declare_at = vr_declare_manager.declare_at[vr];
		Symbol *symb = declare_at->symb;
		bool is_private = is_private_symb(scp, vr);

		if (!is_private && ((target_symb_stamp & SYMB_PRIVATE) || (target_symb_stamp & SYMB_PRIVATE_transient)))
			return;

		if (is_private && (target_symb_stamp & SYMB_OUTER))
			return;

		int use = scp->use_cnt_in_bb[vr];
		int consume = scp->consume_cnt_in_bb[vr];
		if (use != consume)
		{
			Ast *ast = vr_declare_manager.declare_at[vr];
			LOG("MISMATCH  scp=%s vr=%%%d name=%s, use=%d consume=%d stamp=%d",
			    scp->name.c_str(), vr, symb->src.c_str(), use, consume, ast->symb->stamp);
			ERR("%%%d, %d %d", vr, use, consume);
		}
	}
}

void check_pr_vr_consistency()
{
//	int e_vr = -1;
//	int e_pr = -1;
	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		int vr = pr2vr[pr].vr;
		if (vr == INVALID__VR)
			continue;

		if (vr2pr[vr].pr != pr)
		{
			Ast *declare = vr_declare_manager.declare_at[vr];
			ERR("  MISMATCH pr2vr[R%d].vr=%%%d(%s) but vr2pr[%%%d].pr=%d\n",
			    pr, vr, declare ? declare->tk.src.c_str() : "?", vr, vr2pr[vr].pr);
		}
	}
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		int pr = vr2pr[vr].pr;
		if (pr == X64PR_MAX)
			continue;

		if (pr2vr[pr].vr != vr)
		{
			Ast *declare = vr_declare_manager.declare_at[vr];
			ERR("  MISMATCH vr2pr[%%%d(%s)].pr=R%d but pr2vr[R%d].vr=%%%d\n",
			    vr, declare ? declare->tk.src.c_str() : "?", pr, pr, pr2vr[pr].vr);
		}
	}
}

void reg_alloc_wave__mc(BasicBlock *bb, const X64mc &schedued, int i)
{
	vector<X64mc> &x64mc_alloc = bb->x64mc_alloc_wave;
	X64mc mc = schedued;
	MachineCodeStamp mc_stamp = mc.mc_stamp;

	switch (mc_stamp)
	{
	case MC_LI:
		mc.pr1 = get_pr__load_vr(bb, mc.s1, VR_USAGE_WRITE, i);
		x64mc_alloc.push_back(mc);
		break;

	case MC_ASSIGN:
		if (mc.s1 == mc.s2)
			break;

		mc.pr1 = get_pr__load_vr(bb, mc.s1, VR_USAGE_WRITE, i);
		mc.pr2 = get_pr__load_vr(bb, mc.s2, VR_USAGE_READ, i);
		x64mc_alloc.push_back(mc);
		break;

	case MC_ADD:
		case MC_SUB:
		case MC_IMUL:
		case MC_DIV:

		mc.pr1 = get_pr__load_vr(bb, mc.s1, VR_USAGE_READ_WRITE, i);
		mc.pr2 = get_pr__load_vr(bb, mc.s2, VR_USAGE_READ, i);
		x64mc_alloc.push_back(mc);
		break;

	case MC_CMP_E:
		case MC_CMP_NE:
		case MC_CMP_L:
		case MC_CMP_LE:
		case MC_CMP_G:
		case MC_CMP_GE:
		mc.pr_dst = get_pr__load_vr(bb, mc.dst, VR_USAGE_WRITE, i);
		mc.pr1 = get_pr__load_vr(bb, mc.s1, VR_USAGE_READ, i);
		mc.pr2 = get_pr__load_vr(bb, mc.s2, VR_USAGE_READ, i);

		x64mc_alloc.push_back(mc);
		break;

	case MC_SAVE_RET:
		mc.pr1 = get_pr__load_vr(bb, mc.s1, VR_USAGE_READ, i);
		x64mc_alloc.push_back(mc);
		break;

	default:
		ERR("%d \n", mc_stamp);
		break;
	}
}

#define enable_while_opt 1
void reg_alloc_wave__scope(Scope *scp)
{
	//	LOG("scp %s", scp->name.c_str());
	BasicBlock *bb = (BasicBlock*) scp->basic_block;
	init_wave(bb);
	gen_wave(bb);
//	dump_wave(*bb);
	scp->consume_cnt_in_bb.clear();
	scp->consume_cnt_in_bb.resize(vr_declare_manager.size());

	if (scp->sem_stamp == SEM_WHILE)
	{
#if enable_while_opt
		reg_alloc_wave__while(scp);
		return;
#else
		while__pre_work(scp);
#endif
	}

	////////////////////////////////////////////////////////////

	for (int i = 0; i < bb->x64mc_schedu.size(); i++)
	{
		const X64mc &mc = bb->x64mc_schedu[i];
		reg_alloc_wave__mc(bb, mc, i);

		update_mc_consume_cnt(scp, mc, SYMB_ALL);

		check_and_clear_vr(scp, mc.dst);
		check_and_clear_vr(scp, mc.s1);
		check_and_clear_vr(scp, mc.s2);

		check_pr_vr_consistency();
	}

#if !enable_while_opt
	spill_all_pr(bb);
#endif

	for (Scope *p : scp->clds)
		reg_alloc_wave__scope(p);

	////////////////////////////////////////////////////////////
#if !enable_while_opt
	if (scp->sem_stamp == SEM_WHILE)
		while__end_work(scp);
#endif

}

void x64_reg_alloc_wave()
{
	LOG("\n======================== wave ========================\n");

	clear_vr_consume_cnt(&file_scp);
	vrwave.resize(vr_declare_manager.size());

	vr2pr.clear();
	vr2pr.resize(vr_declare_manager.size(), {X64PR_MAX, VR_USEAGE_INVALID});
	pr2vr.clear();
	pr2vr.resize(X64PR_MAX, {INVALID__VR});

	reg_alloc_wave__scope(&file_scp);

	check_vr_consume_cnt(&file_scp);
}

