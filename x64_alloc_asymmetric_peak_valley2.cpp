///*
// * asymmetric_peak_valley_regalloc.cpp
// *
// *  Created on: 2026年9月26日
// *      Author: x
// */
//
//#include "frontend.h"
//#include "x64_back_end.h"
//#include "basic_block.h"
//#include <cmath>
//
//extern vector<BasicBlock> basic_blocks;
//
//struct VRWave {
//	vector<float> score;
//	vector<int> inst_idx;		// >=0, <= 3
//};
//vector<VRWave> wave;
//vector<int> wave_vrid;
//float max_wave_value = 0;
//
//static void dump_wave()
//{
//	static const char *level[] = {
//	    " ",
//	    "▁",
//	    "▂",
//	    "▃",
//	    "▄",
//	    "▅",
//	    "▆",
//	    "▇",
//
//	    "█",
//	};
//
//	printf("\n========== wave ==========\n");
//
//	for (int vr : wave_vrid)
//	{
//		const vector<float> &score = wave[vr].score;
//
//		if (score.empty())
//			continue;
//
//		float max_score =
//		    *std::max_element(score.begin(), score.end());
//
//		printf("VR %-4d | ", vr);
//
//		if (max_score <= 0.0f)
//		{
//			for (size_t pc = 0; pc < score.size(); ++pc)
//				printf(" ");
//			printf("\n");
//			continue;
//		}
//
//		for (float s : score)
//		{
//			float ratio = s / max_score;
//
//			int h = (int) (ratio * 8.0f);
//
//			if (h < 0)
//				h = 0;
//			if (h > 8)
//				h = 8;
//
//			printf("%s", level[h]);
//		}
//
//		printf("  max=%.2f\n", max_score);
//	}
//
//	printf("     ");
//
//	if (!wave_vrid.empty())
//	{
//		int n = wave[wave_vrid[0]].score.size();
//
//		printf("   ");
//
//		for (int i = 0; i < n; ++i)
//		{
//			if (i % 10 == 0)
//				printf("|");
//
//			else
//				printf("-");
//		}
//
//		printf("\n     ");
//
//		for (int i = 0; i < n; ++i)
//		{
//			if (i % 10 == 0)
//				printf("%-10d", i);
//		}
//
//		printf("\n");
//	}
//
//	printf("===========================\n");
//}
//
//static void init_wave()
//{
//	int mc_size = 0;
//
//	for (BasicBlock &bb : basic_blocks)
//	{
//		int n = bb.x64mc_schedu.size();
//		mc_size += n;
//
//		for (X64mc &mc : bb.x64mc_schedu)
//		{
////		for (int i = 0; i < 3; i++)
////		{
////			int vr = mc.vr[i];
////			if (vr >= 0)
////			{
////				wave[vr].score.clear();
////				wave[vr].score.resize(n, 0);
////				wave[vr].inst_idx.clear();
////				wave[vr].inst_idx.resize(n, 0);
////			}
////		}
//		}
//	}
//
//	for (int k = 0; k < bb.x64mc_schedu.size(); k++)
//	{
//		X64mc &mc = bb.x64mc_schedu[k];
//
//		for (int i = 0; i < 3; i++)
//		{
//			int vr = mc.vr[i];
//			if (vr >= 0)
//			{
//				wave[vr].inst_idx[k]++;
//				wave_vrid.push_back(vr);
//			}
//		}
//	}
//
//	auto &v = wave_vrid;
//	std::sort(v.begin(), v.end());
//	v.erase(std::unique(v.begin(), v.end()), v.end());
//}
//
//static void compute_wave_triangle(VRWave &wave)
//{
//	constexpr int R = 5;
//	const int slots = wave.inst_idx.size();
//
//	std::fill(wave.score.begin(), wave.score.end(), 0.0f);
//
//	for (int inst_appear_cnt = 0; inst_appear_cnt < slots; inst_appear_cnt++)
//	{
//		if (wave.inst_idx[inst_appear_cnt] == 0)
//			continue;
//
//		for (int pc = 0; pc < slots; ++pc)
//		{
//			int d = std::abs(pc - inst_appear_cnt);
//
//			if (d <= R)
//			{
//				float weight = (float) (R - d) / R;
//				wave.score[pc] += wave.inst_idx[inst_appear_cnt] * weight;
//			}
//		}
//	}
//}
//static void compute_wave_decay(VRWave &w)
//{
//	int left_R = 4;
//	float decay = 0.8f;
//
//	const int n = w.inst_idx.size();
//	std::fill(w.score.begin(), w.score.end(), 0.0f);
//
//	for (int use_pc = 0; use_pc < n; ++use_pc)
//	{
//		int count = w.inst_idx[use_pc];
//
//		if (count == 0)
//			continue;
//
//		for (int pc = 0; pc < n; ++pc)
//		{
//			float weight = 0.0f;
//
//			if (pc <= use_pc)
//			{
//				// use 前：缓慢建立波峰
//				int d = use_pc - pc;
//
//				if (d <= left_R)
//					weight = 1.0f - (float) d / left_R;
//			}
//			else
//			{
//				// use 后：快速指数衰减
//				int d = pc - use_pc;
//				weight = std::exp(-decay * d);
//			}
//
//			w.score[pc] += count * weight;
//		}
//	}
//}
//
//static void compute_wave(BasicBlock &bb)
//{
//	for (int vr : wave_vrid)
//	{
////    	compute_wave_triangle(wave[vr]);
//		compute_wave_decay(wave[vr]);
//	}
//}
//
//void asymmetric_peak_wave_reg_alloc()
//{
//	for (BasicBlock &bb : basic_blocks)
//	{
//		wave_vrid.clear();
//		max_wave_value = 0;
//
//		init_wave();
//		compute_wave(bb);
//
//		dump_wave();
//
////		break;
//	}
//}
/*
#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

extern bool simulate;
extern bool simulate_end;

static void get_scope_use_vr(Scope *scp, vector<int> &vrs, int visitor)
{
	LOG("%s", scp->name.c_str());
	if (vrs.size() == X64PR_MAX || scp->visitor == visitor)
		return;

	scp->visitor = visitor;

	auto &mcs = scp->basic_block->x64mc_schedu;
	for (auto mc : mcs)
	{
		for (int i = 0; i < 3; i++)
		{
			int vr = mc.vr_list[i];
			if (vr < 0)
				continue;

			if (std::find(vrs.begin(), vrs.end(), vr) == vrs.end())
				vrs.push_back(vr);
			printf("vrs.size %ld \n", vrs.size());

			if (vrs.size() == X64PR_MAX)
				return;
		}
	}

	for (Scope *p : scp->clds)
	{
		if (vrs.size() < X64PR_MAX)
			get_scope_use_vr(p, vrs, visitor);
	}

}
static void get_future_use_vr(Scope *scp, int cld_idx)
{
	if (!scp)
		return;

	for (int i = cld_idx; i < scp->clds.size(); i++)
	{
		Scope *p = scp->clds[i];
		ra_wave_scope(p);
	}

	if (scp->sem_stamp == SEM_WHILE_BODY)
	{
		simulate_end = 1;
		return;
	}

	if (!simulate_end && scp->sem_stamp != SEM_FUNC_DEFINE)
	{
		Scope *next_scp = scp->parent->clds[scp->idx + 1];
		ra_wave_scope(next_scp);
	}
}

static void then_pre_work(Scope *else_branch)
{
	for (int vr = 0; vr < else_branch->outer_symb_read.size(); vr++)
	{
		int cnt = else_branch->outer_symb_read[vr];
		if (cnt == 0)
			continue;
		Symbol *symb = (vr_declare_manager.declare_at[vr])->symb;
		symb->use_cnt -= cnt;
	}
}
static void then_end_work(Scope *else_branch)
{
	for (int vr = 0; vr < else_branch->outer_symb_read.size(); vr++)
	{
		int cnt = else_branch->outer_symb_read[vr];
		if (cnt == 0)
			continue;
		Symbol *symb = (vr_declare_manager.declare_at[vr])->symb;
		symb->use_cnt += cnt;
	}
}

void ra_wave_if(Scope *if_scp)
{
	LOG("scp %d", if_scp->id);

	Scope *cond = if_scp->clds[0];
	Scope *then_branch = if_scp->clds[1];
	Scope *else_branch = if_scp->clds[2];
	Scope *if_tail = if_scp->clds[3];
	Scope *then_tail = get_inner_tail_of_scope(then_branch);
	Scope *else_tail = get_inner_tail_of_scope(else_branch);

	// cond
	ra_wave_scope(cond);
	vector<VrToPr> after_cond_vr_state = vr_state;
	vector<PrToVr> after_cond_pr_state = pr_state;

	// then
	then_pre_work(else_branch);
	ra_wave_scope(then_branch);

	int after_then_swap_pr = get_swap_pr(then_branch->basic_block);
	assert(after_then_swap_pr != X64PR_MAX);

	vector<VrToPr> after_then_vr_state = vr_state;
	vector<PrToVr> after_then_pr_state = pr_state;

	then_end_work(else_branch);
	vr_state = after_cond_vr_state;
	pr_state = after_cond_pr_state;

	// else
	ra_wave_scope(else_branch);

	int after_else_swap_pr = get_swap_pr(then_branch->basic_block);
	assert(after_else_swap_pr != X64PR_MAX);

//	vector<VrToPr> after_else_vr_state = vr_state;
//	vector<PrToVr> after_else_pr_state = pr_state;

// future pr use
	simulate = 1;
	simulate_end = 0;

	vector<int> future_vr;
	vector<VrToPr> tmp_vr_state = vr_state;
	vector<PrToVr> tmp_pr_state = pr_state;
	vr_state.clear();
	vr_state.resize(vr_size, {X64PR_MAX, VR_USAGE_INVALID});
	pr_state.clear();
	pr_state.resize(X64PR_MAX, {INVALID_VR});

	get_future_use_vr(if_scp->parent, if_scp->idx + 1);

	vector<VrToPr> target_vr_state = vr_state;
	vector<PrToVr> target_pr_state = pr_state;
	vr_state = tmp_vr_state;
	pr_state = tmp_pr_state;
//	 not really good, because while and if have branch

	printf("\n================== if%d target_vr_state ==================\n", if_scp->id);
	for (int vr = 0; vr < vr_size; vr++)
	{
		if(target_vr_state[vr].pr != X64PR_MAX)
			printf("  %s=%%%d", vr_name(vr), vr);
	}
	printf("\n");

	dump_vr_state(after_then_vr_state, "after_then_vr_state" + to_string(if_scp->id));
	int then_branch_prs = 0;
	for (auto &r : after_then_pr_state)
	{
		int vr = r.vr;
		if (vr == INVALID_VR)
			continue;
		then_branch_prs++;

		if (std::find(future_vr.begin(), future_vr.end(), vr) == future_vr.end())
		{
			auto &mcs = then_branch->basic_block->x64mc_alloc_wave;
			spill_vr(after_then_vr_state, after_then_pr_state, vr, mcs, " then");
			then_branch_prs--;
		}
	}
	dump_vr_state(after_then_vr_state, "update after_then_vr_state" + to_string(if_scp->id));

	dump_vr_state(vr_state, "vr_state" + to_string(if_scp->id));
	int else_branch_prs = 0;
	for (auto &r : pr_state)
	{
		int vr = r.vr;
		if (vr == INVALID_VR)
			continue;
		else_branch_prs++;

		if (std::find(future_vr.begin(), future_vr.end(), vr) == future_vr.end())
		{
			auto &mcs = else_branch->basic_block->x64mc_alloc_wave;
			spill_vr(vr_state, pr_state, vr, mcs, " else");
			else_branch_prs--;
		}
	}
	dump_vr_state(vr_state, "update vr_state" + to_string(if_scp->id));

	if (then_branch_prs > else_branch_prs)
	{
		while_recover_pr(
		    after_then_vr_state,
		    after_then_pr_state,
		    vr_state,
		    pr_state,
		    get_inner_tail_of_scope(else_branch),
		    after_then_swap_pr);
	}
	else
	{
		while_recover_pr(
		    vr_state,
		    pr_state,
		    after_then_vr_state,
		    after_then_pr_state,
		    get_inner_tail_of_scope(then_branch),
		    after_else_swap_pr);
	}

	for (int vr = 0; vr < vr_size; vr++)
	{
		if (after_then_vr_state[vr].pr != X64PR_MAX)
			vr_state[vr].dirty |= after_then_vr_state[vr].dirty;
	}

}

 *
 */




