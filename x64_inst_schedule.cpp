/*
 * x64_inst_schdule.cpp
 *
 *  Created on: 2026年9月13日
 *      Author: x
 */

#include "h.h"
#include "x64_mc.h"

extern vector<X64_mc> x64mc;
extern vector<Mc_dep> mc_dep;
extern vector<Mc_dep> mc_bdep;
extern vector<X64_mc> x64mc_scheduled;

void create_dependcy__a_wait_b(int use, int def)
{
	mc_dep[use].mcs.push_back(def);
	mc_bdep[def].mcs.push_back(use);
}
static void dump_chain()
{

	printf("dep\n");
	for (int i = 0; i < mc_dep.size(); i++)
	{
		auto &v = mc_dep[i].mcs;

		std::sort(v.begin(), v.end());
		auto it = std::adjacent_find(v.begin(), v.end());
		if (it != v.end())
			ERR();

		mc_dep[i].edges = v.size();
		for (auto r : v)
			printf("%d %d\n", i, r);
	}

	printf("\nbdep\n");
	for (int i = 0; i < mc_bdep.size(); i++)
	{
		auto &v = mc_bdep[i].mcs;

		std::sort(v.begin(), v.end());
		auto it = std::adjacent_find(v.begin(), v.end());
		if (it != v.end())
			ERR();

		for (auto r : v)
			printf("%d %d\n", i, r);
	}

}
void gen_use_def_chain()
{
	vector<int> prev_w_mc_of_vr(vreg.id + 1, -1);
	vector<int> prev_r_mc_of_vr(vreg.id + 1, -1);
	mc_dep.resize(x64mc.size());
	mc_bdep.resize(x64mc.size());

	printf("%zu, %zu\n", x64mc.size(), prev_w_mc_of_vr.size());
	int prev_ret = -1;

	for (int i = 0; i < x64mc.size(); i++)
	{
		Machine_code_type ty = x64mc[i].mcty;

		int s1 = x64mc[i].s1;
		int w1 = prev_w_mc_of_vr[s1];	// write and write ?
		int r1 = prev_r_mc_of_vr[s1];

		int s2 = x64mc[i].s2;
		int w2 = -1;
		int r2 = -1;
		if (s2 >= 0)
		{
			w2 = prev_w_mc_of_vr[s2];
			r2 = prev_r_mc_of_vr[s2];
		}

		switch (ty)
		{
		case mc_li:
			prev_w_mc_of_vr[s1] = i;
			break;

		case mc_assign:
			if (w1 >= 0)
				create_dependcy__a_wait_b(i, w1);

			if (r1 >= 0)
				create_dependcy__a_wait_b(i, r1);

			prev_w_mc_of_vr[s1] = i;

			if (w2 >= 0)
				create_dependcy__a_wait_b(i, w2);

			prev_r_mc_of_vr[s2] = i;
			break;

		case mc_add:
			case mc_sub:
			case mc_imul:
			case mc_div:
			if (w1 >= 0)
				create_dependcy__a_wait_b(i, w1);

			if (r1 >= 0)
				create_dependcy__a_wait_b(i, r1);

			prev_w_mc_of_vr[s1] = i;
			prev_r_mc_of_vr[s1] = i;

			if (w2 >= 0)
				create_dependcy__a_wait_b(i, w2);

			prev_r_mc_of_vr[s2] = i;
			break;

		case mc_ret:
			if (w1 >= 0)
				create_dependcy__a_wait_b(i, w1);

			prev_r_mc_of_vr[s1] = i;

			for (int j = prev_ret + 1; j < i; j++)
			{
				if (j == w1)
					continue;
				create_dependcy__a_wait_b(i, j);
			}
			prev_ret = i;
			break;

		default:
			ERR("%d \n", ty);
			break;
		}
	}
}
int get_mc_latency(int mc_id)
{
	int &c = x64mc[mc_id].chain_latency;
	if (c >= 0)
		return c;

	int &n = x64mc[mc_id].latency;
	assert(n >= 0);
	c = n;

	auto &mcs = mc_bdep[mc_id].mcs;
	int mx = 0;
	for (int i = 0; i < mcs.size(); i++)
	{
		int mc = mcs[i];
		int r = get_mc_latency(mc);
		mx = std::max(mx, r);
	}

	c += mx;
	return c;
}
void gen_schedule_chain_latency()
{
	for (int i = 0; i < mc_bdep.size(); i++)
	{
		get_mc_latency(i);
	}
}

