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

static void spill_vr(int vr, vector<X64mc> &mcs, const string &tag = "")
{
	spill_vr(O1, vr_state, pr_state, vr, mcs, tag);
}

int get_free_pr_num(const vector<PrState> &prst, int reserved_pr)
{
	int free_pr = 0;
	for (int pr = 0; pr < X64PR_MAX; pr++)
	{
		int vr = prst[pr].vr;
		if (vr == INVALID_VR && pr != reserved_pr)
			free_pr++;
	}
	return free_pr;
}

int get_pr_wave(BasicBlock *bb, int mc_idx)
{
	LOG();
	X64mc &mc = bb->mc_schedu[mc_idx];

	for (int pr = 0; pr < X64PR_MAX; pr++)
	{
		if (pr_state[pr].vr == INVALID_VR)
			return pr;
	}

	float min_score = 100000;
	int min_score_vr = INVALID_VR;

	for (int pr = 0; pr < X64PR_MAX; pr++)
	{
		int vr = pr_state[pr].vr;
		if (vr == mc.dst
		    || vr == mc.s1
		    || vr == mc.s2)
			continue;

		// if a vr did not appear in bb, it's score.size() is 0
		float score;
		if (bb_vr_wave[vr].score.empty())
			score = 0.0;
		else
			score = bb_vr_wave[vr].score[mc_idx];

		if(vr_state[vr].dirty)
			score += 0.5;

		if (score < min_score)
		{
			min_score = score;
			min_score_vr = vr;
		}

		LOG("%s=%%%d score=%f", vr_name(vr), vr, score);

	}
	assert(min_score_vr != INVALID_VR);

	LOG("min_score_vr %s=%%%d score=%.2f \n", vr_name(min_score_vr), min_score_vr, min_score);

	int pr = vr_state[min_score_vr].pr;
	spill_vr(min_score_vr, bb->mc_alloc_wave);
	return pr;
}

void check_dead_vr(Scope *scp, vector<X64mc> &mcs, int vr)
{
	if (vr < 0)
		return;

	Ast *declare_at = declare_record.ast[vr];
	Symbol *symb = declare_at->symb;

//	LOG("scp=%d, %s %%%d name=%s, bb: %d %d, symb: %d %d",
//	    scp->id, is_private_symb(scp, vr) ? "private" : "outer  ",
//	    vr, symb->src.c_str(),
//	    scp->use_cnt_in_bb[vr], scp->consume_cnt_in_bb[vr],
//	    symb->use_cnt, symb->consume_cnt);

	if (vr_state[vr].pr != X64PR_MAX
	    && symb->consume_cnt >= symb->use_cnt)
	{
		assert(symb->consume_cnt == symb->use_cnt);

		if (scp->use_cnt_in_bb[vr] != scp->consume_cnt_in_bb[vr])
			ERR("%s, %%%d, bb: %d %d, symb %d %d", scp->name.c_str(), vr,
			    scp->use_cnt_in_bb[vr], scp->consume_cnt_in_bb[vr],
			    symb->use_cnt, symb->consume_cnt);

		spill_vr(vr, mcs, " check_dead_vr");
	}
}

static void check_bb_use_cnt(Scope *scp, int target_symb_stamp = SYMB_ALL)
{
	for (int vr = 0; vr < vr_size; vr++)
	{
		Ast *declare_at = declare_record.ast[vr];
		Symbol *symb = declare_at->symb;
		bool is_private = is_private_symb(scp, vr);

		if (!is_private && ((target_symb_stamp & SYMB_PRIVATE) || (target_symb_stamp & SYMB_PRIVATE_transient)))
			return;

		if (is_private && (target_symb_stamp & SYMB_OUTER))
			return;

		int use = scp->use_cnt_in_bb[vr];
		int consume = scp->consume_cnt_in_bb[vr];
		if (use != consume)
		{
			Ast *ast = declare_record.ast[vr];
			LOG("MISMATCH  scp=%s vr=%%%d name=%s, use=%d consume=%d stamp=%d",
			    scp->name.c_str(), vr, symb->src.c_str(), use, consume, ast->symb->stamp);
			ERR("%%%d, %d %d", vr, use, consume);
		}
	}
}
void check_pr_vr_consistency()
{
	for (int pr = 0; pr < X64PR_MAX; pr++)
	{
		int vr = pr_state[pr].vr;
		if (vr == INVALID_VR)
			continue;

		if (vr_state[vr].pr != pr)
		{
			Ast *declare = declare_record.ast[vr];
			ERR("  MISMATCH pr2vr[R%d].vr=%%%d(%s) but vr2pr[%%%d].pr=%d\n",
			    pr, vr, declare ? declare->tk.src.c_str() : "?", vr, vr_state[vr].pr);
		}
	}
	for (int vr = 0; vr < vr_size; vr++)
	{
		int pr = vr_state[vr].pr;
		if (pr >= X64PR_MAX)
			continue;

		if (pr_state[pr].vr != vr)
		{
			Ast *declare = declare_record.ast[vr];
			ERR("  MISMATCH vr2pr[%%%d(%s)].pr=R%d but pr2vr[R%d].vr=%%%d\n",
			    vr, declare ? declare->tk.src.c_str() : "?", pr, pr, pr_state[pr].vr);
		}
	}
}

