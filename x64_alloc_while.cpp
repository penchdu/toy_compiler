/*
 * x64_alloc_while.cpp
 *
 *  Created on: 2026年10月4日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

int swap_pr = X64PR_MAX;
static int get_swap_pr(BasicBlock *bb)
{
	if (swap_pr != X64PR_MAX)
		return swap_pr;

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

	spill_vr(bb->x64mc_alloc_wave, swap_pr);
	return swap_pr;
}
static int while__get_pr(BasicBlock *bb, int score)
{
	for (int i = R10D; i < X64PR_MAX; i++)
	{
		if (pr2vr[i].vr == INVALID__VR)
			return i;
	}

	float min_score = 100000;
	int min_score_vr = INVALID__VR;

	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		int vr = pr2vr[pr].vr;
		if (vr2pr[vr].score >= score)
			continue;

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
static int while_get_pr__load_vr(BasicBlock *bb, int vr, int u, int mc_idx)
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
		bb->x64mc_alloc_wave.push_back(mc);
	}

	return vr2pr[vr].pr;
}

static void while__pre_work(Scope *scp)
{
	for (int vr : scp->outer_symb_read)
	{
		Ast *p = vr_declare_manager.declare_at[vr];
		Symbol *symb = p->symb;
		symb->use_cnt++;
	}
	for (int vr : scp->outer_symb_write)
	{
		Ast *p = vr_declare_manager.declare_at[vr];
		Symbol *symb = p->symb;
		symb->use_cnt++;
	}
}
static void while__swap_pr(Scope *scp, const vector<VrToPr> &pre_cond, const vector<VrToPr> &now)
{
	BasicBlock *body_bb = (BasicBlock*) scp->clds[1]->basic_block;
	vector<X64mc> &x64mcs = body_bb->x64mc_alloc_wave;
	X64mc mc;
	bool ok = 0;
	int spilled = 0;
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		if (pre_cond[vr].pr == X64PR_MAX && vr2pr[vr].pr != X64PR_MAX)
		{
			swap_pr = vr2pr[vr].pr;
			spill_vr(x64mcs, vr);

			ok = 1;
			spilled++;
			//			break;
		}
	}
	swap_pr = get_swap_pr(body_bb);
	printf("scp %s spilled: %d\n", scp->name.c_str(), spilled);
	//	assert(ok);

	for (int vr = 0; vr < pre_cond.size(); vr++)
	{
		int A = pre_cond[vr].pr;

		if (A == X64PR_MAX)
			continue;

		int B = vr2pr[vr].pr;

		if (B == A)
		{
			continue;
		}
		else
		{
			if (B != X64PR_MAX)
			{
				int curr_vr_A = pr2vr[A].vr;

				if (curr_vr_A != INVALID__VR)
				{
					auto &curr_vr_A_struct = vr2pr[curr_vr_A];

					mc = X64mc(MC_ASSIGN);
					//				mc.s1 = INVALID__VR;
					//				mc.s2 = vr_to_mov;
					mc.pr1 = swap_pr;
					mc.pr2 = A;
					mc.ori_sem = "while";
					x64mcs.push_back(mc);

					mc = X64mc(MC_ASSIGN);
					//				mc.s1 = INVALID__VR;
					//				mc.s2 = vr_to_mov;
					mc.pr1 = A;
					mc.pr2 = B;
					mc.ori_sem = "while";
					x64mcs.push_back(mc);

					mc = X64mc(MC_ASSIGN);
					//				mc.s1 = INVALID__VR;
					//				mc.s2 = vr_to_mov;
					mc.pr1 = B;
					mc.pr2 = swap_pr;
					mc.ori_sem = "while";
					x64mcs.push_back(mc);

					vr2pr[vr] = pre_cond[vr];
					pr2vr[A].vr = vr;

					vr2pr[curr_vr_A] = curr_vr_A_struct;
					vr2pr[curr_vr_A].pr = B;
					pr2vr[B].vr = curr_vr_A;
				}
				else	//  (curr_vr_A == INVALID__VR)
				{
					mc = X64mc(MC_ASSIGN);
					//				mc.s1 = INVALID__VR;
					//				mc.s2 = vr_to_mov;
					mc.pr1 = A;
					mc.pr2 = B;
					mc.ori_sem = "while";
					x64mcs.push_back(mc);

					vr2pr[vr] = pre_cond[vr];
					pr2vr[A].vr = vr;

					//
					pr2vr[B].vr = INVALID__VR;
				}
			}
			else	// B == X64PR_MAX
			{
				int curr_vr_A = pr2vr[A].vr;

				if (curr_vr_A != INVALID__VR)
				{
					auto &curr_vr_A_struct = vr2pr[curr_vr_A];

					//					mc = X64mc(MC_ASSIGN);
					//					//				mc.s1 = INVALID__VR;
					//					//				mc.s2 = vr_to_mov;
					//					mc.pr1 = swap_pr;
					//					mc.pr2 = A;
					//					mc.ori_sem = "while";
					//					x64mcs.push_back(mc);
					//
					//					vr2pr[curr_vr_A].pr = swap_pr;
					//					pr2vr[swap_pr].vr = curr_vr_A;
					//
					//					swap_vr = INVALID__VR;
					//					swap_pr = X64PR_MAX;

					spill_vr(x64mcs, curr_vr_A);

					////////////////////////////////////////////////////
					mc = X64mc(MC_LD);
					mc.s1 = vr;
					mc.pr1 = A;
					mc.ori_sem = "while_ld";
					x64mcs.push_back(mc);

					vr2pr[vr] = pre_cond[vr];
					pr2vr[A].vr = vr;

				}
				else	//  (curr_vr_A == INVALID__VR)
				{
					mc = X64mc(MC_LD);
					mc.s1 = vr;
					mc.pr1 = A;
					mc.ori_sem = "while_ld";
					x64mcs.push_back(mc);

					vr2pr[vr] = pre_cond[vr];
					pr2vr[A].vr = vr;
				}
			}

		}
	}
}
static void while__end_work(Scope *scp)
{
	for (int vr : scp->outer_symb_read)
	{
		Ast *p = vr_declare_manager.declare_at[vr];
		Symbol *symb = p->symb;
		symb->use_cnt--;
	}
	for (int vr : scp->outer_symb_write)
	{
		Ast *p = vr_declare_manager.declare_at[vr];
		Symbol *symb = p->symb;
		symb->use_cnt--;
	}

	for (int vr : scp->outer_symb_read)
		check_and_clear_vr(scp, vr);
	for (int vr : scp->outer_symb_write)
		check_and_clear_vr(scp, vr);

	BasicBlock *bb = (BasicBlock*) scp->clds[2]->basic_block;
	spill_all_pr(bb);
}
void reg_alloc_wave__while(Scope *scp)
{
	//	LOG("scp %s", scp->name.c_str());
	BasicBlock *bb = (BasicBlock*) scp->basic_block;
	while__pre_work(scp);

//	spill_all_pr(bb);



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

// while cond
	vector<VrToPr> pre_cond = vr2pr;
	reg_alloc_wave__scope(scp->clds[0]);
	vector<VrToPr> after_cond = vr2pr;

	// while body
	reg_alloc_wave__scope(scp->clds[1]);

	while__swap_pr(scp, pre_cond, vr2pr);
	while__end_work(scp);
}
