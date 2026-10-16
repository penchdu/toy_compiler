/*
 * x64_alloc_while.cpp
 *
 *  Created on: 2026年10月4日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

int get_reserved_pr(BasicBlock *bb)
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
	pr_state[swap_pr].vr = INVALID_VR;
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

	Scope *tail = scp->clds[2];
	auto &mcs = tail->basic_block->x64mc_alloc_wave;

	for (int vr = 0; vr < scp->outer_symb_read.size(); vr++)
	{
		if (scp->outer_symb_read[vr] > 0)
			check_dead_vr(scp, mcs, vr);
	}
}
void dump_vr_state(const vector<VrState> &vr_state, const string &tag)
{
	printf("===== %s\n", tag.c_str());
	for (int vr = 0; vr < vr_size; vr++)
	{
		int pr = vr_state[vr].pr;
		if (pr != X64PR_MAX)
			printf("%s=%%%d %s  ", vr_name(vr), vr, pr_name(pr));
	}
	printf("\n	dirty: ");
	for (int vr = 0; vr < vr_size; vr++)
	{
		int pr = vr_state[vr].pr;
		if (pr != X64PR_MAX)
			printf("	%%%d %d", vr, vr_state[vr].dirty);
	}
	printf("\n");
}

void recover_pr_state(
    const vector<VrState> &before_vr_state,
    const vector<PrState> &before_pr_state,
    int swap_pr,
    vector<VrState> &now_vr_state,
    vector<PrState> &now_pr_state,
    Scope *tail_scope_to_gen_mc)
{
	vector<X64mc> &mcs = tail_scope_to_gen_mc->basic_block->x64mc_alloc_wave;
	X64mc mc;

	int spilled = 0;
	for (int vr = 0; vr < vr_size; vr++)
	{
		if (before_vr_state[vr].pr == X64PR_MAX && now_vr_state[vr].pr != X64PR_MAX)
		{
//			LOG("scp %s, recover spill %s=%%%d-%s", scp->name.c_str(),
//			    vr_name(vr), vr, get_pr_name(pr));
			spill_vr(now_vr_state, now_pr_state, vr, mcs, " while recover");
			spilled++;
		}
	}
	LOG("scp %s spilled: %d, swap_pr=%d %s", tail_scope_to_gen_mc->name.c_str(),
		spilled, swap_pr, pr_name(swap_pr));

	if (now_pr_state[swap_pr].vr != INVALID_VR)
	{
		int free_pr = X64PR_MAX;
		for (int i = R10D; i < X64PR_MAX; i++)
		{
			if (now_pr_state[i].vr == INVALID_VR)
			{
				free_pr = i;
				break;
			}
		}
		assert(free_pr != X64PR_MAX /* && free_pr != swap_pr */);

		int vr = now_pr_state[swap_pr].vr;
		const string &name = vr_name(vr);

		mc = X64mc(MC_ASSIGN);
		mc.pr1 = free_pr;
		mc.pr2 = swap_pr;
		mc.ori_sem = "while swap_pr, assign " + name + "=%" + to_string(vr);
		mcs.push_back(mc);

		now_vr_state[vr].pr = free_pr;
		now_pr_state[free_pr].vr = vr;

		now_pr_state[swap_pr].vr = INVALID_VR;
	}
	if (swap_pr == X64PR_MAX)
		ERR();

	for (int vr = 0; vr < before_vr_state.size(); vr++)
	{
		int A = before_vr_state[vr].pr;

		if (A == X64PR_MAX)
			continue;

		int B = now_vr_state[vr].pr;

		if (B == A)
		{
			continue;
		}

		string &name = vr_declare_manager.declare_at[vr]->symb->unique_name;

		if (B != X64PR_MAX)
		{
			int now_A_vr = now_pr_state[A].vr;

			// (curr A is busy && B == PR)
			if (now_A_vr != INVALID_VR)
			{
				assert(now_pr_state[swap_pr].vr == INVALID_VR);
				auto &now_A_vr_struct = now_vr_state[now_A_vr];

				mc = X64mc(MC_ASSIGN);
				mc.pr1 = swap_pr;
				mc.pr2 = A;
				mc.ori_sem = "while 1, assign " + name + "=%" + to_string(vr);
				mcs.push_back(mc);

				mc = X64mc(MC_ASSIGN);
				mc.pr1 = A;
				mc.pr2 = B;
				mc.ori_sem = "while 1, assign " + name + "=%" + to_string(vr);
				mcs.push_back(mc);

				mc = X64mc(MC_ASSIGN);
				mc.pr1 = B;
				mc.pr2 = swap_pr;
				mc.ori_sem = "while 1, assign " + name + "=%" + to_string(vr);
				mcs.push_back(mc);

				now_vr_state[now_A_vr].pr = B;
				now_pr_state[B].vr = now_A_vr;
			}
			else	//  (now_A is free && B == PR)
			{
				mc = X64mc(MC_ASSIGN);
				mc.pr1 = A;
				mc.pr2 = B;
				mc.ori_sem = "while 2, assign " + name + "=%" + to_string(vr);
				mcs.push_back(mc);

				//
//				now_pr2vr[vr]
				now_pr_state[B].vr = INVALID_VR;
			}
		}
		else	// B == X64PR_MAX
		{
			int now_A_vr = now_pr_state[A].vr;

			if (now_A_vr != INVALID_VR)
			{
				// must have another free pr
				int free_pr = X64PR_MAX;
				for (int pr = R10D; pr < X64PR_MAX; pr++)
				{
					if (now_pr_state[pr].vr == INVALID_VR && pr != swap_pr)
					{
						free_pr = pr;
						break;
					}
				}
				if (free_pr == X64PR_MAX)
					ERR("%d", before_vr_state[now_A_vr].pr);

				auto &now_A_vr_struct = now_vr_state[now_A_vr];

				mc = X64mc(MC_ASSIGN);
				mc.pr1 = free_pr;
				mc.pr2 = A;
				mc.ori_sem = "while 3, assign " + name + "=%" + to_string(vr)
				    + ", move %s" + to_string(now_A_vr);
				mcs.push_back(mc);

				now_vr_state[now_A_vr].pr = free_pr;
				now_pr_state[free_pr].vr = now_A_vr;
			}
			//  (now_A_vr == INVALID__VR)
			mc = X64mc(MC_LD, vr);
			mc.pr1 = A;
			mc.ori_sem = "while 4, ld ";
			mcs.push_back(mc);

			// don't care if before dirty == 1
			// now.dirty = 0, seems ok
//			now_vr2pr[vr].u = VR_USAGE_READ;
			now_vr_state[vr] = before_vr_state[vr];
			now_vr_state[vr].dirty = 0;
		}

		now_vr_state[vr].pr = A;
		now_pr_state[A].vr = vr;
	}
}