static int get_pr(BasicBlock *bb, int mc_idx, int vr, int u)
{
	return get_pr(O1, bb->mc_alloc_wave, bb, mc_idx, vr, u);
}
static int get_pr_alloc_imm(BasicBlock *bb, int mc_idx, int vr)
{
	return get_pr_alloc_imm(O1, bb->mc_alloc_wave, bb, mc_idx, vr);
}

void swap_operand(X64mc &mc)
{
	int s1 = mc.s1;
	int pr1 = mc.pr1;

	mc.s1 = mc.s2;
	mc.pr1 = mc.pr2;

	mc.s2 = s1;
	mc.pr2 = pr1;
}

void ra_wave__mc(Scope *scp, BasicBlock *bb, const X64mc &schedued, int i)
{
	vector<X64mc> &mcs = bb->mc_alloc_wave;
	X64mc mc = schedued;

	switch (mc.stamp)
	{
	case MC_ASSIGN:
		if (mc.dst == mc.s1)
			break;

		mc.pr1 = get_pr(bb, i, mc.s1, VR_USAGE_READ);
		update_vr_consume_cnt(scp, mc.s1);
		check_dead_vr(scp, mcs, mc.s1);

		mc.pr_dst = get_pr(bb, i, mc.dst, VR_USAGE_WRITE);
		mcs.push_back(mc);
		break;

	case MC_ADD:
		case MC_IMUL:
		mc.pr1 = get_pr_alloc_imm(bb, i, mc.s1);
		mc.pr2 = get_pr(bb, i, mc.s2, VR_USAGE_READ);

		update_vr_consume_cnt(scp, mc.s1);
		update_vr_consume_cnt(scp, mc.s2);
		check_dead_vr(scp, mcs, mc.s1);
		check_dead_vr(scp, mcs, mc.s2);

		mc.pr_dst = get_pr(bb, i, mc.dst, VR_USAGE_WRITE);

		if(mc.pr_dst == mc.pr2)
		{
			LOG("swap %s=%%%d %s=%%%d", vr_name(mc.s1), mc.s1, vr_name(mc.s2), mc.s2);
			swap_operand(mc);
		}

		mcs.push_back(mc);
		break;

	case MC_SUB:
		mc.pr1 = get_pr_alloc_imm(bb, i, mc.s1);
		mc.pr2 = get_pr(bb, i, mc.s2, VR_USAGE_READ);

		update_vr_consume_cnt(scp, mc.s1);
		update_vr_consume_cnt(scp, mc.s2);
		check_dead_vr(scp, mcs, mc.s1);
		check_dead_vr(scp, mcs, mc.s2);

		mc.pr_dst = get_pr(bb, i, mc.dst, VR_USAGE_WRITE);

		if(mc.pr_dst == mc.pr2)
		{
			if(mc.pr1 == mc.pr2)
			{
				/*
				 * s1 and s2 share the same register: s1 - s2 == 0.
				 * The neg+add trick would clobber s1, so emit
				 *   mov pr_dst, 0
				 * directly.
				 */
				X64mc li_mc(MC_LI, mc.dst);
				li_mc.pr1 = mc.pr_dst;
				li_mc.const_num = 0;
				li_mc.ori_sem = "li 0 (s1-s2)";
				mcs.push_back(li_mc);
				break;
			}
			X64mc neg_mc(MC_NEG, mc.s2);
			neg_mc.pr1 = mc.pr2;
			mcs.push_back(neg_mc);

			mc.stamp = MC_ADD;

			LOG("swap sub, %s=%%%d %s=%%%d", vr_name(mc.s1), mc.s1, vr_name(mc.s2), mc.s2);
			swap_operand(mc);
		}

		mcs.push_back(mc);
		break;

	case MC_CMP_E:
		case MC_CMP_NE:
		case MC_CMP_L:
		case MC_CMP_LE:
		case MC_CMP_G:
		case MC_CMP_GE:
		mc.pr1 = get_pr_alloc_imm(bb, i, mc.s1);
		mc.pr2 = get_pr(bb, i, mc.s2, VR_USAGE_READ);

		update_vr_consume_cnt(scp, mc.s1);
		update_vr_consume_cnt(scp, mc.s2);
		check_dead_vr(scp, mcs, mc.s1);
		check_dead_vr(scp, mcs, mc.s2);

		if (vr_symb(mc.dst)->use_cnt > 0)
			mc.pr_dst = get_pr(bb, i, mc.dst, VR_USAGE_WRITE);

		mcs.push_back(mc);
		break;

	case MC_DIV:
		mc.pr1 = get_pr(bb, i, mc.s1, VR_USAGE_READ);
		mc.pr2 = get_pr_alloc_imm(bb, i, mc.s2);

		update_vr_consume_cnt(scp, mc.s1);
		update_vr_consume_cnt(scp, mc.s2);
		check_dead_vr(scp, mcs, mc.s1);
		check_dead_vr(scp, mcs, mc.s2);

		mc.pr_dst = get_pr(bb, i, mc.dst, VR_USAGE_WRITE);
		mcs.push_back(mc);
		break;

	case MC_TEST:
		mc.pr1 = get_pr_alloc_imm(bb, i, mc.s1);
		update_vr_consume_cnt(scp, mc.s1);
		check_dead_vr(scp, mcs, mc.s1);

		mcs.push_back(mc);
		break;

	case MC_SAVE_RET:
		mc.pr1 = get_pr(bb, i, mc.s1, VR_USAGE_READ);
		update_vr_consume_cnt(scp, mc.s1);
		check_dead_vr(scp, mcs, mc.s1);

		mcs.push_back(mc);
		break;

	default:
		ERR("%d \n", mc.stamp);
		break;
	}
}

