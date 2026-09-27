/*
 * dependcy.cpp
 *
 *  Created on: 2026年9月27日
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
	if (b < 0)
		return;

	auto &pred = mcs_predecessor[a].mcs;

	if (std::find(pred.begin(), pred.end(), b) != pred.end())
	{
		return;
	}

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
	auto &pred = mcs_predecessor[a].mcs;

	for (int b : _prev_read)
	{
		if (std::find(pred.begin(), pred.end(), b) != pred.end())
			continue;

		// a depend on b, data flow: b -> a
		mcs_predecessor[a].mcs.push_back(b);
		mcs_successor[b].mcs.push_back(a);
	}
}

void gen_use_def_chain(vector<X64mc> &x64mc)
{
	memset(prev_write.data(), -1, prev_write.size() * sizeof(int));
	for (auto &v : prev_read)
		v.clear();

	mcs_predecessor.clear();
	mcs_successor.clear();
	mcs_predecessor.resize(x64mc.size());
	mcs_successor.resize(x64mc.size());

	LOG("%zu, %zu\n", x64mc.size(), prev_write.size());

	for (int mc = 0; mc < x64mc.size(); mc++)
	{
		MachineCodeStamp mc_stamp = x64mc[mc].mc_stamp;
		int s1 = x64mc[mc].s1;
		int s2 = x64mc[mc].s2;
		int dst = x64mc[mc].dst;

		switch (mc_stamp)
		{
		case MC_LI:
			prev_write[s1] = mc;
			prev_read[s1].clear();
			break;

		case MC_ASSIGN:
			assert(s1 != s2);
			create_dependcy_WAW(mc, prev_write[s1]);
			create_dependcy_WAR(mc, prev_read[s1]);
			prev_write[s1] = mc;
			prev_read[s1].clear();

			create_dependcy_RAW(mc, prev_write[s2]);
			prev_read[s2].push_back(mc);
			break;

		case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			case MC_DIV:
			create_dependcy_RAW(mc, prev_write[s1]);
			create_dependcy_WAR(mc, prev_read[s1]);
			prev_write[s1] = mc;
			prev_read[s1].clear();

			create_dependcy_RAW(mc, prev_write[s2]);
			prev_read[s2].push_back(mc);
			break;

		case MC_CMP_E:	// ???
		case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:
			create_dependcy_RAW(mc, prev_write[s1]);
			prev_read[s1].push_back(mc);

			create_dependcy_RAW(mc, prev_write[s2]);
			prev_read[s2].push_back(mc);

			create_dependcy_WAW(mc, prev_write[dst]);
			create_dependcy_WAR(mc, prev_read[dst]);
			prev_write[dst] = mc;
			prev_read[dst].clear();
			break;

		case MC_SAVE_RET_VALUE:
			printf("MC_SAVE_RET_VALUE %d %d, %lu\n", s1, s2, prev_read.size());
			create_dependcy_RAW(mc, prev_write[s1]);
			prev_read[s1].push_back(mc);
			break;

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
void dump_chain()
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
			printf("%d %d,    ", i, r);
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

	chain_latency = x64mc[mc_id].latency;
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