void ra_wave_continue_break_save_vr_state(Scope *scp)
{
//	LOG("%s", scp->name.c_str());

	scp->scp_vr_state = new vector<VrState>;
	scp->scp_pr_state = new vector<PrState>;
	vector<VrState> &scp_vr_state = *scp->scp_vr_state;
	vector<PrState> &scp_pr_state = *scp->scp_pr_state;
	scp_vr_state = vr_state;
	scp_pr_state = pr_state;
}

int find_additional_dirty(const vector<VrState> &state_a,
    const vector<VrState> &state_b,
    vector<bool> &addtional_dirty)
{
	int dirty_cnt = 0;
	for (int vr = 0; vr < vr_size; vr++)
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
		if (mcs[i].stamp != MC_victim)
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
		 * in current cond asm:
		 * 1st time:
		 * victim a, no st (lost the value of body_tail_dirty[a], in this case, should insert st)
		 *
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
		    && mcs[i + 1].stamp == MC_ST
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
    const vector<VrState> &state,
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
	for (int vr = 0; vr < vr_size; vr++)
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
    const vector<VrState> &cond_vr_state,
    Scope *body_tail__continue__break,
    const vector<VrState> &body_tail__continue__break_vr_state)
{
	vector<bool> additional_dirty(vr_size, 0);

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
    vector<VrState> &cond_vr_state,
    vector<PrState> &cond_pr_state,
    int swap_pr,
    Scope *source)
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

	recover_pr_state(
	    cond_vr_state,
	    cond_pr_state,
	    swap_pr,
	    source_vr_state,
	    source_pr_state,
	    source);

	dump_vr_state(source_vr_state, "after source swaped " + to_string(source->id));
	printf("\n");

	if (source->sem_stamp == SEM_BREAK)
		st_additional_dirty(0, cond_vr_state, source, source_vr_state);
	else
		st_additional_dirty(cond, cond_vr_state, source, source_vr_state);
}

