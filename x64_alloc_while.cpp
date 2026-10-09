/*
 * x64_alloc_while.cpp
 *
 *  Created on: 2026年10月4日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

static int get_swap_pr(BasicBlock *bb)
{
	for (int i = R10D; i < X64PR_MAX; i++)
	{
		if (pr2vr[i].vr == INVALID__VR)
			return i;
	}

	int swap_pr = X64PR_MAX;
	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		int vr = pr2vr[pr].vr;
		if (vr2pr[vr].u == VR_USAGE_READ)
		{
			swap_pr = pr;
			break;
		}
	}
	if (swap_pr == X64PR_MAX)
		swap_pr = X64PR_MAX - 1;

	spill_vr(vr2pr, pr2vr, pr2vr[swap_pr].vr, bb->x64mc_alloc_wave);
	pr2vr[swap_pr].vr == INVALID__VR;
	return swap_pr;
}
//static int while__get_pr(BasicBlock *bb, int score)
//{
//	for (int i = R10D; i < X64PR_MAX; i++)
//	{
//		if (pr2vr[i].vr == INVALID__VR)
//			return i;
//	}
//
//	float min_score = 100000;
//	int min_score_vr = INVALID__VR;
//
//	for (int pr = R10D; pr < X64PR_MAX; pr++)
//	{
//		int vr = pr2vr[pr].vr;
//		if (vr2pr[vr].score >= score)
//			continue;
//
//		if (score < min_score)
//		{
//			min_score = score;
//			min_score_vr = vr;
//		}
//	}
//	assert(min_score_vr != INVALID__VR);
//
//	int pr = vr2pr[min_score_vr].pr;
//	spill_vr(bb->x64mc_alloc_wave, min_score_vr);
//	return pr;
//}
//static int while_get_pr__load_vr(BasicBlock *bb, int vr, int u, int mc_idx)
//{
//	/*
//	 * todo
//	 * mc_assign %1, %1
//	 * s1 == s2
//	 */
//	assert(u != VR_USEAGE_INVALID);
//	bool need_load = 0;
//
//	if (vr2pr[vr].pr == X64PR_MAX)
//	{
//		int pr = wave_get_pr(bb, mc_idx);
//		vr2pr[vr].pr = pr;
//		vr2pr[vr].u = u | vr_usage[vr];
//		pr2vr[pr].vr = vr;
//		need_load = (u & VR_USAGE_READ);
//	}
//	// todo, if (!(vr2pr[vr].u & VR_USAGE_WRITE) && (u & VR_USAGE_WRITE) ?
//// 	// else: VR 已经在寄存器中，值有效 —— 永远不需要 MC_LD！
//// 	else if (!(vr2pr[vr].u & VR_USAGE_READ)
//// 	    && (u & VR_USAGE_READ))
//// 	{
//// 		need_load = 1;
//// 	}
//	else
//	{
//		vr2pr[vr].u |= u | vr_usage[vr];
//	}
//
//	if (need_load)
//	{
//		X64mc mc(MC_LD, vr);
//		mc.pr1 = vr2pr[vr].pr;
//		mc.ori_sem = "alloc";
//		bb->x64mc_alloc_wave.push_back(mc);
//	}
//
//	return vr2pr[vr].pr;
//}

