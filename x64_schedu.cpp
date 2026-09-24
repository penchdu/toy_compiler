/*
 * x64_inst_schdule.cpp
 *
 *  Created on: 2026年9月13日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

vector<int> prev_write_mc;
vector<int> prev_read_mc;
vector<McDepend> mcs_pred;	// predecessor
vector<McDepend> mcs_succ;	// successor

void create_dependcy(int a, int b)
{
    if (b < 0)
        return;

    auto &pred = mcs_pred[a].mcs;

    if (std::find(pred.begin(), pred.end(), b) != pred.end())
        return;

	// a depend on b, data flow: b -> a
	mcs_pred[a].mcs.push_back(b);
	mcs_succ[b].mcs.push_back(a);
}

void gen_use_def_chain(vector<X64mc> &x64mc)
{
	memset(prev_write_mc.data(), 0xFF, prev_write_mc.size() * sizeof(int));
	memset(prev_read_mc.data(), 0xFF, prev_read_mc.size() * sizeof(int));

	mcs_pred.clear();
	mcs_succ.clear();
	mcs_pred.resize(x64mc.size());
	mcs_succ.resize(x64mc.size());

	LOG("%zu, %zu\n", x64mc.size(), prev_write_mc.size());
	int prev_ret = -1;

	for (int i = 0; i < x64mc.size(); i++)
	{
		MachineCodeStamp mc_stamp = x64mc[i].mc_stamp;

		int s1 = x64mc[i].s1;
		int s1_prev_w_mc = prev_write_mc[s1];	// write and write ?
		int s1_prev_read_mc = prev_read_mc[s1];

		int s2 = x64mc[i].s2;
		int s2_prev_write_mc = -1;
		int s2_prev_read_mc = -1;
		if (s2 >= 0)
		{
			s2_prev_write_mc = prev_write_mc[s2];
			s2_prev_read_mc = prev_read_mc[s2];
		}

		int dst = x64mc[i].dst;
		int dst_prev_write_mc = -1;
		int dst_prev_read_mc = -1;
		if (dst >= 0)
		{
			dst_prev_write_mc = prev_write_mc[dst];
			dst_prev_read_mc = prev_read_mc[dst];
		}

		switch (mc_stamp)
		{
		case MC_LI:
			prev_write_mc[s1] = i;
			break;

		case MC_ASSIGN:
			if (s1_prev_w_mc >= 0)
				create_dependcy(i, s1_prev_w_mc);

			if (s1_prev_read_mc >= 0)
				create_dependcy(i, s1_prev_read_mc);

			prev_write_mc[s1] = i;

			if (s2_prev_write_mc >= 0)
				create_dependcy(i, s2_prev_write_mc);

			prev_read_mc[s2] = i;
			break;

		case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			case MC_DIV:
			if (s1_prev_w_mc >= 0)
				create_dependcy(i, s1_prev_w_mc);

			if (s1_prev_read_mc >= 0)
				create_dependcy(i, s1_prev_read_mc);

			prev_write_mc[s1] = i;
			prev_read_mc[s1] = i;

			if (s2_prev_write_mc >= 0)
				create_dependcy(i, s2_prev_write_mc);

			prev_read_mc[s2] = i;
			break;

		case MC_CMP_E:	// ???
		case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:
			if (s1_prev_w_mc >= 0)
				create_dependcy(i, s1_prev_w_mc);

			prev_read_mc[s1] = i;

			if (s2_prev_write_mc >= 0)
				create_dependcy(i, s2_prev_write_mc);

			prev_read_mc[s2] = i;

			if (dst_prev_write_mc >= 0)
				create_dependcy(i, dst_prev_write_mc);

			if (dst_prev_read_mc >= 0)
				create_dependcy(i, dst_prev_read_mc);

			prev_write_mc[dst] = i;
			break;

//		case MC_SET_E:
//			case MC_SET_NE:
//			case MC_SET_L:
//			case MC_SET_LE:
//			case MC_SET_G:
//			case MC_SET_GE:
//			if (s1_prev_w_mc >= 0)
//				create_dependcy(i, s1_prev_w_mc);
//
//			if (s1_prev_read_mc >= 0)
//				create_dependcy(i, s1_prev_read_mc);
//
//			prev_write_mc[s1] = i;
//
//			assert((s2_prev_write_mc < 0));
//			break;

		case MC_SAVE_RET_VALUE:
			if (s1_prev_w_mc >= 0)
				create_dependcy(i, s1_prev_w_mc);

			prev_read_mc[s1] = i;

			for (int j = prev_ret + 1; j < i; j++)
			{
				if (j == s1_prev_w_mc)
					continue;
				create_dependcy(i, j);
			}
			prev_ret = i;
			break;

		default:
			ERR("%d \n", mc_stamp);
			break;
		}
	}

	for (auto &r : mcs_pred)
		r.edges = r.mcs.size();
	for (auto &r : mcs_succ)
		r.edges = r.mcs.size();

}
static void dump_chain()
{
	printf("dep\n");
	for (int i = 0; i < mcs_pred.size(); i++)
	{
		auto &v = mcs_pred[i].mcs;

		std::sort(v.begin(), v.end());
		auto it = std::adjacent_find(v.begin(), v.end());
		if (it != v.end())
			ERR();

		for (auto r : v)
			printf("%d %d, ", i, r);
	}

	printf("\nbdep\n");
	for (int i = 0; i < mcs_succ.size(); i++)
	{
		auto &v = mcs_succ[i].mcs;

		std::sort(v.begin(), v.end());
		auto it = std::adjacent_find(v.begin(), v.end());
		if (it != v.end())
			ERR();

		for (auto r : v)
			printf("%d %d, ", i, r);
	}
	printf("\n");

}
int get_mc_latency(vector<X64mc> &x64mc, int mc_id)
{
	int &c = x64mc[mc_id].chain_latency;
	if (c >= 0)
		return c;

	int &n = x64mc[mc_id].latency;
	assert(n >= 0);
	c = n;

	auto &mcs = mcs_succ[mc_id].mcs;
	int mx = 0;
	for (int i = 0; i < mcs.size(); i++)
	{
		int mc = mcs[i];
		int r = get_mc_latency(x64mc, mc);
		mx = std::max(mx, r);
	}

	c += mx;
	return c;
}
void gen_schdu_chain_latency(vector<X64mc> &x64mc)
{
	for (int i = 0; i < mcs_succ.size(); i++)
	{
		get_mc_latency(x64mc, i);
	}
}

using std::multimap;
multimap<int, int> ready;
vector<int> running;

const int alu_unit = 2;
const int imul_unit = 1;
const int div_unit = 1;

int free_alu_unit = alu_unit;
int free_imul_unit = imul_unit;
int free_div_unit = div_unit;

bool get_function_unit(MachineCodeStamp stamp)
{
	switch (stamp)
	{
	case MC_LI:
		case MC_ASSIGN:
		return true;

	case MC_ADD:
		case MC_SUB:
		case MC_CMP_E:
		case MC_CMP_NE:
		case MC_CMP_L:
		case MC_CMP_LE:
		case MC_CMP_G:
		case MC_CMP_GE:

		if (free_alu_unit > 0)
		{
			free_alu_unit--;
			return true;
		}
		break;

	case MC_IMUL:
		if (free_imul_unit > 0)
		{
			free_imul_unit--;
			return true;
		}
		break;

	case MC_DIV:
		if (free_div_unit > 0)
		{
			free_div_unit--;
			return true;
		}
		break;

	case MC_SAVE_RET_VALUE:
		return true;
		break;

	default:
		ERR("%d \n", stamp);
		break;
	}

	return 0;
}
void free_function_unit(MachineCodeStamp stamp)
{
	switch (stamp)
	{
	case MC_LI:
		case MC_ASSIGN:
		break;

	case MC_ADD:
		case MC_SUB:
		case MC_CMP_E:
		case MC_CMP_NE:
		case MC_CMP_L:
		case MC_CMP_LE:
		case MC_CMP_G:
		case MC_CMP_GE:

		free_alu_unit++;
		break;

	case MC_IMUL:
		free_imul_unit++;
		break;

	case MC_DIV:
		free_div_unit++;
		break;

	case MC_SAVE_RET_VALUE:
		break;

	default:
		ERR("%d \n", stamp);
		break;
	}

	assert(free_alu_unit <= alu_unit);
	assert(free_imul_unit <= imul_unit);
	assert(free_div_unit <= div_unit);
}

int mc_select(vector<X64mc> &x64mc)
{
	for (auto it = ready.end(); it != ready.begin();)
	{
		it--;
		int mc = it->second;
		if (get_function_unit(x64mc[mc].mc_stamp))
		{
			ready.erase(it);
			return mc;
		}
	}
	return -1;
}

void init_ready_queue(vector<X64mc> &x64mc)
{
	for (int i = 0; i < mcs_pred.size(); i++)
	{
		if (mcs_pred[i].edges == 0)
			ready.insert(
			    { x64mc[i].chain_latency, i });
	}
}
void finish_mc__update_ready_queue(vector<X64mc> &x64mc, int mc)
{
	auto &v = mcs_succ[mc].mcs;
	for (auto mc : v)
	{
		mcs_pred[mc].edges--;
		assert(mcs_pred[mc].edges >= 0);

		if (mcs_pred[mc].edges == 0)
			ready.insert(
			    { x64mc[mc].chain_latency, mc });
	}
}

static void mc_schdu(BasicBlock &bb)
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
	 *				if x64mc_scheduled.size == x64mc.size
	 *					break
	 *
	 *		for mc : running
	 *			if mc.start_time + mc.latency >= cycle
	 *				rm mc in running
	 *				mv mc->edges[i] to ready if dep[i].deps == 0
	 */

	//	LOG("scp %s", scp->name.c_str());
	vector<X64mc> &x64mc = bb.x64mc;
	vector<X64mc> &x64mc_schedu = bb.x64mc_schedu;

	int cycle = 0;
	init_ready_queue(x64mc);

	if (ready.size() == 0)
	{

		if (bb.entry_label)
			LOG("bb %s == 0", bb.entry_label->name.c_str());
		if (x64mc.size() || bb.tacs.size())
			ERR();
	}

	while (!running.empty() || !ready.empty())
	{
		for (auto it = running.begin(); it != running.end();)
		{
			int mc = *it;
			assert(x64mc[mc].start_cycle >= 0);
			assert(x64mc[mc].latency > 0);

			if (cycle >= x64mc[mc].start_cycle + x64mc[mc].latency)
			{
				free_function_unit(x64mc[mc].mc_stamp);
				finish_mc__update_ready_queue(x64mc, mc);

				it = running.erase(it);
				continue;
			}
			it++;
		}

		// select from ready[]
		while (1)
		{
			int mc = mc_select(x64mc);
			if (mc < 0)
				break;

			x64mc[mc].start_cycle = cycle;
			x64mc_schedu.push_back(x64mc[mc]);
			running.push_back(mc);

			assert(x64mc[mc].start_cycle >= 0);
			if (x64mc[mc].latency <= 0)
			{
				ERR("%d, %s", x64mc[mc].mc_stamp, x64mc[mc].asm_code.c_str());
			}
		}

//		if (x64mc_scheduled.size() == x64mc.size())
//			break;
		cycle++;
	}
//	printf("max cycle: %d \n", cycle);

//	auto &t = *x64mc_schedu.rbegin();
//	if (t.mc_stamp != MC_RET)
//	{
//		LOG("x64mc_scheduled last: %s %s, s1 %%%d, s2 %%%d, cycle: %d-%d\n",
//		    t.asm_code.c_str(), t.ori_sem.c_str(),
//		    t.s1, t.s2,
//		    t.start_cycle, t.chain_latency);
//	}
}
static void dump()
{
	printf("\n========== mc schedu ==========\n");

	for (BasicBlock &bb : basic_blocks)
		dump_mc(bb, bb.x64mc_schedu);
}

void mc_schedule()
{
	prev_write_mc.resize(vrm.id + 1, -1);
	prev_read_mc.resize(vrm.id + 1, -1);

	for (BasicBlock &bb : basic_blocks)
	{
		if (!bb.x64mc.size())
			continue;

		gen_use_def_chain(bb.x64mc);
		dump_chain();
		gen_schdu_chain_latency(bb.x64mc);

		mc_schdu(bb);
	}

	dump();
}