void move_victim_to_pre_cond(
    Scope *while_scp,
    Scope *cond_or_body,
    vector<VrState> &pre_cond_vr_state,
    vector<PrState> &pre_cond_pr_state,
    vector<int> &vr_referenced,
    vector<int> &vr_victim)
{
	auto &mcs_while = while_scp->basic_block->x64mc_alloc_wave;
	auto &mcs = cond_or_body->basic_block->x64mc_alloc_wave;

	for (int i = 0; i < mcs.size(); i++)
	{
		auto &mc = mcs[i];

		if (mc.stamp == MC_victim && vr_referenced[mc.s1] == 0)
		{
			int vr = mc.s1;
			assert(mc.pr1 == pre_cond_vr_state[vr].pr);

			spill_vr(pre_cond_vr_state,
			    pre_cond_pr_state,
			    vr, mcs_while, " ,move from b" + to_string(cond_or_body->id) + ",");

			vr_victim[vr]++;
			mc.stamp = MC_INVALID;

			if (i + 1 < mcs.size()
			    && mcs[i + 1].stamp == MC_ST
			    && mcs[i + 1].s1 == mc.s1)
			{
				mcs[i + 1].stamp = MC_INVALID;
			}

			LOG("%s=%%%d", vr_name(vr), vr);
		}

		if (mc.s1 >= 0)
			vr_referenced[mc.s1]++;
		if (mc.s2 >= 0)
			vr_referenced[mc.s2]++;
		if (mc.dst >= 0)
			vr_referenced[mc.dst]++;
	}

	for (int i = 0; i < mcs.size();)
	{
		if (mcs[i].stamp == MC_INVALID)
			mcs.erase(mcs.begin() + i);
		else
			i++;
	}
}
void while_move_victim_to_pre_cond(
    Scope *while_scp,
    vector<VrState> &pre_cond_vr_state,
    vector<PrState> &pre_cond_pr_state,
    vector<VrState> &after_cond_vr_state,
    vector<PrState> &after_cond_pr_state)
{
	Scope *cond = while_scp->clds[0];
	Scope *body = while_scp->clds[1];

	vector<int> vr_reference(vr_size, 0);
	vector<int> vr_victim(vr_size, 0);
	vector<int> vr_ld(vr_size, 0);

	move_victim_to_pre_cond(
	    while_scp,
	    cond,
	    pre_cond_vr_state,
	    pre_cond_pr_state,
	    vr_reference,
	    vr_victim);

	vr_victim.clear();
	vr_victim.resize(vr_size, 0);

	move_victim_to_pre_cond(
	    while_scp,
	    body,
	    pre_cond_vr_state,
	    pre_cond_pr_state,
	    vr_reference,
	    vr_victim);

	/*
	 * if move_victim_to_pre_cond happened in body, vr have pr in pre_cond
	 * and in_cond and after_cond and have no op in in_cond
	 *
	 */
	for (int vr = 0; vr < vr_size; vr++)
	{
		if (vr_victim[vr] > 0 && after_cond_vr_state[vr].pr != X64PR_MAX)
		{
			int pr = after_cond_vr_state[vr].pr;
			after_cond_vr_state[vr].pr = X64PR_MAX;
			after_cond_vr_state[vr].u = VR_USAGE_INVALID;
			after_cond_vr_state[vr].dirty = 0;

			after_cond_pr_state[pr].vr = INVALID_VR;
		}
	}
}
void move_ld_to_pre_cond(
    Scope *while_scp,
    Scope *cond_or_body,
    vector<VrState> &pre_cond_vr_state,
    vector<PrState> &pre_cond_pr_state,
    vector<int> &vr_reference,
    vector<int> &pr_reference,
    vector<int> &vr_ld,
    int reserved_pr)
{
	auto &mcs_while = while_scp->basic_block->x64mc_alloc_wave;
	auto &mcs = cond_or_body->basic_block->x64mc_alloc_wave;

	for (int i = 0; i < mcs.size(); i++)
	{
		auto &mc = mcs[i];

		if (mc.stamp == MC_LD && vr_reference[mc.s1] == 0)
		{
			int vr = mc.s1;
			int pr = mc.pr1;

			if (pr != reserved_pr
			    && pr_reference[pr] == 0
			    && pre_cond_pr_state[pr].vr == INVALID_VR)
			{
				pre_cond_vr_state[vr].pr = pr;
				pre_cond_vr_state[vr].u = VR_USAGE_READ;
				pre_cond_vr_state[vr].dirty = 0;
				pre_cond_pr_state[pr].vr = vr;

				mcs_while.push_back(mc);
				vr_ld[mc.s1]++;
				mc.stamp = MC_INVALID;

				int free_pr = get_free_pr_num(pre_cond_pr_state, reserved_pr);
				LOG("ok");
				LOG("%d, pr_reference[%s]=%d, pre_cond_pr_state[%s].vr=%%%d, free_pr=%d",
				    pr != reserved_pr,
				    pr_name(pr), pr_reference[pr],
				    pr_name(pr), pre_cond_pr_state[pr].vr,
				    free_pr);
			}
		}

		if (mc.s1 >= 0)
			vr_reference[mc.s1]++;
		if (mc.s2 >= 0)
			vr_reference[mc.s2]++;
		if (mc.dst >= 0)
			vr_reference[mc.dst]++;

		if (mc.pr1 != X64PR_MAX)
			pr_reference[mc.pr1]++;
		if (mc.pr2 != X64PR_MAX)
			pr_reference[mc.pr2]++;
		if (mc.pr_dst != X64PR_MAX)
			pr_reference[mc.pr_dst]++;
	}

	for (int i = 0; i < mcs.size();)
	{
		if (mcs[i].stamp == MC_INVALID)
			mcs.erase(mcs.begin() + i);
		else
			i++;
	}
}
void while_move_ld_to_pre_cond(
    Scope *while_scp,
    vector<VrState> &pre_cond_vr_state,
    vector<PrState> &pre_cond_pr_state,
    vector<VrState> &after_cond_vr_state,
    vector<PrState> &after_cond_pr_state,
    int pre_cond_reserved_pr)
{
	Scope *cond = while_scp->clds[0];
	Scope *body = while_scp->clds[1];

	vector<int> vr_reference(vr_size, 0);
	vector<int> pr_reference(X64PR_MAX, 0);
	vector<int> vr_ld(vr_size, 0);

	move_ld_to_pre_cond(
	    while_scp,
	    cond,
	    pre_cond_vr_state,
	    pre_cond_pr_state,
	    vr_reference,
	    pr_reference,
	    vr_ld,
	    pre_cond_reserved_pr);

	vr_ld.clear();
	vr_ld.resize(vr_size, 0);

	move_ld_to_pre_cond(
	    while_scp,
	    body,
	    pre_cond_vr_state,
	    pre_cond_pr_state,
	    vr_reference,
	    pr_reference,
	    vr_ld,
	    pre_cond_reserved_pr);

	/*
	 * if move_ld_to_pre_cond happened in body, vr have no pr in pre_cond
	 * and cond and after_cond, and have no op in cond
	 *
	 */
	for (int vr = 0; vr < vr_size; vr++)
	{
		if (vr_ld[vr] > 0)
		{
			int pr = pre_cond_vr_state[vr].pr;

			assert(after_cond_vr_state[vr].pr == X64PR_MAX);
			assert(after_cond_pr_state[pr].vr == INVALID_VR);

			after_cond_vr_state[vr].pr = pr;
			after_cond_vr_state[vr].u = VR_USAGE_READ;
			after_cond_vr_state[vr].dirty = 0;

			after_cond_pr_state[pr].vr = vr;
		}
	}
}

