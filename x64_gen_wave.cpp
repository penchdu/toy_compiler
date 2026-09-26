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



void dump_wave(BasicBlock &bb)
{
	static const char *level[] = {
	    " ",
	    "▁",
	    "▂",
	    "▃",
	    "▄",
	    "▅",
	    "▆",
	    "▇",
	    "█",
	};
	printf("\n========== wave ==========\n");

	for (int vr : bb.vrids)
	{
		const vector<float> &score = vrwave[vr].score;
		if (score.empty())
			continue;

		float max_score =
		    *std::max_element(score.begin(), score.end());

		printf("VR %-4d | ", vr);

		if (max_score <= 0.0f)
		{
			for (size_t pc = 0; pc < score.size(); ++pc)
				printf(" ");
			printf("\n");
			continue;
		}

		for (float s : score)
		{
			float ratio = s / max_score;
			int h = (int) (ratio * 8.0f);
			if (h < 0)
				h = 0;
			if (h > 8)
				h = 8;
			printf("%s", level[h]);
		}
		printf("  max=%.2f\n", max_score);
	}
	printf("     ");

	if (!bb.vrids.empty())
	{
		int n = vrwave[bb.vrids[0]].score.size();
		printf("   ");
		for (int i = 0; i < n; ++i)
		{
			if (i % 10 == 0)
				printf("|");
			else
				printf("-");
		}
		printf("\n     ");
		for (int i = 0; i < n; ++i)
		{
			if (i % 10 == 0)
				printf("%-10d", i);
		}
		printf("\n");
	}
	printf("===========================\n");
}
#include <cstdio>
#include <cstdlib>

void dump_wave_gnuplot(Wave &w, int vr)
{
	char data_file[128];

	sprintf(data_file, "/tmp/vr_wave_%d.dat", vr);
	FILE *fp = fopen(data_file, "w");
	if (!fp)
	{
		perror("open wave data");
		return;
	}

	for (int pc = 0; pc < w.score.size(); pc++)
	{
		fprintf(fp, "%d %.6f\n", pc, w.score[pc]);
	}
	fclose(fp);

	FILE *gp = popen("gnuplot -persist", "w");
	if (!gp)
	{
		printf("gnuplot open failed\n");
		return;
	}

	fprintf(gp,
	    "set title 'VR %d wave'\n"
		    "set xlabel 'PC'\n"
		    "set ylabel 'Score'\n"
		    "set grid\n"
		    "plot '%s' using 1:2 with lines lw 2 title 'score'\n",
	    vr, data_file);

	fflush(gp);
	pclose(gp);
}
static void dump_all_wave_gnuplot(BasicBlock &bb)
{
	FILE *fp = fopen("/tmp/all_wave.dat", "w");
	int count = 0;

	for (int vr : bb.vrids)
	{
		printf("dump VR %d size=%ld\n",
		    vr,
		    vrwave[vr].score.size());

		for (int pc = 0; pc < vrwave[vr].score.size(); pc++)
		{
			fprintf(fp,
			    "%d %d %.6f\n",
			    pc,
			    vr,
			    vrwave[vr].score[pc]);

			count++;
		}
	}
	fclose(fp);
	printf("dump points=%d\n", count);
}

static void compute_wave_triangle(Wave &w)
{
	constexpr int R = 5;

	const int n = w.insts.size();

	std::fill(w.score.begin(), w.score.end(), 0.0f);

	for (int use_pc = 0; use_pc < n; ++use_pc)
	{
		if (w.insts[use_pc] == 0)
			continue;

		for (int pc = 0; pc < n; ++pc)
		{
			int d = std::abs(pc - use_pc);

			if (d <= R)
			{
				float weight = (float) (R - d) / R;
				w.score[pc] += w.insts[use_pc] * weight;
			}
		}
	}
}
static void compute_wave_decay(Wave &_wave)
{
	int left_radius = 4;
	float decay = 0.8f;

	const int inst_size = _wave.insts.size();
	std::fill(_wave.score.begin(), _wave.score.end(), 0.0f);

	for (int inst_idx = 0; inst_idx < inst_size; inst_idx++)
	{
		int count = _wave.insts[inst_idx];
		if (count == 0)
			continue;

		for (int prev__after_use_idx = 0; prev__after_use_idx < inst_size; prev__after_use_idx++)
		{
//            if (_wave.insts[prev__after_use_idx] == 0)
//                continue;
			float weight = 0.0f;

			if (prev__after_use_idx <= inst_idx)
			{
				// use 前：缓慢建立波峰
				int distance = inst_idx - prev__after_use_idx;

				if (distance <= left_radius)
					weight = (float) (left_radius - distance) / left_radius;
			}
			else
			{
				// use 后：快速指数衰减
				int d = prev__after_use_idx - inst_idx;
				weight = std::exp(-decay * d);
			}

			_wave.score[prev__after_use_idx] += count * weight;
		}
	}
}
static void compute_wave_decay_gemini_improved(Wave &_wave)
{
    const int inst_size = _wave.insts.size();
    std::fill(_wave.score.begin(), _wave.score.end(), 0.0f);

    // 参数调整：
    // 1. left_radius: use 前预热半径（不需要太长，3 即可）
    // 2. decay: 右侧衰减系数（降为 0.35f，让密集使用的波峰能够有效叠加）
    constexpr int left_radius = 3;
    constexpr float decay = 0.35f;

    for (int inst_idx = 0; inst_idx < inst_size; inst_idx++)
    {
        int count = _wave.insts[inst_idx];
        if (count == 0)
            continue;

        for (int pc = 0; pc < inst_size; pc++)
        {
            float weight = 0.0f;

            if (pc < inst_idx)
            {
                // 【use 前 ( Look-ahead )】：采用平滑的二次方或高斯上升，让临近 use 时迅速拉升
                int distance = inst_idx - pc;
                if (distance <= left_radius)
                {
                    float ratio = (float)distance / left_radius;
                    weight = 1.0f - ratio * ratio; // 二次方衰减，比线性更平滑，临近时上升更快
                }
            }
            else if (pc == inst_idx)
            {
                // 【use 当天】：满分 1.0
                weight = 1.0f;
            }
            else
            {
                // 【use 后】：指数衰减
                int d = pc - inst_idx;
                weight = std::exp(-decay * d);
            }

            // 核心：利用 count 和权重累加！
            // 连续使用时，多个 use 点的 weight 会在此处进行叠加 (Superposition)
            _wave.score[pc] += (float)count * weight;
        }
    }
}
void compute_wave(BasicBlock &bb)
{
	for (int vr : bb.vrids)
	{
//    	compute_wave_triangle(wave[vr]);
//		compute_wave_decay(vrwave[vr]);
		compute_wave_decay_gemini_improved(vrwave[vr]);
	}
}

