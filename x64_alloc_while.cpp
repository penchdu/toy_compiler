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

	spill_vr(bb->x64mc_alloc_wave, pr2vr[swap_pr].vr);
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
			check_and_clear_vr(scp, vr);
	}
	for (int vr = 0; vr < scp->outer_symb_write.size(); vr++)
	{
		if (scp->outer_symb_write[vr] > 0)
			check_and_clear_vr(scp, vr);
	}
}
void dump_vr2pr(const vector<VrToPr> &vp, const string &tag)
{
	printf("===== %s\n", tag.c_str());
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		int pr = vp[vr].pr;
		if (pr != X64PR_MAX)
			printf("%s=%%%d %s    ", vr_declare_manager.declare_at[vr]->symb->unique_name.c_str()
			    , vr, pr_name[pr]);
	}
	printf("\n");
}

static void while__recover_pr(Scope *scp,
    vector<VrToPr> &before_vr2pr,
    vector<PrToVr> &before_pr2vr,
    vector<VrToPr> &now_vr2pr,
    vector<PrToVr> &now_pr2vr,
    int swap_pr)
{
	BasicBlock *bb = (BasicBlock*) scp->basic_block;
	vector<X64mc> &x64mcs = bb->x64mc_alloc_wave;
	X64mc mc;
	(void) vr2pr;
	(void) pr2vr;

	int spilled = 0;
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		if (before_vr2pr[vr].pr == X64PR_MAX && now_vr2pr[vr].pr != X64PR_MAX)
		{
//			spill_vr(x64mcs, vr, "while ");
			int &pr = now_vr2pr[vr].pr;
			int &u = now_vr2pr[vr].u;
			int need_st = (u | vr_usage[vr]) & VR_USAGE_WRITE;

			assert(pr != X64PR_MAX);
			assert(u != VR_USEAGE_INVALID);
			assert(now_pr2vr[pr].vr == vr);

			LOG("scp %s, recover spill %s=%%%d-%s %d", scp->name.c_str(),
			    vr_declare_manager.declare_at[vr]->symb->unique_name.c_str(),
			    vr, pr_name[pr], need_st);

			if (need_st)
			{
				X64mc mc(MC_ST, vr);
				mc.pr1 = (int) pr;
				mc.ori_sem = "while recover spill";
				x64mcs.push_back(mc);
			}

			now_pr2vr[pr].vr = INVALID__VR;
			pr = X64PR_MAX;
			u = VR_USEAGE_INVALID;
			spilled++;
		}
	}

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

	LOG("scp %s spilled: %d, swap_pr=%d %s", scp->name.c_str(), spilled, swap_pr, pr_name[swap_pr]);
//	Scope *cond = scp->clds[0];
//	auto & cond_read = cond->outer_symb_read;
//	auto & cond_write = cond->outer_symb_write;

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

	Scope *cond_scp = while_scp->clds[0];
	Scope *body_scp = while_scp->clds[1];

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

//	pr2vr[swap_pr].vr = INVALID__VR;
//	vr2pr[swap_pr].

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
	for (int i = 0; i < vr_declare_manager.size(); i++)
	{
		if (cond_scp->outer_symb_read[i] > 0
		    && body_scp->outer_symb_write[i] > 0)
			vr_usage[i] |= VR_USAGE_WRITE;
	}

	vector<VrToPr> pre_cond_vr2pr = vr2pr;
	vector<PrToVr> pre_cond_pr2vr = pr2vr;

	ra_wave__scope(while_scp->clds[0]);

	vector<VrToPr> after_cond_vr2pr = vr2pr;
	vector<PrToVr> after_cond_pr2vr = pr2vr;

	////////////////////////////////////////////////////////////
	// while body
	ra_wave__scope(while_scp->clds[1]);

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

	assert(while_scp->clds[2]->sem_stamp == SEM_WHILE_BODY_suffix);
	Scope *body_suffix = while_scp->clds[2];

	BasicBlock *cond_bb = (BasicBlock*) cond_scp->basic_block;
	BasicBlock *bs_bb = (BasicBlock*) body_suffix->basic_block;
	vector<X64mc> &cond_mcs = cond_bb->x64mc_alloc_wave;
	for (auto &cond_mc : cond_mcs)
	{
		if (cond_mc.mc_stamp != MC_LD)
			continue;

		int vr = cond_mc.s1;
		int pr = cond_mc.pr1;

		assert(pr != X64PR_MAX);

		if (body_scp->outer_symb_write[vr] == 0)
			continue;

		//  | vr_usage ?
		if (after_body_vr2pr[vr].pr == X64PR_MAX)
			continue;

		if (!(after_body_vr2pr[vr].u & VR_USAGE_WRITE))
			continue;

		LOG("COND_LD: while=%d vr=%d pr=%s pre=%s after_body=%s u=%d body_write=%d",
		    while_scp->id, vr, pr_name[cond_mc.pr1],
		    pre_cond_vr2pr[vr].pr == X64PR_MAX ? "MAX" : pr_name[pre_cond_vr2pr[vr].pr],
		    after_body_vr2pr[vr].pr == X64PR_MAX ? "MAX" : pr_name[after_body_vr2pr[vr].pr],
		    after_body_vr2pr[vr].u,
		    body_scp->outer_symb_write[vr]);

//	    assert(after_body_vr2pr[vr].pr == );
		// todo, do st only if first op of vr in cond is ld
		X64mc mc(MC_ST, vr);
		mc.pr1 = after_body_vr2pr[vr].pr;
		mc.ori_sem = "while spill, cond-ld st";
		bs_bb->x64mc_alloc_wave.push_back(mc);
	}

	while__recover_pr(body_suffix,
	    pre_cond_vr2pr, pre_cond_pr2vr,
	    after_body_vr2pr, after_body_pr2vr, swap_pr);

	dump_vr2pr(after_body_vr2pr, "after body swaped " + to_string(while_scp->id));

	dump_vr2pr(vr2pr, "vr2pr " + to_string(while_scp->id));
	printf("\n");

//	BasicBlock *tail_bb = (BasicBlock*) while_scp->clds[3]->basic_block;
//	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
//	{
//	    if (body_scp->outer_symb_write[vr] == 0)   continue;
//	    if (cond_scp->outer_symb_write[vr] > 0)    continue;
//	    if (after_body_vr2pr[vr].pr == X64PR_MAX)   continue;
//	    if (!(after_body_vr2pr[vr].u & VR_USAGE_WRITE)) continue;
//
//	    X64mc mc(MC_ST, vr);
//	    mc.pr1 = after_body_vr2pr[vr].pr;
//	    mc.ori_sem = "while tail spill";
//	    tail_bb->x64mc_alloc_wave.push_back(mc);
//	}

	while__end_work(while_scp);

//	BasicBlock *tail_bb = (BasicBlock*) tail->basic_block;
//	assert(tail_bb);
//	spill_all_pr(tail_bb);
}