void ra_wave_while(Scope *while_scp)
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

	int pre_cond_reserved_pr = get_reserved_pr(while_scp->basic_block);
	assert(pre_cond_reserved_pr != X64PR_MAX);

//	for(int vr : scp->outer_symb_write)
//	{
//		for(int pr : )
//	}
	/*
	 * pre alloc pr to reduce spill
	 * spill unnecessary pr
	 * try alloc cond write
	 * try alloc body write
	 * try alloc cond read
	 * try alloc body read
	 */
// select cond pr
//	spill_vr(bb->x64mc_alloc_wave, swap_vr);
////////////////////////////////////////////////////////////
// while cond
//	Scope *cond = while_scp->clds[0];
//	Scope *body = while_scp->clds[1];
//	cond->addtional_symb_usage.resize(vr_size, VR_USAGE_INVALID);
//	for (int i = 0; i < vr_size; i++)
//	{
//		if (cond->outer_symb_read[i] > 0 && body->outer_symb_write[i] > 0)
//			cond->addtional_symb_usage[i] |= VR_USAGE_WRITE;
//	}
	cond->scp_vr_state = new vector<VrState>;
	cond->scp_pr_state = new vector<PrState>;
	vector<VrState> &pre_cond_vr_state = *cond->scp_vr_state;
	vector<PrState> &pre_cond_pr_state = *cond->scp_pr_state;
	pre_cond_vr_state = vr_state;
	pre_cond_pr_state = pr_state;

	ra_wave_scope(cond);

	int after_cond_swap_pr = get_reserved_pr(cond->basic_block);	// while_scp->basic_block ?
	assert(after_cond_swap_pr != X64PR_MAX);

	vector<VrState> after_cond_vr_state = vr_state;
	vector<PrState> after_cond_pr_state = pr_state;

	////////////////////////////////////////////////////////////
	// while body
	ra_wave_scope(body);

	vector<VrState> after_body_vr_state = vr_state;
	vector<PrState> after_body_pr_state = pr_state;

	////////////////////////////////////////////////////////////

	while_move_victim_to_pre_cond(
	    while_scp,
	    pre_cond_vr_state,
	    pre_cond_pr_state,
	    after_cond_vr_state,
	    after_cond_pr_state);

	while_move_ld_to_pre_cond(
	    while_scp,
	    pre_cond_vr_state,
	    pre_cond_pr_state,
	    after_cond_vr_state,
	    after_cond_pr_state,
	    pre_cond_reserved_pr);

	vr_state = after_cond_vr_state;
	pr_state = after_cond_pr_state;

	////////////////////////////////////////////////////////////
	printf("\n============== while %d ==============\n", while_scp->id);
	dump_vr_state(pre_cond_vr_state, "pre cond " + to_string(while_scp->id));
	dump_vr_state(after_cond_vr_state, "after cond " + to_string(while_scp->id));
	dump_vr_state(after_body_vr_state, "after body " + to_string(while_scp->id));

	recover_pr_state(
	    pre_cond_vr_state,
	    pre_cond_pr_state,
	    pre_cond_reserved_pr,
	    after_body_vr_state,
	    after_body_pr_state,
	    get_inner_tail_of_scope(body));

	dump_vr_state(after_body_vr_state, "after body swaped " + to_string(while_scp->id));
	dump_vr_state(vr_state, "vr_state " + to_string(while_scp->id));
	printf("\n");

	////////////////////////////////////////////////////////////////////////////////////////

	Scope *body_inner_tail = get_inner_tail_of_scope(body);

	st_additional_dirty(cond, pre_cond_vr_state, body_inner_tail, after_body_vr_state);

	for (Scope *p : body->continue_list)
	{
		ra_wave__continue_break(cond,
		    pre_cond_vr_state,
		    pre_cond_pr_state,
		    pre_cond_reserved_pr,
		    p);
	}

	for (Scope *p : body->break_list)
	{
		ra_wave__continue_break(cond,
		    after_cond_vr_state,
		    after_cond_pr_state,
		    after_cond_swap_pr,
		    p);
	}

	while__end_work(while_scp);
}