void while__pre_work(Scope *scp)
{
	for (int vr = 0; vr < scp->outer_symb_read.size(); vr++)
	{
		int cnt = scp->outer_symb_read[vr];
		if (cnt == 0)
			continue;
		Ast *p = vr_declare_manager.declare_at[vr];
		Symbol *symb = p->symb;
		symb->use_cnt += cnt;
	}
	for (int vr = 0; vr < scp->outer_symb_write.size(); vr++)
	{
		int cnt = scp->outer_symb_write[vr];
		if (cnt == 0)
			continue;
		Ast *p = vr_declare_manager.declare_at[vr];
		Symbol *symb = p->symb;
		symb->use_cnt += cnt;
	}
}
void while__end_work(Scope *scp)
{
	for (int vr = 0; vr < scp->outer_symb_read.size(); vr++)
	{
		int cnt = scp->outer_symb_read[vr];
		if (cnt == 0)
			continue;
		Ast *p = vr_declare_manager.declare_at[vr];
		Symbol *symb = p->symb;
		symb->use_cnt -= cnt;
	}
	for (int vr = 0; vr < scp->outer_symb_write.size(); vr++)
	{
		int cnt = scp->outer_symb_write[vr];
		if (cnt == 0)
			continue;
		Ast *p = vr_declare_manager.declare_at[vr];
		Symbol *symb = p->symb;
		symb->use_cnt -= cnt;
	}

	for (int vr = 0; vr < scp->outer_symb_read.size(); vr++)
	{
		if (scp->outer_symb_read[vr] > 0)
			check_dead_vr(scp, vr);
	}
	for (int vr = 0; vr < scp->outer_symb_write.size(); vr++)
	{
		if (scp->outer_symb_write[vr] > 0)
			check_dead_vr(scp, vr);
	}
}
void dump_vr2pr(const vector<VrToPr> &vr_state, const string &tag)
{
	printf("===== %s\n", tag.c_str());
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		int pr = vr_state[vr].pr;
		if (pr != X64PR_MAX)
			printf("%s=%%%d %s  ", vr_declare_manager.declare_at[vr]->symb->unique_name.c_str()
			    , vr, pr_name[pr]);
	}
	printf("\n	dirty: ");
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		int pr = vr_state[vr].pr;
		if (pr != X64PR_MAX)
			printf("	%%%d %d", vr, vr_state[vr].dirty);
	}
	printf("\n");
}

