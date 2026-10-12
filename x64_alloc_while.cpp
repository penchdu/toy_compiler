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
		if (pr_state[i].vr == INVALID_VR)
			return i;
	}

	int swap_pr = X64PR_MAX;
	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		int vr = pr_state[pr].vr;
		if (vr_state[vr].u == VR_USAGE_READ)
		{
			swap_pr = pr;
			break;
		}
	}
	if (swap_pr == X64PR_MAX)
		swap_pr = X64PR_MAX - 1;

	spill_vr(vr_state, pr_state, pr_state[swap_pr].vr, bb->x64mc_alloc_wave);
	pr_state[swap_pr].vr == INVALID_VR;
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

	Scope *tail = scp->clds[2];
	auto &mcs = tail->basic_block->x64mc_alloc_wave;

	for (int vr = 0; vr < scp->outer_symb_read.size(); vr++)
	{
		if (scp->outer_symb_read[vr] > 0)
			check_dead_vr(scp, mcs, vr);
	}
	for (int vr = 0; vr < scp->outer_symb_write.size(); vr++)
	{
		if (scp->outer_symb_write[vr] > 0)
			check_dead_vr(scp, mcs, vr);
	}
}
void dump_vr_state(const vector<VrToPr> &vr_state, const string &tag)
{
	printf("===== %s\n", tag.c_str());
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		int pr = vr_state[vr].pr;
		if (pr != X64PR_MAX)
			printf("%s=%%%d %s  ", vr_name(vr), vr, pr_name[pr]);
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

void while__recover_pr(
    const vector<VrToPr> &before_vr2pr,
    const vector<PrToVr> &before_pr2vr,
    vector<VrToPr> &now_vr2pr,
    vector<PrToVr> &now_pr2vr,
    Scope *scp_to_gen_mc,
    int swap_pr)
{
	BasicBlock *bb = scp_to_gen_mc->basic_block;
	vector<X64mc> &x64mcs = bb->x64mc_alloc_wave;
	X64mc mc;
	(void) vr_state;
	(void) pr_state;

	int spilled = 0;
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		if (before_vr2pr[vr].pr == X64PR_MAX && now_vr2pr[vr].pr != X64PR_MAX)
		{
//			LOG("scp %s, recover spill %s=%%%d-%s", scp->name.c_str(),
//			    vr_name(vr),
//			    vr, pr_name[pr]);
			spill_vr(now_vr2pr, now_pr2vr, vr, x64mcs, " while recover");
			spilled++;
		}
	}
	LOG("scp %s spilled: %d, swap_pr=%d %s", scp_to_gen_mc->name.c_str(), spilled, swap_pr, pr_name[swap_pr]);

	if (now_pr2vr[swap_pr].vr != INVALID_VR)
	{
		int free_pr = X64PR_MAX;
		for (int i = R10D; i < X64PR_MAX; i++)
		{
			if (now_pr2vr[i].vr == INVALID_VR)
			{
				free_pr = i;
				break;
			}
		}
		assert(free_pr != X64PR_MAX /* && free_pr != swap_pr */);

		int vr = now_pr2vr[swap_pr].vr;
		const string &name = vr_name(vr);

		mc = X64mc(MC_ASSIGN);
		mc.pr1 = free_pr;
		mc.pr2 = swap_pr;
		mc.ori_sem = "while swap_pr, assign " + name + "=%" + to_string(vr);
		x64mcs.push_back(mc);

		now_vr2pr[vr].pr = free_pr;
		now_pr2vr[free_pr].vr = vr;

		now_pr2vr[swap_pr].vr = INVALID_VR;
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
		{
			continue;
		}

		string &name = vr_declare_manager.declare_at[vr]->symb->unique_name;

		if (B != X64PR_MAX)
		{
			int now_A_vr = now_pr2vr[A].vr;

			// (curr A is busy && B == PR)
			if (now_A_vr != INVALID_VR)
			{
				assert(now_pr2vr[swap_pr].vr == INVALID_VR);
				auto &now_A_vr_struct = now_vr2pr[now_A_vr];

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

				now_vr2pr[now_A_vr].pr = B;
				now_pr2vr[B].vr = now_A_vr;
			}
			else	//  (now_A is free && B == PR)
			{
				mc = X64mc(MC_ASSIGN);
				mc.pr1 = A;
				mc.pr2 = B;
				mc.ori_sem = "while 2, assign " + name + "=%" + to_string(vr);
				x64mcs.push_back(mc);

				//
//				now_pr2vr[vr]
				now_pr2vr[B].vr = INVALID_VR;
			}
		}
		else	// B == X64PR_MAX
		{
			int now_A_vr = now_pr2vr[A].vr;

			if (now_A_vr != INVALID_VR)
			{
				// must have another free pr
				int free_pr = X64PR_MAX;
				for (int pr = R10D; pr < X64PR_MAX; pr++)
				{
					if (now_pr2vr[pr].vr == INVALID_VR && pr != swap_pr)
					{
						free_pr = pr;
						break;
					}
				}
				if (free_pr == X64PR_MAX)
					ERR("%d", before_vr2pr[now_A_vr].pr);

				auto &now_A_vr_struct = now_vr2pr[now_A_vr];

				mc = X64mc(MC_ASSIGN);
				mc.pr1 = free_pr;
				mc.pr2 = A;
				mc.ori_sem = "while 3, assign " + name + "=%" + to_string(vr)
				    + ", move %s" + to_string(now_A_vr);
				x64mcs.push_back(mc);

				now_vr2pr[now_A_vr].pr = free_pr;
				now_pr2vr[free_pr].vr = now_A_vr;
			}
			//  (now_A_vr == INVALID__VR)
			mc = X64mc(MC_LD, vr);
			mc.pr1 = A;
			mc.ori_sem = "while 4, ld ";
			x64mcs.push_back(mc);

			// TODO if before dirty == 1 ?
			now_vr2pr[vr].dirty = 0;
//			now_vr2pr[vr].u = VR_USAGE_READ;
		}

		now_vr2pr[vr].pr = A;
		now_pr2vr[A].vr = vr;
	}
}