/*
 *
 * /////////////////////////////////////////////////////


#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

vector<int> if_tail_vr;

static void get_scope_use_vr(Scope *scp, vector<int> &vrs, int visitor)
{
	LOG("%s", scp->name.c_str());
	if (vrs.size() == X64PR_MAX || scp->visitor == visitor)
		return;

	scp->visitor = visitor;

	auto &mcs = scp->basic_block->x64mc_schedu;
	for (auto mc : mcs)
	{
		for (int i = 0; i < 3; i++)
		{
			int vr = mc.vr_list[i];
			if (vr < 0)
				continue;

			if (if_tail_vr[vr] > 0)
			{
				if (std::find(vrs.begin(), vrs.end(), vr) == vrs.end())
					vrs.push_back(vr);
			}

			printf("vrs.size %ld \n", vrs.size());

			if (vrs.size() == X64PR_MAX)
				return;
		}
	}

	for (Scope *p : scp->clds)
	{
		if (vrs.size() < X64PR_MAX)
			get_scope_use_vr(p, vrs, visitor);
	}

}
static void get_future_use_vr(Scope *scp, int cld_idx,
    vector<int> &vrs, int visitor)
{
	if (!scp || vrs.size() == X64PR_MAX)
		return;

	for (int i = cld_idx; i < scp->clds.size(); i++)
	{
		if (vrs.size() == X64PR_MAX)
			return;

		Scope *p = scp->clds[i];
		get_scope_use_vr(p, vrs, visitor);
	}

	if (vrs.size() < X64PR_MAX && scp->sem_stamp == SEM_WHILE_BODY)
	{
		LOG();
		Scope *while_scp = scp->parent;
//    	Scope *cond = while_scp->clds[0];
		get_scope_use_vr(while_scp, vrs, visitor);
	}

	if (vrs.size() < X64PR_MAX && scp->sem_stamp != SEM_FUNC_DEFINE)
	{
		get_future_use_vr(scp->parent, scp->idx + 1, vrs, visitor);
	}
}

static void then_pre_work(Scope *else_branch)
{
	for (int vr = 0; vr < else_branch->outer_symb_read.size(); vr++)
	{
		int cnt = else_branch->outer_symb_read[vr];
		if (cnt == 0)
			continue;
		Symbol *symb = (vr_declare_manager.declare_at[vr])->symb;
		symb->use_cnt -= cnt;
	}
}
static void then_end_work(Scope *else_branch)
{
	for (int vr = 0; vr < else_branch->outer_symb_read.size(); vr++)
	{
		int cnt = else_branch->outer_symb_read[vr];
		if (cnt == 0)
			continue;
		Symbol *symb = (vr_declare_manager.declare_at[vr])->symb;
		symb->use_cnt += cnt;
	}
}

void ra_wave_if(Scope *if_scp)
{
	LOG("scp %d", if_scp->id);

	Scope *cond = if_scp->clds[0];
	Scope *then_branch = if_scp->clds[1];
	Scope *else_branch = if_scp->clds[2];
	Scope *if_tail = if_scp->clds[3];
	Scope *then_tail = get_inner_tail_of_scope(then_branch);
	Scope *else_tail = get_inner_tail_of_scope(else_branch);

	// cond
	ra_wave_scope(cond);
	vector<VrToPr> after_cond_vr_state = vr_state;
	vector<PrToVr> after_cond_pr_state = pr_state;

	// then
	then_pre_work(else_branch);
	ra_wave_scope(then_branch);

	vector<VrToPr> after_then_vr_state = vr_state;
	vector<PrToVr> after_then_pr_state = pr_state;

	then_end_work(else_branch);
	vr_state = after_cond_vr_state;
	pr_state = after_cond_pr_state;

	// else
	ra_wave_scope(else_branch);

//	int after_else_swap_pr = get_swap_pr(then_branch->basic_block);
//	assert(after_else_swap_pr != X64PR_MAX);

//	vector<VrToPr> after_else_vr_state = vr_state;
//	vector<PrToVr> after_else_pr_state = pr_state;

// future pr use
	if_tail_vr.clear();
	if_tail_vr.resize(vr_size, 0);

	for (int vr = 0; vr < vr_size; vr++)
	{
		if(after_then_vr_state[vr].pr != X64PR_MAX)
			if_tail_vr[vr]++;
		if(vr_state[vr].pr != X64PR_MAX)
			if_tail_vr[vr]++;
	}

	vector<int> future_vr;
	get_future_use_vr(if_scp->parent, if_scp->idx + 1, future_vr, if_scp->id);

//	 not really good, because while and if have branch

	printf("\n================== if%d future_vr ==================\n", if_scp->id);
	for (auto vr : future_vr)
		printf("  %s=%%%d", vr_name(vr), vr);
	printf("\n");

	if (future_vr.size() > X64PR_MAX - 1)
		future_vr.resize(X64PR_MAX - 1);

	dump_vr_state(after_then_vr_state, "after_then_vr_state" + to_string(if_scp->id));
	for (auto &r : after_then_pr_state)
	{
		int vr = r.vr;
		if (vr == INVALID_VR)
			continue;

		if (std::find(future_vr.begin(), future_vr.end(), vr) == future_vr.end())
		{
			Scope *tail = get_inner_tail_of_scope(then_branch);
			auto &mcs = tail->basic_block->x64mc_alloc_wave;
			spill_vr(after_then_vr_state, after_then_pr_state, vr, mcs, " then");
		}
	}

	int swap_pr = X64PR_MAX;
	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		if (after_then_pr_state[pr].vr == INVALID_VR)
		{
			swap_pr = pr;
			break;
		}
	}
	assert(swap_pr != X64PR_MAX);
//	int after_then_swap_pr = get_swap_pr(then_branch->basic_block);
//	assert(after_then_swap_pr != X64PR_MAX);

//	for (auto vr : future_vr)
//	{
//		if(after_then_vr_state[vr].pr == X64PR_MAX)
//
//	}
	dump_vr_state(after_then_vr_state, "update after_then_vr_state" + to_string(if_scp->id));

	dump_vr_state(vr_state, "vr_state" + to_string(if_scp->id));
	for (auto &r : pr_state)
	{
		int vr = r.vr;
		if (vr == INVALID_VR)
			continue;

		if (std::find(future_vr.begin(), future_vr.end(), vr) == future_vr.end())
		{
			Scope *tail = get_inner_tail_of_scope(else_branch);
			auto &mcs = tail->basic_block->x64mc_alloc_wave;
			spill_vr(vr_state, pr_state, vr, mcs, " else");
		}
	}
	dump_vr_state(vr_state, "update vr_state" + to_string(if_scp->id));

		while_recover_pr(
		    after_then_vr_state,
		    after_then_pr_state,
		    swap_pr,
		    vr_state,
		    pr_state,
		    get_inner_tail_of_scope(else_branch));


	//		while_recover_pr(
//		    vr_state,
//		    pr_state,
//		    after_then_vr_state,
//		    after_then_pr_state,
//		    get_inner_tail_of_scope(then_branch),
//		    after_else_swap_pr);
//	}
//
	for (int vr = 0; vr < vr_size; vr++)
	{
		if (after_then_vr_state[vr].pr != X64PR_MAX)
			vr_state[vr].dirty |= after_then_vr_state[vr].dirty;
	}

}
 *
 *
 */

