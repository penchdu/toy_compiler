/*
 * x64_alloc_if.cpp
 *
 *  Created on: 2026年10月8日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

static void get_future_use_vr(Scope *scp, std::set<int> &vrs)
{
	auto &mcs = (scp->basic_block)->x64mc_schedu;
	for (auto mc : mcs)
	{
		for (int i = 0; i < 3; i++)
		{
			int vr = mc.vr_list[i];
			if (vr < 0)
				continue;

			vrs.insert(vr);
			if (vrs.size() == X64PR_MAX)
				return;
		}
	}

	for (Scope *p : scp->clds)
		get_future_use_vr(p, vrs);
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
//	vector<VrToPr> after_else_vr_state = vr_state;
//	vector<PrToVr> after_else_pr_state = pr_state;

	while_recover_pr(
		after_then_vr_state,
		after_then_pr_state,
		vr_state,
		pr_state,
		get_inner_tail_of_scope(else_branch),
		after_then_swap_pr);

	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		if(after_then_vr_state[vr].pr != X64PR_MAX)
			vr_state[vr].dirty |= after_then_vr_state[vr].dirty;
	}

	// future pr use
//	std::set<int> future_vr;

//	Scope *pa = if_scp->parent;
//	Scope *next = 0;
//	for (int i = 0; i < pa->clds.size(); i++)
//	{
//		if (pa->clds[i] == if_scp)
//		{
//			if (i + 1 < pa->clds.size())
//				next = pa->clds[i + 1];
//			else
//				next = nullptr;
//			break;
//		}
//	}
//	// not really good, while and if have branch
//	get_future_use_vr(next, future_vr);
//	printf("if_scp %d future_vr: ", if_scp->id);
//	for (auto vr : future_vr)
//		printf(" %%%d", vr);
//	printf("\n");
//
//	dump_vr2pr(then_vr2pr, "then before" + to_string(if_scp->id));
//	int then_branch_prs = 0;
//	for (auto &r : then_pr2vr)
//	{
//		int vr = r.vr;
//		if (vr == INVALID__VR)
//			continue;
//		then_branch_prs++;
//
//		if (std::find(future_vr.begin(), future_vr.end(), vr) == future_vr.end())
//		{
//			auto &mcs = then_branch->basic_block->x64mc_alloc_wave;
//			spill_vr(then_vr2pr, then_pr2vr, vr, mcs, " then");
//			then_branch_prs--;
//		}
//	}
//	dump_vr2pr(then_vr2pr, "then " + to_string(if_scp->id));
//
//	dump_vr2pr(vr2pr, "else before" + to_string(if_scp->id));
//	int else_branch_prs = 0;
//	for (auto &r : pr2vr)
//	{
//		int vr = r.vr;
//		if (vr == INVALID__VR)
//			continue;
//		else_branch_prs++;
//
//		if (std::find(future_vr.begin(), future_vr.end(), vr) == future_vr.end())
//		{
//			auto &mcs = else_branch->basic_block->x64mc_alloc_wave;
//			spill_vr(vr2pr, pr2vr, vr, mcs, " else");
//			else_branch_prs--;
//		}
//	}
//	dump_vr2pr(vr2pr, "else " + to_string(if_scp->id));

//	usleep(1000);
//	ERR();
//	if(then_branch_prs > else_branch_prs)
//	{
//		while__recover_pr(else_branch,
//			    then_vr2pr, then_pr2vr,
//			    vr2pr, pr2vr, swap_pr);
//	}
//	else
//	{
//
//	}

//	BasicBlock *cond_bb = cond->basic_block;
//	BasicBlock *tail_bb = tail->basic_block;

//	int swap_pr = X64PR_MAX;
//	swap_pr = get_swap_pr(if_scp->basic_block);
//	if (swap_pr == X64PR_MAX)
//		ERR();

//	for (int i = 0; i < vr_declare_manager.size(); i++)
//	{
//		if (cond->outer_symb_read[i] > 0
//		    && body->outer_symb_write[i] > 0)
//			vr_usage[i] |= VR_USAGE_WRITE;
//	}

////////////////////////////////////////////////////////////
//	printf("\n============== swap %d ==============\n", if_scp->id);
//	dump_vr2pr(pre_cond_vr2pr, "pre cond " + to_string(if_scp->id));
//	dump_vr2pr(after_cond_vr2pr, "after cond " + to_string(if_scp->id));
//	dump_vr2pr(after_then_vr2pr, "after body " + to_string(if_scp->id));
//
//	Scope *body_tial = get_inner_tail_of_scope(if_scp->clds[1]);
//
//	BasicBlock *cond_bb = cond->basic_block;
//	BasicBlock *bs_bb = body_tial->basic_block;
//	vector<X64mc> &cond_mcs = cond_bb->x64mc_alloc_wave;
//	for (auto &cond_mc : cond_mcs)
//	{
//		if (cond_mc.mc_stamp != MC_LD)
//			continue;
//
//		int vr = cond_mc.s1;
//		int pr = cond_mc.pr1;
//
//		assert(pr != X64PR_MAX);
//
//		if (body->outer_symb_write[vr] == 0)
//			continue;
//
//		//  | vr_usage ?
//		if (after_then_vr2pr[vr].pr == X64PR_MAX)
//			continue;
//
//		if (!(after_then_vr2pr[vr].u & VR_USAGE_WRITE))
//			continue;
//
//		LOG("COND_LD: while=%d vr=%d pr=%s pre=%s after_body=%s u=%d body_write=%d",
//		    if_scp->id, vr, pr_name[cond_mc.pr1],
//		    pre_cond_vr2pr[vr].pr == X64PR_MAX ? "MAX" : pr_name[pre_cond_vr2pr[vr].pr],
//		    after_then_vr2pr[vr].pr == X64PR_MAX ? "MAX" : pr_name[after_then_vr2pr[vr].pr],
//		    after_then_vr2pr[vr].u,
//		    body->outer_symb_write[vr]);
//
////	    assert(after_body_vr2pr[vr].pr == );
//		// todo, do st only if first op of vr in cond is ld
//		X64mc mc(MC_ST, vr);
//		mc.pr1 = after_then_vr2pr[vr].pr;
//		mc.ori_sem = "while spill, cond-ld st";
//		bs_bb->x64mc_alloc_wave.push_back(mc);
//	}
//
//	while__recover_pr(body_tial,
//	    pre_cond_vr2pr, pre_cond_pr2vr,
//	    after_then_vr2pr, after_then_pr2vr, swap_pr);
//
//	dump_vr2pr(after_then_vr2pr, "after body swaped " + to_string(if_scp->id));
//
//	dump_vr2pr(vr2pr, "vr2pr " + to_string(if_scp->id));
//	printf("\n");

//	while__end_work(if_scp);

}