void ra_wave__continue_break_save_vr_state(Scope *scp)
{
//	LOG("%s", scp->name.c_str());

	scp->scp_vr_state = new vector<VrToPr>;
	scp->scp_pr_state = new vector<PrToVr>;
	vector<VrToPr> &scp_vr_state = *scp->scp_vr_state;
	vector<PrToVr> &scp_pr_state = *scp->scp_pr_state;
	scp_vr_state = vr_state;
	scp_pr_state = pr_state;
}

int find_additional_dirty(const vector<VrToPr> &state_a,
    const vector<VrToPr> &state_b,
    vector<bool> &addtional_dirty)
{
	int dirty_cnt = 0;
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		if (state_b[vr].pr != X64PR_MAX
		    && !state_a[vr].dirty
		    && state_b[vr].dirty)
		{
			addtional_dirty[vr] = 1;
			dirty_cnt++;
			printf("new_dirty: %%%d \n", vr);
		}
	}

	return dirty_cnt;
}
void st_additional_dirty_in_cond(Scope *cond, vector<bool> &additional_dirty, int &dirty_cnt)
{
	int meet = 0;
	int fix = 0;
	auto &mcs = cond->basic_block->x64mc_alloc_wave;

	for (int i = 0; i < mcs.size(); i++)
	{
		if (mcs[i].mc_stamp != MC_victim)
			continue;

		int vr = mcs[i].s1;

		if (!additional_dirty[vr])
			continue;

		meet++;

		/*
		 * the first victim of body_tail_dirty[*] must followed by st
		 *
		 * example:
		 * a is clean in pre_cond
		 * a is dirty in new_dirty
		 *
		 * in currnet cond asm:
		 * 1st time:
		 * victim a, no st (lost the value of body_tail_dirty[a])
		 *
		 * 2nd time:
		 * alloc a
		 * dirty a
		 * victim a
		 * st a
		 *
		 * the 2nd time "victim + st" dosn't st body_tail_dirty[a]
		 *
		 */

		if (i + 1 < mcs.size()
		    && mcs[i + 1].mc_stamp == MC_ST
		    && mcs[i + 1].s1 == vr)
		{
			//already have st
		}
		else
		{
			X64mc mc(MC_ST, vr);
			mc.pr1 = mcs[i].pr1;
			mc.ori_sem = "while dirty st";
			mcs.insert(mcs.begin() + i + 1, mc);
			fix++;
		}

		additional_dirty[vr] = 0;
		dirty_cnt--;

		if (dirty_cnt == 0)
			return;
	}

	LOG("%s, meet %d, fix %d", cond->name.c_str(), meet, fix);
}
void st_additional_dirty_in_tail(Scope *scp,
    const vector<VrToPr> &state,
    const vector<bool> &addtional_dirty)
{
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
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		if (!addtional_dirty[vr])
			continue;

		auto &mcs = scp->basic_block->x64mc_alloc_wave;

		X64mc mc(MC_ST, vr);
		mc.pr1 = state[vr].pr;
		mc.ori_sem = "while spill st dirty ";
		mcs.push_back(mc);
	}
}
void st_additional_dirty(Scope *cond,
    const vector<VrToPr> &cond_vr_state,
    Scope *body_tail__continue__break,
    const vector<VrToPr> &body_tail__continue__break_vr_state)
{
	vector<bool> additional_dirty(vr_declare_manager.size(), 0);

	int dirty_cnt = find_additional_dirty(cond_vr_state, body_tail__continue__break_vr_state, additional_dirty);

	if (dirty_cnt && cond)
	{
		auto &mcs = cond->basic_block->x64mc_alloc_wave;
		st_additional_dirty_in_cond(cond, additional_dirty, dirty_cnt);
	}

	if (dirty_cnt)
		st_additional_dirty_in_tail(body_tail__continue__break, body_tail__continue__break_vr_state, additional_dirty);
}