void while__recover_pr(Scope *while_scp,
    const vector<VrToPr> &before_vr2pr,
    const vector<PrToVr> &before_pr2vr,
    vector<VrToPr> &now_vr2pr,
    vector<PrToVr> &now_pr2vr,
    int swap_pr)
{
	Scope *body_inner_tail = get_inner_tail_of_scope(while_scp->clds[1]);
	BasicBlock *bb = (BasicBlock*) body_inner_tail->basic_block;
	vector<X64mc> &x64mcs = bb->x64mc_alloc_wave;
	X64mc mc;
	(void) vr2pr;
	(void) pr2vr;

	int spilled = 0;
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		if (before_vr2pr[vr].pr == X64PR_MAX && now_vr2pr[vr].pr != X64PR_MAX)
		{
//			LOG("scp %s, recover spill %s=%%%d-%s", scp->name.c_str(),
//			    vr_declare_manager.declare_at[vr]->symb->unique_name.c_str(),
//			    vr, pr_name[pr]);
			spill_vr(now_vr2pr, now_pr2vr, vr, x64mcs, " while recover");
			spilled++;
		}
	}
	LOG("scp %s spilled: %d, swap_pr=%d %s", body_inner_tail->name.c_str(), spilled, swap_pr, pr_name[swap_pr]);

	if (now_pr2vr[swap_pr].vr != INVALID__VR)
	{
		int free_pr = X64PR_MAX;
		for (int i = R10D; i < X64PR_MAX; i++)
		{
			if (now_pr2vr[i].vr == INVALID__VR)
			{
				free_pr = i;
				break;
			}
		}
		assert(free_pr != X64PR_MAX /* && free_pr != swap_pr */);

		int vr = now_pr2vr[swap_pr].vr;
		string &name = vr_declare_manager.declare_at[vr]->symb->unique_name;

		mc = X64mc(MC_ASSIGN);
		mc.pr1 = free_pr;
		mc.pr2 = swap_pr;
		mc.ori_sem = "while swap_pr, assign " + name + "=%" + to_string(vr);
		x64mcs.push_back(mc);

		now_vr2pr[vr].pr = free_pr;
		now_pr2vr[free_pr].vr = vr;

		now_pr2vr[swap_pr].vr = INVALID__VR;
	}
	if (swap_pr == X64PR_MAX)
		ERR();

	for (int vr = 0; vr < before_vr2pr.size(); vr++)
	{
		int A = before_vr2pr[vr].pr;

		if (A == X64PR_MAX)
			continue;

		int B = now_vr2pr[vr].pr;

		if (B == A)
			continue;

		string &name = vr_declare_manager.declare_at[vr]->symb->unique_name;

		if (B != X64PR_MAX)
		{
			int curr_vr_have_A = now_pr2vr[A].vr;

			// (curr A is busy && B == PR)
			if (curr_vr_have_A != INVALID__VR)
			{
				assert(now_pr2vr[swap_pr].vr == INVALID__VR);
				auto &curr_vr_struct_have_A = now_vr2pr[curr_vr_have_A];

				mc = X64mc(MC_ASSIGN);
				mc.pr1 = swap_pr;
				mc.pr2 = A;
				mc.ori_sem = "while 1, assign " + name + "=%" + to_string(vr);
				x64mcs.push_back(mc);

				mc = X64mc(MC_ASSIGN);
				mc.pr1 = A;
				mc.pr2 = B;
				mc.ori_sem = "while 1, assign " + name + "=%" + to_string(vr);
				x64mcs.push_back(mc);

				mc = X64mc(MC_ASSIGN);
				mc.pr1 = B;
				mc.pr2 = swap_pr;
				mc.ori_sem = "while 1, assign " + name + "=%" + to_string(vr);
				x64mcs.push_back(mc);

				now_vr2pr[curr_vr_have_A].pr = B;
				now_pr2vr[B].vr = curr_vr_have_A;
			}
			else	//  (curr A is free && B == PR)
			{
				mc = X64mc(MC_ASSIGN);
				mc.pr1 = A;
				mc.pr2 = B;
				mc.ori_sem = "while 2, assign " + name + "=%" + to_string(vr);
				x64mcs.push_back(mc);

				//
				now_pr2vr[B].vr = INVALID__VR;
			}
		}
		else	// B == X64PR_MAX
		{
			int curr_vr_have_A = now_pr2vr[A].vr;

			if (curr_vr_have_A != INVALID__VR)
			{
				// must have another free pr
				int free_pr = X64PR_MAX;
				for (int pr = R10D; pr < X64PR_MAX; pr++)
				{
					if (now_pr2vr[pr].vr == INVALID__VR && pr != swap_pr)
					{
						free_pr = pr;
						break;
					}
				}
				if (free_pr == X64PR_MAX)
					ERR("%d", before_vr2pr[curr_vr_have_A].pr);

				auto &curr_vr_A_struct = now_vr2pr[curr_vr_have_A];

				mc = X64mc(MC_ASSIGN);
				mc.pr1 = free_pr;
				mc.pr2 = A;
				mc.ori_sem = "while 3, assign " + name + "=%" + to_string(vr)
				    + ", move %s" + to_string(curr_vr_have_A);
				x64mcs.push_back(mc);

				now_vr2pr[curr_vr_have_A].pr = free_pr;
				now_pr2vr[free_pr].vr = curr_vr_have_A;
			}
			//  (curr_vr_A == INVALID__VR)
			mc = X64mc(MC_LD, vr);
			mc.pr1 = A;
			mc.ori_sem = "while 4, ld ";
			x64mcs.push_back(mc);
		}

// now_vr2pr[vr] = before_vr2pr[vr];
		now_vr2pr[vr].pr = A;
		now_pr2vr[A].vr = vr;
	}
}

