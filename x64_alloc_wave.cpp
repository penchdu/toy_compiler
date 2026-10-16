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

void spill_vr(vector<VrState> &vr_st, vector<PrState> &pr_st, int vr,
    vector<X64mc> &mcs, const string &tag)
{
	int &pr = vr_st[vr].pr;
	int &u = vr_st[vr].u;
	bool &dirty = vr_st[vr].dirty;

	Ast *p = vr_declare_manager.declare_at[vr];
	if (p->sem_stamp == SEM_CONST_NUM && pr > X64PR_MAX)
	{
//		LOG("vr=%%%d, pr=%d", vr, vr_state[vr].pr);
		assert(vr_state[vr].pr > X64PR_MAX);
		return;
	}

	if(pr == X64PR_MAX || u == VR_USAGE_INVALID || pr_st[pr].vr != vr)
		ERR("pr=%d, u=%d, pr_st[%d].vr=%d != %d", pr, u, pr, pr_st[pr].vr, vr);

	assert(pr != X64PR_MAX);
	assert(u != VR_USAGE_INVALID);
	assert(pr_st[pr].vr == vr);

	// MC_victim
	X64mc mc(MC_victim, vr);
	mc.pr1 = (int) pr;
	mc.ori_sem = string("victim ") + pr_name(pr, vr) + tag;
	mcs.push_back(mc);

	if (/*p->sem_stamp != SEM_CONST_NUM &&*/dirty
	    && p->symb->consume_cnt < p->symb->use_cnt)
	{
		X64mc mc(MC_ST, vr);
		mc.pr1 = (int) pr;
		mc.ori_sem = string("spill ") + pr_name(pr, vr) + tag;
		mcs.push_back(mc);
	}

	pr_st[pr].vr = INVALID_VR;
	pr = X64PR_MAX;
	u = VR_USAGE_INVALID;
	dirty = false;
}
void spill_all_pr(BasicBlock *bb)
{
	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		int vr = pr_state[pr].vr;
		if (vr != INVALID_VR)
		{
			spill_vr(vr_state, pr_state, vr, bb->x64mc_alloc_wave);
//			ERR();
		}
	}
}

int get_free_pr_num(const vector<PrState> &prst, int reserved_pr)
{
	int free_pr = 0;
	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		int vr = prst[pr].vr;
		if (vr == INVALID_VR && pr != reserved_pr)
			free_pr++;
	}
	return free_pr;
}

int wave_get_pr(BasicBlock *bb, int mc_idx)
{
	X64mc &mc = bb->x64mc_schedu[mc_idx];

	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		if (pr_state[pr].vr == INVALID_VR)
			return pr;
	}

	float min_score = 100000;
	int min_score_vr = INVALID_VR;

	for (int pr = R10D; pr < X64PR_MAX; pr++)
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

		if (score < min_score)
		{
			min_score = score;
			min_score_vr = vr;
		}
	}
	assert(min_score_vr != INVALID_VR);

	int pr = vr_state[min_score_vr].pr;
	spill_vr(vr_state, pr_state, min_score_vr, bb->x64mc_alloc_wave);
	return pr;
}

static int get_pr__load_vr(BasicBlock *bb, int vr, int u, int mc_idx,
	bool immediate_force_alloc = false)
{
	Ast *p = vr_declare_manager.declare_at[vr];
	if (!immediate_force_alloc && p->sem_stamp == SEM_CONST_NUM)
	{
		vr_state[vr].pr = X64PR_CONST;
		vr_state[vr].u = VR_USAGE_READ;
		return X64PR_CONST;
	}

	if (immediate_force_alloc && p->sem_stamp == SEM_CONST_NUM && vr_state[vr].pr == X64PR_CONST)
	{
		vr_state[vr].pr = X64PR_MAX;
	}

	/*
	 * todo
	 * mc_assign %1, %1
	 * s1 == s2
	 */
	assert(u != VR_USAGE_INVALID);
	bool need_load = 0;

	if (vr_state[vr].pr == X64PR_MAX)
	{
		int pr = wave_get_pr(bb, mc_idx);
		vr_state[vr].pr = pr;
		vr_state[vr].u = u;
		vr_state[vr].dirty = u & VR_USAGE_WRITE;
		pr_state[pr].vr = vr;
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
		vr_state[vr].u |= u;
		vr_state[vr].dirty |= (u & VR_USAGE_WRITE);
	}

	if (need_load)
	{
		Ast *p = vr_declare_manager.declare_at[vr];
		if (p->sem_stamp == SEM_CONST_NUM)
		{
			X64mc mc(MC_LI, vr);
			mc.pr1 = vr_state[vr].pr;
			mc.const_num = p->const_value;
			mc.ori_sem = "li";
			bb->x64mc_alloc_wave.push_back(mc);
		}
		else
		{
			X64mc mc(MC_LD, vr);
			mc.pr1 = vr_state[vr].pr;
			mc.ori_sem = "ld";
			bb->x64mc_alloc_wave.push_back(mc);
		}
	}

	return vr_state[vr].pr;
}