using std::multimap;
multimap<int, int> ready;
vector<int> running;
int free_add_sub_unit = 2;
int free_imul_unit = 1;
int free_div_unit = 1;

bool get_function_unit(int mc)
{
	Machine_code_type ty = x64mc[mc].mcty;

	switch (ty)
	{
	case mc_li:
		case mc_assign:
		return true;

	case mc_add:
		case mc_sub:
		if (free_add_sub_unit > 0)
		{
			free_add_sub_unit--;
			return true;
		}
		break;

	case mc_imul:
		if (free_imul_unit > 0)
		{
			free_imul_unit--;
			return true;
		}
		break;

	case mc_div:
		if (free_div_unit > 0)
		{
			free_div_unit--;
			return true;
		}
		break;

	case mc_ret:
		return true;
		break;

	default:
		ERR("%d \n", ty);
		break;
	}

	return 0;
}
void free_function_unit(int mc)
{
	Machine_code_type ty = x64mc[mc].mcty;
	switch (ty)
	{
	case mc_li:
		case mc_assign:
		break;

	case mc_add:
		case mc_sub:
		free_add_sub_unit++;
		break;

	case mc_imul:
		free_imul_unit++;
		break;

	case mc_div:
		free_div_unit++;
		break;

	case mc_ret:
		break;

	default:
		ERR("%d \n", ty);
		break;
	}
}
int mc_select()
{
	bool ok = false;
	for (auto it = ready.end(); it != ready.begin();)
	{
		it--;
		int mc = it->second;
		if (get_function_unit(mc))
		{
			ready.erase(it);
			return mc;
		}
	}
	return -1;
}
void init_ready_queue()
{
	for (int i = 0; i < mc_dep.size(); i++)
	{
		if (mc_dep[i].edges == 0)
			ready.insert(
					{ x64mc[i].chain_latency, i });
	}

	if (ready.size() == 0)
		ERR();
}

void update_ready_queue(int mc)
{
	auto &mcs = mc_bdep[mc].mcs;
	for (int i = 0; i < mcs.size(); i++)
	{
		int k = mcs[i];
		mc_dep[k].edges--;
		if (mc_dep[k].edges == 0)
			ready.insert(
					{ x64mc[k].chain_latency, k });
	}
}
void _mc_schedule()
{
	/*
	 * 	for mc : x64mc
	 * 		if mc has no dep
	 * 			move mc to ready
	 *
	 *	for cycle
	 *		for mc : ready
	 *			select mc, critical path select, function unit limit
	 *				rm mc in ready
	 *				run mc:
	 *				mc.start_time = cycle
	 *				cp mc to x64mc_scheduled & running
	 *				if x64mc_scheduled == x64mc
	 *					break
	 *
	 *		for mc : running
	 *			if mc.start_time + mc.latency >= cycle
	 *				rm mc in running
	 *				mv mc->edges[i] to ready if dep[i].deps == 0
	 */

	int cycle = 0;

	init_ready_queue();

	while (!running.empty() || !ready.empty())
	{
		for (auto it = running.begin(); it != running.end();)
		{
			int mc = *it;
			if (cycle >= x64mc[mc].start_cycle + x64mc[mc].latency)
			{
				free_function_unit(mc);
				update_ready_queue(mc);

				it = running.erase(it);
				continue;
			}
			it++;
		}

		int mc = 0;
		while ((mc = mc_select()) >= 0)
		{
			x64mc[mc].start_cycle = cycle;
			x64mc_scheduled.push_back(x64mc[mc]);
			running.push_back(mc);
		}

//		if (x64mc_scheduled.size() == x64mc.size())
//			break;
		cycle++;
	}

	auto &t = *x64mc_scheduled.rbegin();
	if (t.mcty != mc_ret)
	{
		printf("x64mc_scheduled last: %s %s, s1 %%%d, s2 %%%d, cycle: %d-%d\n",
				t.mc_code.c_str(), t.ori_sem.c_str(),
				t.s1, t.s2,
				t.start_cycle, t.chain_latency);
	}
}

void mc_schedule()
{
	gen_use_def_chain();
	dump_chain();

	gen_schedule_chain_latency();

	_mc_schedule();
	dump_mc(x64mc_scheduled);
}