void ra_wave__while(Scope *while_scp)
{
	//	LOG("scp %s", scp->name.c_str());

	while__pre_work(while_scp);

	Scope *cond = while_scp->clds[0];
	Scope *body = while_scp->clds[1];

	/*
	 * Invariant:
	 *
	 * before state has at least one free PR.
	 *
	 * Before recovery, spill every VR that:
	 *     before.pr == MAX
	 *     now.pr    != MAX
	 *
	 * Therefore after the spill phase, at least one PR is free
	 * in the current state, and it can be used as recovery scratch.
	 *
	 */

	int swap_pr = X64PR_MAX;
	swap_pr = get_swap_pr((BasicBlock*) while_scp->basic_block);
	if (swap_pr == X64PR_MAX)
		ERR();

// try alloc cond write
//	for(int vr : scp->outer_symb_write)
//	{
//		for(int pr : )
//	}
// try alloc body write
// try alloc cond read
// try alloc body read
// select cond pr
//	spill_vr(bb->x64mc_alloc_wave, swap_vr);

////////////////////////////////////////////////////////////
// while cond
//	Scope *cond = while_scp->clds[0];
//	Scope *body = while_scp->clds[1];
//	cond->addtional_symb_usage.resize(vr_declare_manager.size(), VR_USAGE_INVALID);
//	for (int i = 0; i < vr_declare_manager.size(); i++)
//	{
//		if (cond->outer_symb_read[i] > 0 && body->outer_symb_write[i] > 0)
//			cond->addtional_symb_usage[i] |= VR_USAGE_WRITE;
//	}
	vector<VrToPr> pre_cond_vr2pr = vr2pr;
	vector<PrToVr> pre_cond_pr2vr = pr2vr;

	ra_wave__scope(cond);
//	if(pr2vr[swap_pr].vr != INVALID__VR)
//		ERR();

	vector<VrToPr> after_cond_vr2pr = vr2pr;
	vector<PrToVr> after_cond_pr2vr = pr2vr;

	////////////////////////////////////////////////////////////
	// while body
	ra_wave__scope(body);

	vector<VrToPr> after_body_vr2pr = vr2pr;
	vector<PrToVr> after_body_pr2vr = pr2vr;

	////////////////////////////////////////////////////////////
	vr2pr = after_cond_vr2pr;
	pr2vr = after_cond_pr2vr;

	////////////////////////////////////////////////////////////
	printf("\n============== swap %d ==============\n", while_scp->id);
	dump_vr2pr(pre_cond_vr2pr, "pre cond " + to_string(while_scp->id));
	dump_vr2pr(after_cond_vr2pr, "after cond " + to_string(while_scp->id));
	dump_vr2pr(after_body_vr2pr, "after body " + to_string(while_scp->id));

	while__recover_pr(while_scp,
	    pre_cond_vr2pr, pre_cond_pr2vr,
	    after_body_vr2pr, after_body_pr2vr, swap_pr);

	dump_vr2pr(after_body_vr2pr, "after body swaped " + to_string(while_scp->id));
	dump_vr2pr(vr2pr, "vr2pr " + to_string(while_scp->id));
	printf("\n");

	int re_alloc = 0;
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		if (after_body_vr2pr[vr].pr != X64PR_MAX
		    && after_body_vr2pr[vr].dirty == 1
		    && pre_cond_vr2pr[vr].dirty == 0)
		{
			pre_cond_vr2pr[vr].dirty = 1;
			re_alloc++;
		}
	}
	if (re_alloc)
	{
		vr2pr = pre_cond_vr2pr;
		pr2vr = pre_cond_pr2vr;
		reset_scope_symb_consume_cnt(cond, -1);

		LOG("scp=%d, re_alloc=%d", while_scp->id, re_alloc);
		dump_vr2pr(vr2pr, "before re_alloc " + to_string(while_scp->id));

		ra_wave__scope(cond);

		dump_vr2pr(vr2pr, "after re_alloc " + to_string(while_scp->id));
	}

	/*
	 * If body dirtys a VR, propagating dirty to cond then re-alloc cond fixes cond's problem.

	 For a VR that has a PR in both pre_cond and after_body:

	   - If cond spilled this VR (MC_ST in cond_mcs), body does NOT need to spill it again.
	     Reason: cond already wrote the latest value to memory. Body's subsequent MC_LD
	     reads that same value, so even if body later dirtys it, cond's spill already
	     guarantees memory is up-to-date — body does not need to write again.

	   - If cond wrote this VR (MC_ASSIGN / MC_ST in cond_mcs), body does NOT need to
	     spill it either. Reason: cond always ends with an MC_ST for any VR it wrote
	     (dirty VRs get spilled). So cond will overwrite the stack slot with the final
	     value regardless. Body's dirty write is covered by cond's eventual spill.

	 Therefore: if cond has EVER spilled a VR (MC_ST present in cond_mcs), body never
	 needs to spill it — cond guarantees the memory value is correct.
	 *
	 */

	BasicBlock *cond_bb = (BasicBlock*) cond->basic_block;
	vector<X64mc> &cond_mcs = cond_bb->x64mc_alloc_wave;
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		if (after_body_vr2pr[vr].pr == X64PR_MAX || !after_body_vr2pr[vr].dirty)
			continue;

		bool cond_will_spill_this_dirty_vr = 0;
		for (auto &cond_mc : cond_mcs)
		{
			if (cond_mc.mc_stamp == MC_ST && cond_mc.s1 == vr)
			{
				cond_will_spill_this_dirty_vr = 1;
				break;
			}
		}
		if(cond_will_spill_this_dirty_vr == 0)
		{
			Scope *body_inner_tail = get_inner_tail_of_scope(while_scp->clds[1]);
			BasicBlock *bb = (BasicBlock*) body_inner_tail->basic_block;
			vector<X64mc> &x64mcs = bb->x64mc_alloc_wave;

			X64mc mc(MC_ST, vr);
			mc.pr1 = after_body_vr2pr[vr].pr;
			mc.ori_sem = "while recover st";
			x64mcs.push_back(mc);
		}
	}

	while__end_work(while_scp);
}

