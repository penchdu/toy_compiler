///*
// * asymmetric_peak_valley_regalloc.cpp
// *
// *  Created on: 2026年9月26日
// *      Author: x
// */
//
//#include "frontend.h"
//#include "x64_back_end.h"
//#include "basic_block.h"
//#include <cmath>
//
//extern vector<BasicBlock> basic_blocks;
//
//struct VRWave {
//	vector<float> score;
//	vector<int> inst_idx;		// >=0, <= 3
//};
//vector<VRWave> wave;
//vector<int> wave_vrid;
//float max_wave_value = 0;
//
//static void dump_wave()
//{
//	static const char *level[] = {
//	    " ",
//	    "▁",
//	    "▂",
//	    "▃",
//	    "▄",
//	    "▅",
//	    "▆",
//	    "▇",
//
//	    "█",
//	};
//
//	printf("\n========== wave ==========\n");
//
//	for (int vr : wave_vrid)
//	{
//		const vector<float> &score = wave[vr].score;
//
//		if (score.empty())
//			continue;
//
//		float max_score =
//		    *std::max_element(score.begin(), score.end());
//
//		printf("VR %-4d | ", vr);
//
//		if (max_score <= 0.0f)
//		{
//			for (size_t pc = 0; pc < score.size(); ++pc)
//				printf(" ");
//			printf("\n");
//			continue;
//		}
//
//		for (float s : score)
//		{
//			float ratio = s / max_score;
//
//			int h = (int) (ratio * 8.0f);
//
//			if (h < 0)
//				h = 0;
//			if (h > 8)
//				h = 8;
//
//			printf("%s", level[h]);
//		}
//
//		printf("  max=%.2f\n", max_score);
//	}
//
//	printf("     ");
//
//	if (!wave_vrid.empty())
//	{
//		int n = wave[wave_vrid[0]].score.size();
//
//		printf("   ");
//
//		for (int i = 0; i < n; ++i)
//		{
//			if (i % 10 == 0)
//				printf("|");
//
//			else
//				printf("-");
//		}
//
//		printf("\n     ");
//
//		for (int i = 0; i < n; ++i)
//		{
//			if (i % 10 == 0)
//				printf("%-10d", i);
//		}
//
//		printf("\n");
//	}
//
//	printf("===========================\n");
//}
//
//static void init_wave()
//{
//	int mc_size = 0;
//
//	for (BasicBlock &bb : basic_blocks)
//	{
//		int n = bb.x64mc_schedu.size();
//		mc_size += n;
//
//		for (X64mc &mc : bb.x64mc_schedu)
//		{
////		for (int i = 0; i < 3; i++)
////		{
////			int vr = mc.vr[i];
////			if (vr >= 0)
////			{
////				wave[vr].score.clear();
////				wave[vr].score.resize(n, 0);
////				wave[vr].inst_idx.clear();
////				wave[vr].inst_idx.resize(n, 0);
////			}
////		}
//		}
//	}
//
//	for (int k = 0; k < bb.x64mc_schedu.size(); k++)
//	{
//		X64mc &mc = bb.x64mc_schedu[k];
//
//		for (int i = 0; i < 3; i++)
//		{
//			int vr = mc.vr[i];
//			if (vr >= 0)
//			{
//				wave[vr].inst_idx[k]++;
//				wave_vrid.push_back(vr);
//			}
//		}
//	}
//
//	auto &v = wave_vrid;
//	std::sort(v.begin(), v.end());
//	v.erase(std::unique(v.begin(), v.end()), v.end());
//}
//
//static void compute_wave_triangle(VRWave &wave)
//{
//	constexpr int R = 5;
//	const int slots = wave.inst_idx.size();
//
//	std::fill(wave.score.begin(), wave.score.end(), 0.0f);
//
//	for (int inst_appear_cnt = 0; inst_appear_cnt < slots; inst_appear_cnt++)
//	{
//		if (wave.inst_idx[inst_appear_cnt] == 0)
//			continue;
//
//		for (int pc = 0; pc < slots; ++pc)
//		{
//			int d = std::abs(pc - inst_appear_cnt);
//
//			if (d <= R)
//			{
//				float weight = (float) (R - d) / R;
//				wave.score[pc] += wave.inst_idx[inst_appear_cnt] * weight;
//			}
//		}
//	}
//}
//static void compute_wave_decay(VRWave &w)
//{
//	int left_R = 4;
//	float decay = 0.8f;
//
//	const int n = w.inst_idx.size();
//	std::fill(w.score.begin(), w.score.end(), 0.0f);
//
//	for (int use_pc = 0; use_pc < n; ++use_pc)
//	{
//		int count = w.inst_idx[use_pc];
//
//		if (count == 0)
//			continue;
//
//		for (int pc = 0; pc < n; ++pc)
//		{
//			float weight = 0.0f;
//
//			if (pc <= use_pc)
//			{
//				// use 前：缓慢建立波峰
//				int d = use_pc - pc;
//
//				if (d <= left_R)
//					weight = 1.0f - (float) d / left_R;
//			}
//			else
//			{
//				// use 后：快速指数衰减
//				int d = pc - use_pc;
//				weight = std::exp(-decay * d);
//			}
//
//			w.score[pc] += count * weight;
//		}
//	}
//}
//
//static void compute_wave(BasicBlock &bb)
//{
//	for (int vr : wave_vrid)
//	{
////    	compute_wave_triangle(wave[vr]);
//		compute_wave_decay(wave[vr]);
//	}
//}
//
//void asymmetric_peak_wave_reg_alloc()
//{
//	for (BasicBlock &bb : basic_blocks)
//	{
//		wave_vrid.clear();
//		max_wave_value = 0;
//
//		init_wave();
//		compute_wave(bb);
//
//		dump_wave();
//
////		break;
//	}
//}