void ra_wave__continue_break(Scope *cond,
    vector<VrToPr> &cond_vr_state,
    vector<PrToVr> &cond_pr_state,
    Scope *source,
    int swap_pr)
{
	//	LOG("scp %s", scp->name.c_str());
	assert(source->scp_vr_state != nullptr);
	assert(source->scp_pr_state != nullptr);

	auto &source_vr_state = *source->scp_vr_state;
	auto &source_pr_state = *source->scp_pr_state;

	////////////////////////////////////////////////////////////
	printf("\n============== cond %d ==============\n", cond->id);
	dump_vr_state(cond_vr_state, "cond_vr_state " + to_string(cond->id));
	dump_vr_state(source_vr_state, "source " + to_string(source->id));

	while__recover_pr(
		cond_vr_state,
		cond_pr_state,
	    source_vr_state,
	    source_pr_state,
	    source,
	    swap_pr);

	dump_vr_state(source_vr_state, "after source swaped " + to_string(source->id));
	printf("\n");

	if (source->sem_stamp == SEM_BREAK)
		st_additional_dirty(0, cond_vr_state, source, source_vr_state);
	else
		st_additional_dirty(cond, cond_vr_state, source, source_vr_state);
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

	int pre_cond_swap_pr = get_swap_pr(while_scp->basic_block);
	assert(pre_cond_swap_pr != X64PR_MAX);

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
	cond->scp_vr_state = new vector<VrToPr>;
	cond->scp_pr_state = new vector<PrToVr>;
	vector<VrToPr> &pre_cond_vr_state = *cond->scp_vr_state;
	vector<PrToVr> &pre_cond_pr_state = *cond->scp_pr_state;
	pre_cond_vr_state = vr_state;
	pre_cond_pr_state = pr_state;

	ra_wave__scope(cond);

	int after_cond_swap_pr = get_swap_pr(while_scp->basic_block);
	assert(after_cond_swap_pr != X64PR_MAX);

	vector<VrToPr> after_cond_vr_state = vr_state;
	vector<PrToVr> after_cond_pr_state = pr_state;

	////////////////////////////////////////////////////////////
	// while body
	ra_wave__scope(body);

	vector<VrToPr> after_body_vr_state = vr_state;
	vector<PrToVr> after_body_pr_state = pr_state;

	////////////////////////////////////////////////////////////
	vr_state = after_cond_vr_state;
	pr_state = after_cond_pr_state;

	////////////////////////////////////////////////////////////
	printf("\n============== while %d ==============\n", while_scp->id);
	dump_vr_state(pre_cond_vr_state, "pre cond " + to_string(while_scp->id));
	dump_vr_state(after_cond_vr_state, "after cond " + to_string(while_scp->id));
	dump_vr_state(after_body_vr_state, "after body " + to_string(while_scp->id));

	while__recover_pr(
	    pre_cond_vr_state,
	    pre_cond_pr_state,
	    after_body_vr_state,
	    after_body_pr_state,
	    get_inner_tail_of_scope(while_scp->clds[1]),
	    pre_cond_swap_pr);

	dump_vr_state(after_body_vr_state, "after body swaped " + to_string(while_scp->id));
	dump_vr_state(vr_state, "vr_state " + to_string(while_scp->id));
	printf("\n");

	////////////////////////////////////////////////////////////////////////////////////////

	Scope *body_inner_tail = get_inner_tail_of_scope(while_scp->clds[1]);

	st_additional_dirty(cond, pre_cond_vr_state, body_inner_tail, after_body_vr_state);

	for (Scope *p : body->continue_list)
	{
		ra_wave__continue_break(cond,
		    pre_cond_vr_state,
		    pre_cond_pr_state,
		    p,
		    pre_cond_swap_pr);
	}

	for (Scope *p : body->break_list)
	{
		ra_wave__continue_break(cond,
		    after_cond_vr_state,
		    after_cond_pr_state,
		    p,
		    after_cond_swap_pr);
	}

	while__end_work(while_scp);
}

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
void ra_wave__if(Scope *if_scp)
{
	//	LOG("scp %s", scp->name.c_str());

	Scope *cond = if_scp->clds[0];
	Scope *then_branch = if_scp->clds[1];
	Scope *else_branch = if_scp->clds[2];
	Scope *tail = if_scp->clds[3];
	Scope *then_tail = get_inner_tail_of_scope(then_branch);
	Scope *else_tail = get_inner_tail_of_scope(else_branch);

	// cond
	ra_wave__scope(cond);
	spill_all_pr(cond->basic_block);

	vector<VrToPr> cond_vr2pr = vr_state;
	vector<PrToVr> cond_pr2vr = pr_state;

	// then
	for (int vr = 0; vr < else_branch->outer_symb_read.size(); vr++)
	{
		int cnt = else_branch->outer_symb_read[vr];
		if (cnt == 0)
			continue;
		Symbol *symb = (vr_declare_manager.declare_at[vr])->symb;
		symb->use_cnt -= cnt;
	}

	ra_wave__scope(then_branch);
	spill_all_pr(then_tail->basic_block);

	vector<VrToPr> then_vr2pr = vr_state;
	vector<PrToVr> then_pr2vr = pr_state;

	// else
	for (int vr = 0; vr < else_branch->outer_symb_read.size(); vr++)
	{
		int cnt = else_branch->outer_symb_read[vr];
		if (cnt == 0)
			continue;
		Symbol *symb = (vr_declare_manager.declare_at[vr])->symb;
		symb->use_cnt += cnt;
	}

	vr_state = cond_vr2pr;
	pr_state = cond_pr2vr;

	ra_wave__scope(else_branch);
	spill_all_pr(else_tail->basic_block);

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
