/////////////////////////////////////////////////////

/*
 * x64_alloc_if.cpp
 *
 *  Created on: 2026年10月8日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

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

	int after_then_pr = 0;
	int after_else_pr = 0;
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		if (after_then_vr_state[vr].pr != X64PR_MAX)
			after_then_pr++;
		if (vr_state[vr].pr != X64PR_MAX)
			after_else_pr++;
	}

//	dump_vr_state(after_then_vr_state, "after_then_vr_state" + to_string(if_scp->id));
//	dump_vr_state(vr_state, "vr_state" + to_string(if_scp->id));

	if (after_then_pr > after_else_pr)
	{
		recover_pr_state(
		    after_then_vr_state,
		    after_then_pr_state,
		    after_then_swap_pr,
		    vr_state,
		    pr_state,
		    get_inner_tail_of_scope(else_branch));
	}
	else
	{
		recover_pr_state(
		    vr_state,
		    pr_state,
		    after_else_swap_pr,
		    after_then_vr_state,
		    after_then_pr_state,
		    get_inner_tail_of_scope(then_branch));
	}

	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		if (after_then_vr_state[vr].pr != X64PR_MAX)
			vr_state[vr].dirty |= after_then_vr_state[vr].dirty;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//vector<int> if_tail_vr;
//
//static void get_scope_use_vr(Scope *scp, vector<int> &vrs, int visitor)
//{
//	LOG("%s", scp->name.c_str());
//	if (vrs.size() == X64PR_MAX || scp->visitor == visitor)
//		return;
//
//	scp->visitor = visitor;
//
//	auto &mcs = scp->basic_block->x64mc_schedu;
//	for (auto mc : mcs)
//	{
//		for (int i = 0; i < 3; i++)
//		{
//			int vr = mc.vr_list[i];
//			if (vr < 0)
//				continue;
//
//			if (if_tail_vr[vr] > 0)
//			{
//				if (std::find(vrs.begin(), vrs.end(), vr) == vrs.end())
//					vrs.push_back(vr);
//			}
//
//			printf("vrs.size %ld \n", vrs.size());
//
//			if (vrs.size() == X64PR_MAX)
//				return;
//		}
//	}
//
//	for (Scope *p : scp->clds)
//	{
//		if (vrs.size() < X64PR_MAX)
//			get_scope_use_vr(p, vrs, visitor);
//	}
//
//}
//static void get_future_use_vr(Scope *scp, int cld_idx,
//    vector<int> &vrs, int visitor)
//{
//	if (!scp || vrs.size() == X64PR_MAX)
//		return;
//
//	for (int i = cld_idx; i < scp->clds.size(); i++)
//	{
//		if (vrs.size() == X64PR_MAX)
//			return;
//
//		Scope *p = scp->clds[i];
//		get_scope_use_vr(p, vrs, visitor);
//	}
//
//	if (vrs.size() < X64PR_MAX && scp->sem_stamp == SEM_WHILE_BODY)
//	{
//		LOG();
//		Scope *while_scp = scp->parent;
////    	Scope *cond = while_scp->clds[0];
//		get_scope_use_vr(while_scp, vrs, visitor);
//	}
//
//	if (vrs.size() < X64PR_MAX && scp->sem_stamp != SEM_FUNC_DEFINE)
//	{
//		get_future_use_vr(scp->parent, scp->idx + 1, vrs, visitor);
//	}
//}
//
//void ra_wave_if2(Scope *if_scp)
//{
//	LOG("scp %d", if_scp->id);
//
//	Scope *cond = if_scp->clds[0];
//	Scope *then_branch = if_scp->clds[1];
//	Scope *else_branch = if_scp->clds[2];
//	Scope *if_tail = if_scp->clds[3];
//	Scope *then_tail = get_inner_tail_of_scope(then_branch);
//	Scope *else_tail = get_inner_tail_of_scope(else_branch);
//
//	// cond
//	ra_wave_scope(cond);
//	vector<VrToPr> after_cond_vr_state = vr_state;
//	vector<PrToVr> after_cond_pr_state = pr_state;
//
//	// then
//	then_pre_work(else_branch);
//	ra_wave_scope(then_branch);
//
//	vector<VrToPr> after_then_vr_state = vr_state;
//	vector<PrToVr> after_then_pr_state = pr_state;
//
//	then_end_work(else_branch);
//	vr_state = after_cond_vr_state;
//	pr_state = after_cond_pr_state;
//
//	// else
//	ra_wave_scope(else_branch);
//
////	int after_else_swap_pr = get_swap_pr(then_branch->basic_block);
////	assert(after_else_swap_pr != X64PR_MAX);
//
////	vector<VrToPr> after_else_vr_state = vr_state;
////	vector<PrToVr> after_else_pr_state = pr_state;
//
//// future pr use
//	if_tail_vr.clear();
//	if_tail_vr.resize(vr_declare_manager.size(), 0);
//
//	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
//	{
//		if (after_then_vr_state[vr].pr != X64PR_MAX)
//			if_tail_vr[vr]++;
//		if (vr_state[vr].pr != X64PR_MAX)
//			if_tail_vr[vr]++;
//	}
//
//	vector<int> future_vr;
//#if 0
//	get_future_use_vr(if_scp->parent, if_scp->idx + 1, future_vr, if_scp->id);
//#else
//	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
//	{
//		if (after_then_vr_state[vr].pr != X64PR_MAX
//		    && vr_state[vr].pr != X64PR_MAX)
//			future_vr.push_back(vr);
//	}
//#endif
//
////	 not really good, because while and if have branch
//
//	printf("\n================== if%d future_vr ==================\n", if_scp->id);
//	for (auto vr : future_vr)
//		printf("  %s=%%%d", vr_name(vr), vr);
//	printf("\n");
//
//	if (future_vr.size() > X64PR_MAX - 1)
//		future_vr.resize(X64PR_MAX - 1);
//
//	dump_vr_state(after_then_vr_state, "after_then_vr_state" + to_string(if_scp->id));
//	for (auto &r : after_then_pr_state)
//	{
//		int vr = r.vr;
//		if (vr == INVALID_VR)
//			continue;
//
//		if (std::find(future_vr.begin(), future_vr.end(), vr) == future_vr.end())
//		{
//			Scope *tail = get_inner_tail_of_scope(then_branch);
//			auto &mcs = tail->basic_block->x64mc_alloc_wave;
//			spill_vr(after_then_vr_state, after_then_pr_state, vr, mcs, " then");
//		}
//	}
//
//	int swap_pr = X64PR_MAX;
//	for (int pr = R10D; pr < X64PR_MAX; pr++)
//	{
//		if (after_then_pr_state[pr].vr == INVALID_VR)
//		{
//			swap_pr = pr;
//			break;
//		}
//	}
//	assert(swap_pr != X64PR_MAX);
////	int after_then_swap_pr = get_swap_pr(then_branch->basic_block);
////	assert(after_then_swap_pr != X64PR_MAX);
//
////	for (auto vr : future_vr)
////	{
////		if(after_then_vr_state[vr].pr == X64PR_MAX)
////
////	}
//	dump_vr_state(after_then_vr_state, "update after_then_vr_state" + to_string(if_scp->id));
//
//	dump_vr_state(vr_state, "vr_state" + to_string(if_scp->id));
//	for (auto &r : pr_state)
//	{
//		int vr = r.vr;
//		if (vr == INVALID_VR)
//			continue;
//
//		if (std::find(future_vr.begin(), future_vr.end(), vr) == future_vr.end())
//		{
//			Scope *tail = get_inner_tail_of_scope(else_branch);
//			auto &mcs = tail->basic_block->x64mc_alloc_wave;
//			spill_vr(vr_state, pr_state, vr, mcs, " else");
//		}
//	}
//	dump_vr_state(vr_state, "update vr_state" + to_string(if_scp->id));
//
//	while_recover_pr(
//	    after_then_vr_state,
//	    after_then_pr_state,
//	    swap_pr,
//	    vr_state,
//	    pr_state,
//	    get_inner_tail_of_scope(else_branch));
//
//	//		while_recover_pr(
////		    vr_state,
////		    pr_state,
////		    after_then_vr_state,
////		    after_then_pr_state,
////		    get_inner_tail_of_scope(then_branch),
////		    after_else_swap_pr);
////	}
////
//	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
//	{
//		if (after_then_vr_state[vr].pr != X64PR_MAX)
//			vr_state[vr].dirty |= after_then_vr_state[vr].dirty;
//	}
//
//}
