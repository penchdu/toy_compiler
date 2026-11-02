/*
 * dependcy.cpp
 *
 *  Created on: 2026�?�?7�?
 *      Author: x
 */
#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

vector<int> prev_write;
vector<vector<int>> prev_read;
vector<McDepend> mcs_predecessor;
vector<McDepend> mcs_successor;

void create_dependcy_RAW(int a, int b)
{
	assert(a >= 0);
	if (b < 0)
		return;

	if (a == b)
		return;

	auto &pred = mcs_predecessor[a].mcs;
	if (std::find(pred.begin(), pred.end(), b) != pred.end())
		return;

	// a depend on b, data flow: b -> a
	mcs_predecessor[a].mcs.push_back(b);
	mcs_successor[b].mcs.push_back(a);
}
void create_dependcy_WAW(int a, int b)
{
	create_dependcy_RAW(a, b);
}
void create_dependcy_WAR(int a, vector<int> &_prev_read)
{
	for (int b : _prev_read)
		create_dependcy_RAW(a, b);
}

void gen_use_def_chain(vector<X64mc> &x64mc, Scope *scp)
{
	prev_write.clear();
	prev_write.resize(vr_size, -1);
	prev_read.clear();
	prev_read.resize(vr_size);

	mcs_predecessor.clear();
	mcs_predecessor.resize(x64mc.size());
	mcs_successor.clear();
	mcs_successor.resize(x64mc.size());

	// ---- SSA version ordering: higher versions depend on all lower versions ----
	// prevents scheduler from interleaving different SSA versions of the same variable,
	// which would increase simultaneous live range overlap and cause extra spills.
	// NOTE: disabled — out_base_vr lives in parent BB, not reachable within single BB.
	//       No measurable benefit on time_compare, one case even regressed.
#if 0
	if (scp && scp->outer_symb_table_ptr)
	{
		for (Symbol *symb : *scp->outer_symb_table_ptr)
		{
			auto &ssa = symb->ssa;
			for (int i = ssa.size() - 1; i >= 0; i--)
			{
				int b_vr = ssa[i];
				int a_vr = (i == 0) ? symb->out_base_vr : ssa[i - 1];

				for (int idx_b = 0; idx_b < x64mc.size(); idx_b++)
				{
					if (x64mc[idx_b].dst == b_vr)
					{
						for (int idx_a = 0; idx_a < x64mc.size(); idx_a++)
						{
							auto &mc_a = x64mc[idx_a];
							if (mc_a.dst == a_vr || mc_a.s1 == a_vr || mc_a.s2 == a_vr)
							{
								// skip cp_ssa: writes a_vr(base) by reading an SSA version
								if (mc_a.dst == a_vr && std::find(ssa.begin(), ssa.end(), mc_a.s1) != ssa.end())
									continue;
								create_dependcy_RAW(idx_b, idx_a);
							}
						}
						break;
					}
				}
			}
		}
	}
#endif

	for (int idx = 0; idx < x64mc.size(); idx++)
	{
		MachineCodeStamp mc_stamp = x64mc[idx].stamp;
		int s1 = x64mc[idx].s1;
		int s2 = x64mc[idx].s2;
		int dst = x64mc[idx].dst;

		switch (mc_stamp)
		{
		case MC_ASSIGN:
			assert(dst != s1);
			assert(dst >= 0);
			assert(s1 >= 0);
			assert(s2 < 0);
			create_dependcy_WAW(idx, prev_write[dst]);
			create_dependcy_WAR(idx, prev_read[dst]);
			prev_write[dst] = idx;
			prev_read[dst].clear();

			create_dependcy_RAW(idx, prev_write[s1]);
			prev_read[s1].push_back(idx);
			break;

		case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			case MC_DIV:
			//			create_dependcy_RAW(idx, prev_write[s1]);
//			create_dependcy_WAR(idx, prev_read[s1]);
//			prev_write[s1] = idx;
//			prev_read[s1].clear();
//
//			create_dependcy_RAW(idx, prev_write[s2]);
//			prev_read[s2].push_back(idx);
//			break;

		case MC_CMP_E:	// ???
		case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:
			create_dependcy_RAW(idx, prev_write[s1]);
			prev_read[s1].push_back(idx);

			create_dependcy_RAW(idx, prev_write[s2]);
			prev_read[s2].push_back(idx);

			create_dependcy_WAW(idx, prev_write[dst]);
			create_dependcy_WAR(idx, prev_read[dst]);
			prev_write[dst] = idx;
			prev_read[dst].clear();
			break;

		case MC_TEST:
			case MC_SAVE_RET:
			//			printf("MC_SAVE_RET_VALUE %d %d, %lu\n", s1, s2, prev_read.size());
			create_dependcy_RAW(idx, prev_write[s1]);
			prev_read[s1].push_back(idx);
			break;

//		case MC_LI:
//			create_dependcy_WAW(idx, prev_write[s1]);
//			create_dependcy_WAR(idx, prev_read[s1]);
//			prev_write[s1] = idx;
//			prev_read[s1].clear();
//			break;
		default:
			ERR("%d \n", mc_stamp);
			break;
		}
	}

	for (auto &r : mcs_predecessor)
		r.edges = r.mcs.size();
	for (auto &r : mcs_successor)
		r.edges = r.mcs.size();

}
static void print_mc_err(int a, int b, const X64mc &mc)
{
	int dst = mc.dst;
	int s1 = mc.s1;
	int s2 = mc.s2;
	MachineCodeStamp stamp = mc.stamp;

	printf("%d %d, %s %s(%%%d) %s(%%%d) %s(%%%d) \n", a, b,
	    mc_info[stamp].mc_code,
	    dst >= 0 ? declare_record.ast[dst]->tk.src.c_str() : "", dst,
	    s1 >= 0 ? declare_record.ast[s1]->tk.src.c_str() : "", s1,
	    s2 >= 0 ? declare_record.ast[s2]->tk.src.c_str() : "", s2);
}
void dump_chain(vector<X64mc> &x64mc)
{
	printf("dep\n");
	for (int i = 0; i < mcs_predecessor.size(); i++)
	{
		auto &v = mcs_predecessor[i].mcs;

		std::sort(v.begin(), v.end());
		auto it = std::adjacent_find(v.begin(), v.end());
		if (it != v.end())
			ERR();

		for (auto r : v)
			print_mc_err(i, r, x64mc[i]);
	}

	printf("\nbdep\n");
	for (int i = 0; i < mcs_successor.size(); i++)
	{
		auto &v = mcs_successor[i].mcs;

		std::sort(v.begin(), v.end());
		auto it = std::adjacent_find(v.begin(), v.end());
		if (it != v.end())
			ERR();

		for (auto r : v)
			printf("%d %d,    ", i, r);
	}
	printf("\n");

}
int get_mc_latency(vector<X64mc> &x64mc, int mc_id)
{
	int &chain_latency = x64mc[mc_id].chain_latency;
	if (chain_latency >= 0)
		return chain_latency;

	auto stamp = x64mc[mc_id].stamp;

	chain_latency = mc_info[stamp].latency;
	assert(chain_latency >= 0);

	auto &mcs = mcs_successor[mc_id].mcs;
	int mx = 0;
	for (int i = 0; i < mcs.size(); i++)
	{
		int mc = mcs[i];
		int r = get_mc_latency(x64mc, mc);
		mx = std::max(mx, r);
	}

	chain_latency += mx;
	return chain_latency;
}
void gen_schdu_chain_latency(vector<X64mc> &x64mc)
{
	for (int i = 0; i < mcs_successor.size(); i++)
		get_mc_latency(x64mc, i);
}