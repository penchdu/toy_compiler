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
static vector<PrToVr> pr2vr(X64PR_MAX, {invalid_vr});

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
//		vrwave[vr].score.clear();
		vrwave[vr].score.resize(mc_size, 0);
//		vrwave[vr].cnt.clear();
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

static void spill_pr(vector<X64mc> &x64mc_alloced, int vr)
{
	int pr = vr2pr[vr].pr;
	int u = vr2pr[vr].u;

	assert(pr != X64PR_MAX);
	assert(u != VR_USEAGE_INVALID);

	if (u & VR_USAGE_WRITE)
	{
		X64mc mc(MC_ST, vr);
		mc.pr1 = (int) pr;
		mc.ori_sem = "spill";
		x64mc_alloced.push_back(mc);
	}

	vr2pr[vr].pr = X64PR_MAX;
	vr2pr[vr].u = VR_USEAGE_INVALID;

	assert(pr2vr[pr].vr == vr);
	pr2vr[pr].vr = invalid_vr;
}

static vector<int> x64pr_state(X64PR_MAX, invalid_vr);

static int get_pr(vector<X64mc> &x64mc_alloced, int mc_idx)
{
	for (int i = R10D; i < X64PR_MAX; i++)
	{
		if (pr2vr[i].vr == invalid_vr)
		{
			return i;
		}
	}

	float min_score = 100000;
	int min_score_vr = invalid_vr;

	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		int vr = pr2vr[pr].vr;
		if(vr2pr[vr].allow_spill == false)
			continue;

		float score = vrwave[vr].score[mc_idx];
		if(score < min_score)
		{
			min_score = score;
			min_score_vr = vr;
		}
	}
	assert(min_score_vr != invalid_vr);

	int pr = vr2pr[min_score_vr].pr;
	spill_pr(x64mc_alloced, min_score_vr);

	return pr;
}
static int get_pr__load_vr(vector<X64mc> &x64mc_alloced, int vr, int u, int mc_idx)
{
	/*
	 * todo
	 * mc_assign %1, %1
	 * s1 == s2
	 */
	bool need_load = 0;
	if (vr2pr[vr].pr == X64PR_MAX)
	{
		int pr = get_pr(x64mc_alloced, mc_idx);
		vr2pr[vr].pr = pr;
		vr2pr[vr].u = u;
		vr2pr[vr].allow_spill = false;
		pr2vr[pr].vr = vr;
		need_load = (u & VR_USAGE_READ);
	}
// 	// else: VR 已经在寄存器中，值有效 —— 永远不需要 MC_LD！
// 	else if (!(vr2pr[vr].u & VR_USAGE_READ)
// 	    && (u & VR_USAGE_READ))
// 	{
// 		need_load = 1;
// //		ERR();
// 	}

	vr2pr[vr].u |= u;
	vr2pr[vr].allow_spill = false;

	if (need_load)
	{
		X64mc mc(MC_LD, vr);
		mc.pr1 = vr2pr[vr].pr;
		mc.ori_sem = "alloc";
		x64mc_alloced.push_back(mc);
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
			mc.pr1 = get_pr__load_vr(x64mc_alloc, mc.s1, VR_USAGE_WRITE, i);
			x64mc_alloc.push_back(mc);
			vr2pr[mc.s1].allow_spill = true;
			break;

		case MC_ASSIGN:
			//			if (mc.s1 == mc.s2)
//			    break;
			mc.pr1 = get_pr__load_vr(x64mc_alloc, mc.s1, VR_USAGE_WRITE, i);
			mc.pr2 = get_pr__load_vr(x64mc_alloc, mc.s2, VR_USAGE_READ, i);
			x64mc_alloc.push_back(mc);
			vr2pr[mc.s1].allow_spill = true;
			vr2pr[mc.s2].allow_spill = true;
			break;

		case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			case MC_DIV:

			mc.pr1 = get_pr__load_vr(x64mc_alloc, mc.s1, VR_USAGE_READ_WRITE, i);
			mc.pr2 = get_pr__load_vr(x64mc_alloc, mc.s2, VR_USAGE_READ, i);
			x64mc_alloc.push_back(mc);
			vr2pr[mc.s1].allow_spill = true;
			vr2pr[mc.s2].allow_spill = true;
			break;

		case MC_CMP_E:
			case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:
			mc.pr_dst = get_pr__load_vr(x64mc_alloc, mc.dst, VR_USAGE_WRITE, i);
			mc.pr1 = get_pr__load_vr(x64mc_alloc, mc.s1, VR_USAGE_READ, i);
			mc.pr2 = get_pr__load_vr(x64mc_alloc, mc.s2, VR_USAGE_READ, i);

			x64mc_alloc.push_back(mc);
			vr2pr[mc.s1].allow_spill = true;
			vr2pr[mc.s2].allow_spill = true;
			vr2pr[mc.dst].allow_spill = true;
			PRINT_MORE
			break;

		case MC_SAVE_RET_VALUE:
			mc.pr1 = get_pr__load_vr(x64mc_alloc, mc.s1, VR_USAGE_READ, i);
			x64mc_alloc.push_back(mc);
			vr2pr[mc.s1].allow_spill = true;
			break;

		default:
			ERR("%d \n", mc_stamp);
			break;
		}
	}
}

void wave_reg_alloc()
{
	vrwave.resize(vregm.id + 1);
	vr2pr.resize(vregm.id + 1);

	for (BasicBlock &bb : basic_blocks)
	{
		VrToPr a;
		std::fill(vr2pr.begin(), vr2pr.end(), a);
		std::fill(pr2vr.begin(), pr2vr.end(), PrToVr{invalid_vr});

		init_wave(bb);
		compute_wave(bb);
//		dump_wave(bb);

		_x64_reg_alloc_wave(bb);
	}

}