//static void while___reposition(Scope *scp, const vector<VrToPr> &before, vector<VrToPr> &now, int vr, int swap_pr)
//{
//	int B = now[vr].pr;
//
//	if (B == A)
//		return;
//
//	if (B != X64PR_MAX)
//	{
//		int curr_vr_A = pr2vr[A].vr;
//
//		if (curr_vr_A != INVALID__VR)
//		{
//			auto &curr_vr_A_struct = now[curr_vr_A];
//
//			mc = X64mc(MC_ASSIGN);
//			//					mc.s1 = INVALID__VR;
//			//					mc.s2 = curr_vr_A;
//			mc.pr1 = swap_pr;
//			mc.pr2 = A;
//			mc.ori_sem = "assign while";
//			x64mcs.push_back(mc);
//
//			mc = X64mc(MC_ASSIGN);
//			//				mc.s1 = INVALID__VR;
//			//				mc.s2 = vr_to_mov;
//			mc.pr1 = A;
//			mc.pr2 = B;
//			mc.ori_sem = "assign while";
//			x64mcs.push_back(mc);
//
//			mc = X64mc(MC_ASSIGN);
//			//				mc.s1 = INVALID__VR;
//			//				mc.s2 = vr_to_mov;
//			mc.pr1 = B;
//			mc.pr2 = swap_pr;
//			mc.ori_sem = "assign while";
//			x64mcs.push_back(mc);
//
//			now[vr] = before[vr];
//			pr2vr[A].vr = vr;
//
//			now[curr_vr_A] = curr_vr_A_struct;
//			now[curr_vr_A].pr = B;
//			pr2vr[B].vr = curr_vr_A;
//		}
//		else	//  (curr_vr_A == INVALID__VR)
//		{
//			mc = X64mc(MC_ASSIGN);
//			//				mc.s1 = INVALID__VR;
//			//				mc.s2 = vr_to_mov;
//			mc.pr1 = A;
//			mc.pr2 = B;
//			mc.ori_sem = "assign while";
//			x64mcs.push_back(mc);
//
//			now[vr] = before[vr];
//			pr2vr[A].vr = vr;
//
//			//
//			pr2vr[B].vr = INVALID__VR;
//		}
//	}
//	else	// B == X64PR_MAX
//	{
//		int curr_vr_have_A = pr2vr[A].vr;
//
//		if (curr_vr_have_A != INVALID__VR)
//		{
//			ERR("%d", before[curr_vr_have_A].pr);
//
//			auto &curr_vr_A_struct = now[curr_vr_have_A];
//
//			//					mc = X64mc(MC_ASSIGN);
//			//					//				mc.s1 = INVALID__VR;
//			//					//				mc.s2 = vr_to_mov;
//			//					mc.pr1 = swap_pr;
//			//					mc.pr2 = A;
//			//					mc.ori_sem = "while";
//			//					x64mcs.push_back(mc);
//			//
//			//					vr2pr[curr_vr_A].pr = swap_pr;
//			//					pr2vr[swap_pr].vr = curr_vr_A;
//			//
//			//					swap_vr = INVALID__VR;
//			//					swap_pr = X64PR_MAX;
//
//			spill_vr(x64mcs, curr_vr_have_A);
//
//			////////////////////////////////////////////////////
//			mc = X64mc(MC_LD, vr);
//			mc.pr1 = A;
//			mc.ori_sem = "ld while";
//			x64mcs.push_back(mc);
//
//			now[vr] = before[vr];
//			pr2vr[A].vr = vr;
//		}
//		else	//  (curr_vr_A == INVALID__VR)
//		{
//			mc = X64mc(MC_LD, vr);
//			mc.pr1 = A;
//			mc.ori_sem = "ld while";
//			x64mcs.push_back(mc);
//
//			now[vr] = before[vr];
//			pr2vr[A].vr = vr;
//		}
//	}
//
//}
//static void while__swap_pr2(Scope *scp, const vector<VrToPr> &before, vector<VrToPr> &now)
//{
//	BasicBlock *body_bb = (BasicBlock*) scp->clds[1]->basic_block;
//	vector<X64mc> &x64mcs = body_bb->x64mc_alloc_wave;
//	X64mc mc;
//
//	int swap_pr = X64PR_MAX;
//
//	int spilled = 0;
//	for (int vr = 0; vr < vr_declare_manager.size(); vr++)
//	{
//		if (before[vr].pr == X64PR_MAX && now[vr].pr != X64PR_MAX)
//		{
//			int pr = now[vr].pr;
//			LOG("scp %s, spill %%%d-%s %d", scp->name.c_str(), vr, pr_name[pr], pr);
//			spill_vr(x64mcs, vr);
//			spilled++;
//			//			break;
//		}
//	}
//
//	if (swap_pr == X64PR_MAX)
//		swap_pr = get_swap_pr(body_bb);
//
//	LOG("scp %s spilled: %d, swap_pr=%d %s", scp->name.c_str(), spilled, swap_pr, pr_name[swap_pr]);
//
//	for (int vr = 0; vr < before.size(); vr++)
//	{
//		int A = before[vr].pr;
//
//		if (A == X64PR_MAX)
//			continue;
//
//		while___reposition(scp, before, now, vr, swap_pr);
//		int B = now[vr].pr;
//
//		if (B == A)
//		{
//			continue;
//		}
//		else
//		{
//			if (B != X64PR_MAX)
//			{
//				int curr_vr_A = pr2vr[A].vr;
//
//				if (curr_vr_A != INVALID__VR)
//				{
//					auto &curr_vr_A_struct = now[curr_vr_A];
//
//					mc = X64mc(MC_ASSIGN);
////					mc.s1 = INVALID__VR;
////					mc.s2 = curr_vr_A;
//					mc.pr1 = swap_pr;
//					mc.pr2 = A;
//					mc.ori_sem = "assign while";
//					x64mcs.push_back(mc);
//
//					mc = X64mc(MC_ASSIGN);
//					//				mc.s1 = INVALID__VR;
//					//				mc.s2 = vr_to_mov;
//					mc.pr1 = A;
//					mc.pr2 = B;
//					mc.ori_sem = "assign while";
//					x64mcs.push_back(mc);
//
//					mc = X64mc(MC_ASSIGN);
//					//				mc.s1 = INVALID__VR;
//					//				mc.s2 = vr_to_mov;
//					mc.pr1 = B;
//					mc.pr2 = swap_pr;
//					mc.ori_sem = "assign while";
//					x64mcs.push_back(mc);
//
//					now[vr] = before[vr];
//					pr2vr[A].vr = vr;
//
//					now[curr_vr_A] = curr_vr_A_struct;
//					now[curr_vr_A].pr = B;
//					pr2vr[B].vr = curr_vr_A;
//				}
//				else	//  (curr_vr_A == INVALID__VR)
//				{
//					mc = X64mc(MC_ASSIGN);
//					//				mc.s1 = INVALID__VR;
//					//				mc.s2 = vr_to_mov;
//					mc.pr1 = A;
//					mc.pr2 = B;
//					mc.ori_sem = "assign while";
//					x64mcs.push_back(mc);
//
//					now[vr] = before[vr];
//					pr2vr[A].vr = vr;
//
//					//
//					pr2vr[B].vr = INVALID__VR;
//				}
//			}
//			else	// B == X64PR_MAX
//			{
//				int curr_vr_have_A = pr2vr[A].vr;
//
//				if (curr_vr_have_A != INVALID__VR)
//				{
//					ERR("%d", before[curr_vr_have_A].pr);
//
//					auto &curr_vr_A_struct = now[curr_vr_have_A];
//
//					//					mc = X64mc(MC_ASSIGN);
//					//					//				mc.s1 = INVALID__VR;
//					//					//				mc.s2 = vr_to_mov;
//					//					mc.pr1 = swap_pr;
//					//					mc.pr2 = A;
//					//					mc.ori_sem = "while";
//					//					x64mcs.push_back(mc);
//					//
//					//					vr2pr[curr_vr_A].pr = swap_pr;
//					//					pr2vr[swap_pr].vr = curr_vr_A;
//					//
//					//					swap_vr = INVALID__VR;
//					//					swap_pr = X64PR_MAX;
//
//					spill_vr(x64mcs, curr_vr_have_A);
//
//					////////////////////////////////////////////////////
//					mc = X64mc(MC_LD, vr);
//					mc.pr1 = A;
//					mc.ori_sem = "ld while";
//					x64mcs.push_back(mc);
//
//					now[vr] = before[vr];
//					pr2vr[A].vr = vr;
//				}
//				else	//  (curr_vr_A == INVALID__VR)
//				{
//					mc = X64mc(MC_LD, vr);
//					mc.pr1 = A;
//					mc.ori_sem = "ld while";
//					x64mcs.push_back(mc);
//
//					now[vr] = before[vr];
//					pr2vr[A].vr = vr;
//				}
//			}
//
//		}
//	}
//}
//
//void reg_alloc_wave__while(Scope *while_scp)
//{
//	//	LOG("scp %s", scp->name.c_str());
//
//	while__pre_work(while_scp);
//
//// try alloc cond write
////	for(int vr : scp->outer_symb_write)
////	{
////		for(int pr : )
////	}
//
//// try alloc body write
//// try alloc cond read
//// try alloc body read
//
//// select cond pr
////	spill_vr(bb->x64mc_alloc_wave, swap_vr);
//
//	////////////////////////////////////////////////////////////
//	// while cond
//
//	vector<VrToPr> pre_cond_vr2pr = vr2pr;
//	vector<PrToVr> pre_cond_pr2vr = pr2vr;
//
//	reg_alloc_wave__scope(while_scp->clds[0]);
//
//	vector<VrToPr> after_cond_vr2pr = vr2pr;
//	vector<PrToVr> after_cond_pr2vr = pr2vr;
//
//	// cond 写了 outer symb（比如 while ((i3 = i3 - 1 + ...) > 0)），
//	// 但没 spill 回 stack！回边时 body load 会拿到旧值。
//	// 修复：在 cond BB 的 jle/jmp 之前把 cond 写过的 outer symb spill 回 stack
//	Scope *cond_scp = while_scp->clds[0];
//	BasicBlock *cond_bb = (BasicBlock*) cond_scp->basic_block;
//	vector<X64mc> &cond_mcs = cond_bb->x64mc_alloc_wave;
//	int jmp_pos = cond_mcs.size();
//	for (int i = cond_mcs.size() - 1; i >= 0; i--)
//	{
//		if (cond_mcs[i].mc_stamp == MC_JLE || cond_mcs[i].mc_stamp == MC_JMP)
//		{
//			jmp_pos = i;
//			break;
//		}
//	}
//	for (int vr = 0; vr < after_cond_vr2pr.size(); vr++)
//	{
//		if (after_cond_vr2pr[vr].pr == X64PR_MAX)
//			continue;
//		if (!(after_cond_vr2pr[vr].u & VR_USAGE_WRITE))
//			continue;
//		X64mc mc(MC_ST, vr);
//		mc.pr1 = after_cond_vr2pr[vr].pr;
//		mc.ori_sem = "cond force spill";
//		cond_mcs.insert(cond_mcs.begin() + jmp_pos, mc);
//		jmp_pos++;
//	}
//
//	////////////////////////////////////////////////////////////
//	// while body
//	reg_alloc_wave__scope(while_scp->clds[1]);
//
//	vector<VrToPr> after_body_vr2pr = vr2pr;
//	vector<PrToVr> after_body_pr2vr = pr2vr;
//
//	////////////////////////////////////////////////////////////
//	printf("\n============== swap %d ==============\n", while_scp->id);
//	dump_vr2pr(pre_cond_vr2pr, "pre cond " + to_string(while_scp->id));
//	dump_vr2pr(after_cond_vr2pr, "after cond " + to_string(while_scp->id));
//	dump_vr2pr(after_body_vr2pr, "after body " + to_string(while_scp->id));
//
//	assert(while_scp->clds[2]->sem_stamp == SEM_WHILE_BODY_suffix);
//	Scope *body_suffix = while_scp->clds[2];
//	BasicBlock *bs_bb = (BasicBlock*) body_suffix->basic_block;
//
//	// recover: 把 PR 状态恢复到 pre_cond 位置
//	while__recover_pr(body_suffix,
//	    pre_cond_vr2pr, pre_cond_pr2vr,
//	    after_body_vr2pr, after_body_pr2vr);
//
//	dump_vr2pr(after_body_vr2pr, "after body swaped " + to_string(while_scp->id));
//
//	// body_suffix force spill: recover 之后 mov 已把值搬回 pre_cond PR
//	// 强制写 stack → body 写的新值在回边和退出路径都能拿到
//	for (int vr = 0; vr < pre_cond_vr2pr.size(); vr++)
//	{
//		int pr = pre_cond_vr2pr[vr].pr;
//		if (pr == X64PR_MAX)
//			continue;
//		X64mc mc(MC_ST, vr);
//		mc.pr1 = pr;
//		mc.ori_sem = "while force spill";
//		bs_bb->x64mc_alloc_wave.push_back(mc);
//	}
//
//	// vr2pr = after_cond + 清内层临时变量:
//	// after_cond 里被 cond evict 的变量 pr==MAX → 外层从 stack load
//	// 清掉内层引入的临时变量（pre_cond 没有、after_cond 有 pr 的），防止泄漏 pr2vr
//	vr2pr = after_cond_vr2pr;
//	pr2vr = after_cond_pr2vr;
//	for (int vr = 0; vr < vr2pr.size(); vr++)
//	{
//		if (vr2pr[vr].pr == X64PR_MAX)
//			continue;
//		if (pre_cond_vr2pr[vr].pr == X64PR_MAX)
//		{
//			pr2vr[vr2pr[vr].pr].vr = INVALID__VR;
//			vr2pr[vr].pr = X64PR_MAX;
//			vr2pr[vr].u = VR_USEAGE_INVALID;
//		}
//	}
//
//	dump_vr2pr(vr2pr, "vr2pr " + to_string(while_scp->id));
//	printf("\n");
//
//	while__end_work(while_scp);
//}

