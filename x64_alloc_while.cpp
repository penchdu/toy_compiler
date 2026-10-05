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

	ERR();

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

void while__pre_work(Scope *scp)
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
void while__end_work(Scope *scp)
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
}
void dump_vr2pr(const vector<VrToPr> &vp, const string &tag)
{
	printf("\n========== %s ==========\n", tag.c_str());
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		int pr = vp[vr].pr;
		if (pr != X64PR_MAX)
			printf("%%%d %s \n", vr, pr_name[pr]);
	}
	printf("==============================\n");
}

static void while__swap_pr(Scope *scp, const vector<VrToPr> &before)
{
	BasicBlock *body_bb = (BasicBlock*) scp->clds[1]->basic_block;
	vector<X64mc> &x64mcs = body_bb->x64mc_alloc_wave;
	X64mc mc;

	int swap_pr = X64PR_MAX;

	int spilled = 0;
	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
	{
		if (before[vr].pr == X64PR_MAX && vr2pr[vr].pr != X64PR_MAX)
		{
			int pr = vr2pr[vr].pr;
			LOG("scp %s, spill %%%d-%s %d", scp->name.c_str(), vr, pr_name[pr], pr);
			spill_vr(x64mcs, vr);
			spilled++;
			//			break;
		}
	}

	if (swap_pr == X64PR_MAX)
		swap_pr = get_swap_pr(body_bb);

	LOG("scp %s spilled: %d, swap_pr=%d %s", scp->name.c_str(), spilled, swap_pr, pr_name[swap_pr]);

	for (int vr = 0; vr < before.size(); vr++)
	{
		int A = before[vr].pr;

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
//					mc.s1 = INVALID__VR;
//					mc.s2 = curr_vr_A;
					mc.pr1 = swap_pr;
					mc.pr2 = A;
					mc.ori_sem = "assign while";
					x64mcs.push_back(mc);

					mc = X64mc(MC_ASSIGN);
					//				mc.s1 = INVALID__VR;
					//				mc.s2 = vr_to_mov;
					mc.pr1 = A;
					mc.pr2 = B;
					mc.ori_sem = "assign while";
					x64mcs.push_back(mc);

					mc = X64mc(MC_ASSIGN);
					//				mc.s1 = INVALID__VR;
					//				mc.s2 = vr_to_mov;
					mc.pr1 = B;
					mc.pr2 = swap_pr;
					mc.ori_sem = "assign while";
					x64mcs.push_back(mc);

					vr2pr[vr] = before[vr];
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
					mc.ori_sem = "assign while";
					x64mcs.push_back(mc);

					vr2pr[vr] = before[vr];
					pr2vr[A].vr = vr;

					//
					pr2vr[B].vr = INVALID__VR;
				}
			}
			else	// B == X64PR_MAX
			{
				int curr_vr_have_A = pr2vr[A].vr;

				if (curr_vr_have_A != INVALID__VR)
				{
					// must have another free pr
					int free_pr = X64PR_MAX;
					for (int i = R10D; i < X64PR_MAX; i++)
					{
						if (pr2vr[i].vr == INVALID__VR)
						{
							free_pr = i;
							break;
						}

					}
					if(free_pr == X64PR_MAX)
						ERR("%d", before[curr_vr_have_A].pr);

					auto &curr_vr_A_struct = vr2pr[curr_vr_have_A];

					mc = X64mc(MC_ASSIGN);
					mc.pr1 = free_pr;
					mc.pr2 = A;
					mc.ori_sem = "while free_pr";
					x64mcs.push_back(mc);

					mc = X64mc(MC_LD, vr);
					mc.pr1 = A;
					mc.ori_sem = "ld while";
					x64mcs.push_back(mc);

					vr2pr[curr_vr_have_A].pr = free_pr;
					pr2vr[free_pr].vr = curr_vr_have_A;

					vr2pr[vr] = before[vr];
					pr2vr[A].vr = vr;

					////////////////////////////////////////////////////
//					spill_vr(x64mcs, curr_vr_have_A);
//					mc = X64mc(MC_LD, vr);
//					mc.pr1 = A;
//					mc.ori_sem = "ld while";
//					x64mcs.push_back(mc);
//
//					vr2pr[vr] = before[vr];
//					pr2vr[A].vr = vr;
				}
				else	//  (curr_vr_A == INVALID__VR)
				{
					mc = X64mc(MC_LD, vr);
					mc.pr1 = A;
					mc.ori_sem = "ld while";
					x64mcs.push_back(mc);

					vr2pr[vr] = before[vr];
					pr2vr[A].vr = vr;
				}
			}

		}
	}
}

void reg_alloc_wave__while(Scope *while_scp)
{
	//	LOG("scp %s", scp->name.c_str());

	while__pre_work(while_scp);

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

	//	vector<VrToPr> pre_cond = vr2pr;
	reg_alloc_wave__scope(while_scp->clds[0]);

	vector<VrToPr> after_cond = vr2pr;

	////////////////////////////////////////////////////////////
	// while body
	reg_alloc_wave__scope(while_scp->clds[1]);

	////////////////////////////////////////////////////////////

	dump_vr2pr(after_cond, "after cond " + to_string(while_scp->id));
	dump_vr2pr(vr2pr, "after body " + to_string(while_scp->id));

	while__swap_pr(while_scp, after_cond);

	dump_vr2pr(vr2pr, "swaped" + to_string(while_scp->id));

	while__end_work(while_scp);

	BasicBlock *tail_bb = (BasicBlock*) while_scp->clds[2]->basic_block;
	assert(tail_bb);
	spill_all_pr(tail_bb);
}