static void get_future_use_vr(Scope *scp, std::set<int> &vrs)
{
	auto &mcs = ((BasicBlock*) scp->basic_block)->x64mc_schedu;
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
void ra_wave__if(Scope *if_scp)
{
	//	LOG("scp %s", scp->name.c_str());

	// cond
	Scope *cond = if_scp->clds[0];
	ra_wave__scope(cond);

	vector<VrToPr> cond_vr2pr = vr2pr;
	vector<PrToVr> cond_pr2vr = pr2vr;

	// then
	Scope *then_branch = if_scp->clds[1];
	ra_wave__scope(then_branch);

	vector<VrToPr> then_vr2pr = vr2pr;
	vector<PrToVr> then_pr2vr = pr2vr;

	// else
	vr2pr = cond_vr2pr;
	pr2vr = cond_pr2vr;
	Scope *else_branch = if_scp->clds[2];
	ra_wave__scope(else_branch);

	// future pr use
	std::set<int> future_vr;
	Scope *tail = if_scp->clds[3];
	Scope *pa = if_scp->parent;
	Scope *next = 0;
	for (int i = 0; i < pa->clds.size(); i++)
	{
		if (pa->clds[i] == if_scp)
		{
			if (i + 1 < pa->clds.size())
				next = pa->clds[i + 1];
			else
				next = nullptr;
			break;
		}
	}
	// not really good, while and if have branch
	get_future_use_vr(next, future_vr);
	printf("if_scp %d future_vr: ", if_scp->id);
	for (auto vr : future_vr)
		printf(" %%%d", vr);
	printf("\n");

	dump_vr2pr(then_vr2pr, "then before" + to_string(if_scp->id));
	int then_branch_prs = 0;
	for (auto &r : then_pr2vr)
	{
		int vr = r.vr;
		if (vr == INVALID__VR)
			continue;
		then_branch_prs++;

		if (std::find(future_vr.begin(), future_vr.end(), vr) == future_vr.end())
		{
			auto &mcs = ((BasicBlock*) then_branch->basic_block)->x64mc_alloc_wave;
			spill_vr(then_vr2pr, then_pr2vr, vr, mcs, "then");
			then_branch_prs--;
		}
	}
	dump_vr2pr(then_vr2pr, "then " + to_string(if_scp->id));

	dump_vr2pr(vr2pr, "else before" + to_string(if_scp->id));
	int else_branch_prs = 0;
	for (auto &r : pr2vr)
	{
		int vr = r.vr;
		if (vr == INVALID__VR)
			continue;
		else_branch_prs++;

		if (std::find(future_vr.begin(), future_vr.end(), vr) == future_vr.end())
		{
			auto &mcs = ((BasicBlock*) else_branch->basic_block)->x64mc_alloc_wave;
			spill_vr(vr2pr, pr2vr, vr, mcs, "else");
			else_branch_prs--;
		}
	}
	dump_vr2pr(vr2pr, "else " + to_string(if_scp->id));

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

	Scope *then_tail = get_inner_tail_of_scope(then_branch);
	Scope *else_tail = get_inner_tail_of_scope(else_branch);

	BasicBlock *cond_bb = (BasicBlock*) cond->basic_block;
	BasicBlock *tail_bb = (BasicBlock*) tail->basic_block;

	spill_all_pr(tail_bb);
//	int swap_pr = X64PR_MAX;
//	swap_pr = get_swap_pr((BasicBlock*) if_scp->basic_block);
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
//	BasicBlock *cond_bb = (BasicBlock*) cond->basic_block;
//	BasicBlock *bs_bb = (BasicBlock*) body_tial->basic_block;
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