static int load_immediate(BasicBlock *bb, int vr, int mc_idx)
{
	vector<X64mc> &mcs = bb->x64mc_alloc_wave;
	int value = vr_declare_manager.declare_at[vr]->const_value;

	X64mc mc;
	mc = X64mc(MC_LI);
	mc.s1 = vr;
	mc.pr1 = get_pr__load_vr(bb, vr, VR_USAGE_READ, mc_idx, true);
	mc.const_num = value;
	mc.ori_sem = "li";
	mcs.push_back(mc);

	return mc.pr1;
}

void check_dead_vr(Scope *scp, vector<X64mc> &mcs, int vr)
{
	if (vr < 0)
		return;

//	int bb_use = scp->use_cnt_in_bb[vr];
//	if (bb_use < 0)
//		return;

	Ast *declare_at = vr_declare_manager.declare_at[vr];
	Symbol *symb = declare_at->symb;

//	LOG("scp=%d, %s %%%d name=%s, bb: %d %d, symb: %d %d",
//	    scp->id, is_private_symb(scp, vr) ? "private" : "outer  ",
//	    vr, symb->src.c_str(),
//	    scp->use_cnt_in_bb[vr], scp->consume_cnt_in_bb[vr],
//	    symb->use_cnt, symb->consume_cnt);

	if (vr_state[vr].pr != X64PR_MAX
	    && symb->consume_cnt >= symb->use_cnt)
	//	    && symb->cld_consume_cnt >= symb->cld_use_cnt
	{
		assert(symb->consume_cnt == symb->use_cnt);

		if (scp->use_cnt_in_bb[vr] != scp->consume_cnt_in_bb[vr])
			ERR("%s, %%%d, bb: %d %d, symb %d %d", scp->name.c_str(), vr,
			    scp->use_cnt_in_bb[vr], scp->consume_cnt_in_bb[vr],
			    symb->use_cnt, symb->consume_cnt);

		spill_vr(vr_state, pr_state, vr, mcs, " check_dead_vr");

//		// last use in global
//		int pr = vr2pr[vr].pr;
////		LOG("%s clear %s %s", scp->name.c_str(), symb->src.c_str(), get_pr_name(pr));
//
//		vr2pr[vr].pr = X64PR_MAX;
//		vr2pr[vr].u = VR_USAGE_INVALID;
//
//		if (pr2vr[pr].vr == INVALID__VR)
//		{
//			int vr = pr2vr[pr].vr;
//			ERR("scp=%s %s=%%%d", scp->name.c_str(),
//			    vr_declare_manager.declare_at[vr]->tk.src.c_str(), vr);
//		}
//		assert(pr2vr[pr].vr != INVALID__VR);
//		pr2vr[pr].vr = INVALID__VR;
	}
}

static void check_bb_use_cnt(Scope *scp, int target_symb_stamp = SYMB_ALL)
{
	for (int vr = 0; vr < vr_size; vr++)
	{
		Ast *declare_at = vr_declare_manager.declare_at[vr];
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
			Ast *ast = vr_declare_manager.declare_at[vr];
			LOG("MISMATCH  scp=%s vr=%%%d name=%s, use=%d consume=%d stamp=%d",
			    scp->name.c_str(), vr, symb->src.c_str(), use, consume, ast->symb->stamp);
			ERR("%%%d, %d %d", vr, use, consume);
		}
	}
}
void check_pr_vr_consistency()
{
	for (int pr = R10D; pr < X64PR_MAX; pr++)
	{
		int vr = pr_state[pr].vr;
		if (vr == INVALID_VR)
			continue;

		if (vr_state[vr].pr != pr)
		{
			Ast *declare = vr_declare_manager.declare_at[vr];
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
			Ast *declare = vr_declare_manager.declare_at[vr];
			ERR("  MISMATCH vr2pr[%%%d(%s)].pr=R%d but pr2vr[R%d].vr=%%%d\n",
			    vr, declare ? declare->tk.src.c_str() : "?", pr, pr, pr_state[pr].vr);
		}
	}
}