void ra_wave_regular_scope(Scope *scp)
{
	//	LOG("scp %s", scp->name.c_str());
	BasicBlock *bb = scp->bb;

	if (scp->sem_stamp == SEM_FUNC_DEFINE)
	{
		if (scp->arg.size() == 1)
		{
			int vr = scp->arg[0];

			assert(vr_state[vr].pr == X64PR_MAX);
			assert(pr_state[EDI].vr == INVALID_VR);

			vr_state[vr].pr = EDI;
			vr_state[vr].u = VR_USAGE_WRITE;
			vr_state[vr].dirty = 1;

			pr_state[EDI].vr = vr;
		}
	}

	for (int i = 0; i < bb->mc_schedu.size(); i++)
	{
		const X64mc &mc = bb->mc_schedu[i];

		ra_wave__mc(scp, bb, mc, i);
//		update_mc_consume_cnt(scp, mc);
//
//		check_dead_vr(scp, bb->x64mc_alloc_wave, mc.dst);
//		check_dead_vr(scp, bb->x64mc_alloc_wave, mc.s1);
//		check_dead_vr(scp, bb->x64mc_alloc_wave, mc.s2);

		check_pr_vr_consistency();
	}

	for (Scope *p : scp->clds)
		ra_wave_scope(p);
}
void ra_wave_scope(Scope *scp)
{
	//	LOG("scp %s", scp->name.c_str());
	scp->consume_cnt_in_bb.clear();
	scp->consume_cnt_in_bb.resize(vr_size);
	BasicBlock *bb = scp->bb;
	bb->mc_alloc_wave.clear();

	init_wave(bb);
	gen_wave(bb);
//	dump_wave(bb);

	if (scp->sem_stamp == SEM_WHILE)
		ra_wave_while(scp);
	else if (scp->sem_stamp == SEM_CONTINUE || scp->sem_stamp == SEM_BREAK)
		ra_wave_continue_break_save_vr_state(scp);
	else if (scp->sem_stamp == SEM_IF)
		ra_wave_if(scp);
	else
		ra_wave_regular_scope(scp);
}

static void elimicate_copy(Scope *scp)
{
	BasicBlock *bb = scp->bb;
	auto &mcs = bb->mc_alloc_wave;

	for (int i = 0; i < mcs.size(); i++)
	{
		const auto &mc = mcs[i];
		if(mc.stamp == MC_ASSIGN && mc.pr_dst == mc.pr1)
		{
			mcs.erase(mcs.begin() + i);
			continue;
		}

		i++;
	}

	for (Scope *p : scp->clds)
		elimicate_copy(p);
}

void x64_reg_alloc_wave()
{
	LOG("\n======================== wave ========================\n");

	clear_all_vr_consume_cnt(&file_scp);
	bb_vr_wave.resize(vr_size);

	vr_state.clear();
	vr_state.resize(vr_size, {X64PR_MAX, VR_USAGE_INVALID});
	pr_state.clear();
	pr_state.resize(X64PR_MAX, {INVALID_VR});

	ra_wave_scope(&file_scp);

	elimicate_copy(&file_scp);
	check_vr_consume_cnt(&file_scp);
}
