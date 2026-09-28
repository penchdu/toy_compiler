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

extern vector<BasicBlock> basic_blocks;
vector<Wave> vrwave;
static vector<VrToPr> vr2pr;
static vector<PrToVr> pr2vr(X64PR_MAX, {INVALID__VR});

static void init_wave(BasicBlock &bb)
{
	for (auto &r : vrwave)
	{
		r.score.clear();
		r.insts.clear();
	}

	auto &v = bb.vrids;
	std::sort(v.begin(), v.end());
	v.erase(std::unique(v.begin(), v.end()), v.end());

	int mc_size = bb.x64mc_schedu.size();
	for (int vr : bb.vrids)
	{
		vrwave[vr].score.resize(mc_size, 0);
		vrwave[vr].insts.resize(mc_size, 0);
	}

	for (int inst_idx = 0; inst_idx < mc_size; inst_idx++)
	{
		X64mc &mc = bb.x64mc_schedu[inst_idx];
		for (int i = 0; i < 3; i++)
		{
			int vr = mc.vr[i];
			if (vr >= 0)
			{
				vrwave[vr].insts[inst_idx]++;
			}
		}
	}
}

static void spill_vr(vector<X64mc> &x64mc_alloced, int vr)
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
		mc.ori_sem = "spill";
		x64mc_alloced.push_back(mc);
	}

	pr2vr[pr].vr = INVALID__VR;
	pr = X64PR_MAX;
	u = VR_USEAGE_INVALID;
}
static void spill_all_pr(BasicBlock &bb)
{
	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		if (pr2vr[pr].vr != INVALID__VR)
			spill_vr(bb.x64mc_alloc, pr2vr[pr].vr);
	}
}
static vector<int> x64pr_state(X64PR_MAX, INVALID__VR);

static int get_pr(BasicBlock &bb, int mc_idx)
{
	X64mc &mc = bb.x64mc_schedu[mc_idx];
//	int vr = mc.s1;
//	if (vr >= 0 && vr_manager.ast[vr]->symb_live_region == SYMB_PRIVATE_transient)
//		vr_manager.ast[vr]->consume_cnt++;
//
//	vr = mc.s2;
//	if (vr >= 0 && vr_manager.ast[vr]->symb_live_region == SYMB_PRIVATE_transient)
//		vr_manager.ast[vr]->consume_cnt++;

//	if(vr_manager.ast[vr]->symb_live_region == SYMB_PRIVATE_transient
//		&& (vr == bb.x64mc_schedu[mc_idx].s1 || vr == bb.x64mc_schedu[mc_idx].s2))
//	{
//		vr_manager.ast[vr]->consume_cnt++;
//	}

	for (int i = R10D; i < X64PR_MAX; i++)
	{
		int vr = pr2vr[i].vr;
		if (vr != INVALID__VR
			&& vr_manager.ast[vr]->symb_live_region == SYMB_PRIVATE_transient
			&& (mc.mc_stamp == MC_ASSIGN && vr == mc.s2))
			vr_manager.ast[vr]->consume_cnt++;
	}

	for (int i = R10D; i < X64PR_MAX; i++)
	{
		int vr = pr2vr[i].vr;
		if (vr == INVALID__VR)
			return i;

		if (vr_manager.ast[vr]->symb_live_region == SYMB_PRIVATE_transient
		    && vr_manager.ast[vr]->consume_cnt >= vr_manager.ast[vr]->use_cnt
		    && vr != mc.s1
		    && vr != mc.s2
		    && vr != mc.dst)
			return i;
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
	spill_vr(bb.x64mc_alloc, min_score_vr);

	return pr;
}
static int get_pr__load_vr(BasicBlock &bb, int vr, int u, int mc_idx)
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
		int pr = get_pr(bb, mc_idx);
		vr2pr[vr].pr = pr;
		vr2pr[vr].u = u;
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
		vr2pr[vr].u |= u;
	}

	if (need_load)
	{
		X64mc mc(MC_LD, vr);
		mc.pr1 = vr2pr[vr].pr;
		mc.ori_sem = "alloc";
		bb.x64mc_alloc.push_back(mc);
	}

	return vr2pr[vr].pr;
}

void _x64_reg_alloc_wave(BasicBlock &bb)
{
	X64mc inst;
	vector<X64mc> &x64mc_alloc = bb.x64mc_alloc;

	for (int i = 0; i < bb.x64mc_schedu.size(); i++)
	{
		X64mc mc = bb.x64mc_schedu[i];
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
}

void wave_reg_alloc()
{
	vrwave.resize(vr_manager.id + 1);

	vr2pr.clear();
	vr2pr.resize(vr_manager.id + 1, {X64PR_MAX, VR_USEAGE_INVALID});
	pr2vr.clear();
	pr2vr.resize(X64PR_MAX, {INVALID__VR});

	for (BasicBlock &bb : basic_blocks)
	{
		init_wave(bb);
		gen_wave(bb);
//		dump_wave(bb);

		_x64_reg_alloc_wave(bb);
		spill_all_pr(bb);
	}

}