void ra_wave__mc(BasicBlock *bb, const X64mc &schedued, int i)
{
	vector<X64mc> &mcs = bb->x64mc_alloc_wave;
	X64mc mc = schedued;
	MachineCodeStamp mc_stamp = mc.stamp;

	switch (mc_stamp)
	{
	case MC_ASSIGN:
		if (mc.dst == mc.s1)
			break;

		mc.pr_dst = get_pr__load_vr(bb, mc.dst, VR_USAGE_WRITE, i);
		mc.pr1 = get_pr__load_vr(bb, mc.s1, VR_USAGE_READ, i);
		mcs.push_back(mc);
		break;

	case MC_ADD:
		case MC_SUB:
		case MC_IMUL:

	case MC_CMP_E:
		case MC_CMP_NE:
		case MC_CMP_L:
		case MC_CMP_LE:
		case MC_CMP_G:
		case MC_CMP_GE:
		mc.pr_dst = get_pr__load_vr(bb, mc.dst, VR_USAGE_WRITE, i);

		if(vr_declare_manager.declare_at[mc.s1]->sem_stamp == SEM_CONST_NUM)
			mc.pr1 = load_immediate(bb, mc.s1, i);
		else
			mc.pr1 = get_pr__load_vr(bb, mc.s1, VR_USAGE_READ, i);

		mc.pr2 = get_pr__load_vr(bb, mc.s2, VR_USAGE_READ, i);
		mcs.push_back(mc);
		break;

	case MC_DIV:
		mc.pr_dst = get_pr__load_vr(bb, mc.dst, VR_USAGE_WRITE, i);

		mc.pr1 = get_pr__load_vr(bb, mc.s1, VR_USAGE_READ, i);

		if(vr_declare_manager.declare_at[mc.s2]->sem_stamp == SEM_CONST_NUM)
			mc.pr2 = load_immediate(bb, mc.s2, i);
		else
			mc.pr2 = get_pr__load_vr(bb, mc.s2, VR_USAGE_READ, i);
		mcs.push_back(mc);
		break;

	case MC_CMP_TEST:
		mc.pr_dst = get_pr__load_vr(bb, mc.dst, VR_USAGE_WRITE, i);

		if(vr_declare_manager.declare_at[mc.s1]->sem_stamp == SEM_CONST_NUM)
			mc.pr1 = load_immediate(bb, mc.s1, i);
		else
			mc.pr1 = get_pr__load_vr(bb, mc.s1, VR_USAGE_READ, i);

		mcs.push_back(mc);
		break;

	case MC_SAVE_RET:
		mc.pr1 = get_pr__load_vr(bb, mc.s1, VR_USAGE_READ, i);
		mcs.push_back(mc);
		break;

//	case MC_LI:
		//		mc.pr1 = get_pr__load_vr(bb, mc.s1, VR_USAGE_WRITE, i);
//		mcs.push_back(mc);
//		vr_state[mc.s1].dirty = 0;
//		break;
	default:
		ERR("%d \n", mc_stamp);
		break;
	}
}

void ra_wave_regular_scope(Scope *scp)
{
	//	LOG("scp %s", scp->name.c_str());
	BasicBlock *bb = scp->basic_block;

	for (int i = 0; i < bb->x64mc_schedu.size(); i++)
	{
		const X64mc &mc = bb->x64mc_schedu[i];

		ra_wave__mc(bb, mc, i);
		update_mc_consume_cnt(scp, mc);

		check_dead_vr(scp, bb->x64mc_alloc_wave, mc.dst);
		check_dead_vr(scp, bb->x64mc_alloc_wave, mc.s1);
		check_dead_vr(scp, bb->x64mc_alloc_wave, mc.s2);

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
	BasicBlock *bb = scp->basic_block;
	bb->x64mc_alloc_wave.clear();

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

	check_vr_consume_cnt(&file_scp);
}