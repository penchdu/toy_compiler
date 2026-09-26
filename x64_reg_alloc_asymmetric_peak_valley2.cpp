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
