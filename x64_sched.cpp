/*
 * x64_inst_schdule.cpp
 *
 *  Created on: 2026年9月13日
 *      Author: x
 */

#include "h.h"
#include "x64_back_end.h"
#include "scope.h"

vector<X64mc> x64mc;
vector<X64mc> x64mc_schedued;
vector<X64mc> x64mc_alloced;
vector<McDepend> mcs_pred;
vector<McDepend> mcs_succ;
//
//
//void create_dependcy(int a, int b)
//{
//	// a depend on b, data flow: b -> a
//	mcs_pred[a].mcs.push_back(b);
//	mcs_succ[b].mcs.push_back(a);
//}
//static void dump_chain()
//{
//	printf("dep\n");
//	for (int i = 0; i < mcs_pred.size(); i++)
//	{
//		auto &v = mcs_pred[i].mcs;
//
//		std::sort(v.begin(), v.end());
//		auto it = std::adjacent_find(v.begin(), v.end());
//		if (it != v.end())
//			ERR();
//
//		for (auto r : v)
//			printf("%d %d\n", i, r);
//	}
//
//	printf("\nbdep\n");
//	for (int i = 0; i < mcs_succ.size(); i++)
//	{
//		auto &v = mcs_succ[i].mcs;
//
//		std::sort(v.begin(), v.end());
//		auto it = std::adjacent_find(v.begin(), v.end());
//		if (it != v.end())
//			ERR();
//
//		for (auto r : v)
//			printf("%d %d\n", i, r);
//	}
//
//}
//void gen_use_def_chain()
//{
//	vector<int> prev_w_mc_of_vr(vrm.id + 1, -1);
//	vector<int> prev_r_mc_of_vr(vrm.id + 1, -1);
//	mcs_pred.resize(x64mc.size());
//	mcs_succ.resize(x64mc.size());
//
//	LOG("%zu, %zu\n", x64mc.size(), prev_w_mc_of_vr.size());
//	int prev_ret = -1;
//
//	for (int i = 0; i < x64mc.size(); i++)
//	{
//		MachineCodeType ty = x64mc[i].mcty;
//
//		int s1 = x64mc[i].s1;
//		int w1 = prev_w_mc_of_vr[s1];	// write and write ?
//		int r1 = prev_r_mc_of_vr[s1];
//
//		int s2 = x64mc[i].s2;
//		int w2 = -1;
//		int r2 = -1;
//		if (s2 >= 0)
//		{
//			w2 = prev_w_mc_of_vr[s2];
//			r2 = prev_r_mc_of_vr[s2];
//		}
//
//		switch (ty)
//		{
//		case MC_LI:
//			prev_w_mc_of_vr[s1] = i;
//			break;
//
//		case MC_ASSIGN:
//			if (w1 >= 0)
//				create_dependcy(i, w1);
//
//			if (r1 >= 0)
//				create_dependcy(i, r1);
//
//			prev_w_mc_of_vr[s1] = i;
//
//			if (w2 >= 0)
//				create_dependcy(i, w2);
//
//			prev_r_mc_of_vr[s2] = i;
//			break;
//
//		case MC_ADD:
//			case MC_SUB:
//			case MC_IMUL:
//			case MC_DIV:
//			if (w1 >= 0)
//				create_dependcy(i, w1);
//
//			if (r1 >= 0)
//				create_dependcy(i, r1);
//
//			prev_w_mc_of_vr[s1] = i;
//			prev_r_mc_of_vr[s1] = i;
//
//			if (w2 >= 0)
//				create_dependcy(i, w2);
//
//			prev_r_mc_of_vr[s2] = i;
//			break;
//
//		case MC_RET:
//			if (w1 >= 0)
//				create_dependcy(i, w1);
//
//			prev_r_mc_of_vr[s1] = i;
//
//			for (int j = prev_ret + 1; j < i; j++)
//			{
//				if (j == w1)
//					continue;
//				create_dependcy(i, j);
//			}
//			prev_ret = i;
//			break;
//
//		default:
//			ERR("%d \n", ty);
//			break;
//		}
//	}
//
//	for (auto &r : mcs_pred)
//		r.edges = r.mcs.size();
//	for (auto &r : mcs_succ)
//		r.edges = r.mcs.size();
//
//}
//int get_mc_latency(int mc_id)
//{
//	int &c = x64mc[mc_id].chain_latency;
//	if (c >= 0)
//		return c;
//
//	int &n = x64mc[mc_id].latency;
//	assert(n >= 0);
//	c = n;
//
//	auto &mcs = mcs_succ[mc_id].mcs;
//	int mx = 0;
//	for (int i = 0; i < mcs.size(); i++)
//	{
//		int mc = mcs[i];
//		int r = get_mc_latency(mc);
//		mx = std::max(mx, r);
//	}
//
//	c += mx;
//	return c;
//}
//void gen_schdu_chain_latency()
//{
//	for (int i = 0; i < mcs_succ.size(); i++)
//	{
//		get_mc_latency(i);
//	}
//}
//
//using std::multimap;
//multimap<int, int> ready;
//vector<int> running;
//
//const int add_sub_unit = 2;
//const int imul_unit = 1;
//const int div_unit = 1;
//
//int free_add_sub_unit = add_sub_unit;
//int free_imul_unit = imul_unit;
//int free_div_unit = div_unit;
//
//bool get_function_unit(int mc)
//{
//	MachineCodeType ty = x64mc[mc].mcty;
//
//	switch (ty)
//	{
//	case MC_LI:
//		case MC_ASSIGN:
//		return true;
//
//	case MC_ADD:
//		case MC_SUB:
//		if (free_add_sub_unit > 0)
//		{
//			free_add_sub_unit--;
//			return true;
//		}
//		break;
//
//	case MC_IMUL:
//		if (free_imul_unit > 0)
//		{
//			free_imul_unit--;
//			return true;
//		}
//		break;
//
//	case MC_DIV:
//		if (free_div_unit > 0)
//		{
//			free_div_unit--;
//			return true;
//		}
//		break;
//
//	case MC_RET:
//		return true;
//		break;
//
//	default:
//		ERR("%d \n", ty);
//		break;
//	}
//
//	return 0;
//}
//void free_function_unit(int mc)
//{
//	MachineCodeType ty = x64mc[mc].mcty;
//	switch (ty)
//	{
//	case MC_LI:
//		case MC_ASSIGN:
//		break;
//
//	case MC_ADD:
//		case MC_SUB:
//		free_add_sub_unit++;
//		break;
//
//	case MC_IMUL:
//		free_imul_unit++;
//		break;
//
//	case MC_DIV:
//		free_div_unit++;
//		break;
//
//	case MC_RET:
//		break;
//
//	default:
//		ERR("%d \n", ty);
//		break;
//	}
//
//	assert(free_add_sub_unit <= add_sub_unit);
//	assert(free_imul_unit <= imul_unit);
//	assert(free_div_unit <= div_unit);
//}
//
//int mc_select()
//{
//	bool ok = false;
//	for (auto it = ready.end(); it != ready.begin();)
//	{
//		it--;
//		int mc = it->second;
//		if (get_function_unit(mc))
//		{
//			ready.erase(it);
//			return mc;
//		}
//	}
//	return -1;
//}
//
//void init_ready_queue()
//{
//	for (int i = 0; i < mcs_pred.size(); i++)
//	{
//		if (mcs_pred[i].edges == 0)
//			ready.insert(
//			        {x64mc[i].chain_latency, i});
//	}
//
//	if (ready.size() == 0)
//		ERR();
//}
//void finish_mc__update_ready_queue(int mc)
//{
//	auto &v = mcs_succ[mc].mcs;
//	for (auto mc : v)
//	{
//		mcs_pred[mc].edges--;
//		assert(mcs_pred[mc].edges >= 0);
//
//		if (mcs_pred[mc].edges == 0)
//			ready.insert(
//			        {x64mc[mc].chain_latency, mc});
//	}
//}
//
//static void mc_schdu(Scope *scp)
//{
//	/*
//	 * 	for mc : x64mc
//	 * 		if mc has no dep
//	 * 			move mc to ready
//	 *
//	 *	for cycle
//	 *		for mc : ready
//	 *			select mc, critical path select, function unit limit
//	 *				rm mc in ready
//	 *				run mc:
//	 *				mc.start_time = cycle
//	 *				cp mc to x64mc_scheduled & running
//	 *				if x64mc_scheduled.size == x64mc.size
//	 *					break
//	 *
//	 *		for mc : running
//	 *			if mc.start_time + mc.latency >= cycle
//	 *				rm mc in running
//	 *				mv mc->edges[i] to ready if dep[i].deps == 0
//	 */
//
//	//	LOG("scp %s", scp->name.c_str());
//	printf("scp %s:\n", scp->name.c_str());
//
//	int cycle = 0;
//	init_ready_queue();
//
//	while (!running.empty() || !ready.empty())
//	{
//		for (auto it = running.begin(); it != running.end();)
//		{
//			int mc = *it;
//			assert(x64mc[mc].start_cycle >= 0);
//			assert(x64mc[mc].latency > 0);
//
//			if (cycle >= x64mc[mc].start_cycle + x64mc[mc].latency)
//			{
//				free_function_unit(mc);
//				finish_mc__update_ready_queue(mc);
//
//				it = running.erase(it);
//				continue;
//			}
//			it++;
//		}
//
//		// select from ready[]
//		while (1)
//		{
//			int mc = mc_select();
//			if (mc < 0)
//				break;
//
//			x64mc[mc].start_cycle = cycle;
//			x64mc_schedued.push_back(x64mc[mc]);
//			running.push_back(mc);
//
//			assert(x64mc[mc].start_cycle >= 0);
//			assert(x64mc[mc].latency > 0);
//		}
//
////		if (x64mc_scheduled.size() == x64mc.size())
////			break;
//		cycle++;
//	}
////	printf("max cycle: %d \n", cycle);
//
//	auto &t = *x64mc_schedued.rbegin();
//	if (t.mcty != MC_RET)
//	{
//		printf("x64mc_scheduled last: %s %s, s1 %%%d, s2 %%%d, cycle: %d-%d\n",
//		        t.asm_code.c_str(), t.ori_sem.c_str(),
//		        t.s1, t.s2,
//		        t.start_cycle, t.chain_latency);
//	}
//
//	for (Scope *p : scp->clds)
//		mc_schdu(p);
//}
//static void dump_scope(Scope *scp)
//{
////	LOG("scp %s", scp->name.c_str());
//	printf("scp %s:\n", scp->name.c_str());
//
//	dump_mc(x64mc_schedued);
//
//	for (Scope *p : scp->clds)
//		dump_scope(p);
//}
//static void dump_schedued_mc()
//{
//	printf("\n========== mc ==========\n");
//
//	// push rbp
//	// mov rbp, rsp
//	// sub rsp, <num>
//	printf("push rbp\n");
//	printf("mov rbp, rsp\n");
//	printf("sub rsp, %d\n\n", vrm.offset);
//
//	dump_scope(&file_scp);
//
//	printf("\nmov rsp, rbp \n");
//	printf("pop rbp \n");
//	printf("ret \n\n");
//}
//void mc_schedule()
//{
//	gen_use_def_chain();
//	dump_chain();
//	gen_schdu_chain_latency();
//
//	mc_schdu(&file_scp);
//	dump_schedued_mc();
//}
