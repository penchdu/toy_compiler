/*
 * x64_inst_schdule.cpp
 *
 *  Created on: 2026年9月13日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

extern vector<int> prev_write;
extern vector<vector<int>> prev_read;
extern vector<McDepend> mcs_predecessor;
extern vector<McDepend> mcs_successor;


using std::multimap;
multimap<int, int> ready;
vector<int> running;

const int reg_limit = X64PR_MAX;
const int alu_unit = 1;
const int imul_unit = 1;
const int div_unit = 1;

int free_reg = reg_limit;
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

	case MC_SAVE_RET:
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

	case MC_SAVE_RET:
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
	for (int i = 0; i < mcs_predecessor.size(); i++)
	{
		if (mcs_predecessor[i].edges == 0)
			ready.insert(
			    {x64mc[i].chain_latency, i});
	}
}
void finish_mc__update_ready_queue(vector<X64mc> &x64mc, int mc)
{
	auto &v = mcs_successor[mc].mcs;
	for (auto mc : v)
	{
		mcs_predecessor[mc].edges--;
		assert(mcs_predecessor[mc].edges >= 0);

		if (mcs_predecessor[mc].edges == 0)
			ready.insert(
			    {x64mc[mc].chain_latency, mc});
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

			for (int vr : x64mc[mc].vr)
			{
				if (vr >= 0)
					bb.vrids.push_back(vr);
			}

			running.push_back(mc);

			assert(x64mc[mc].start_cycle >= 0);
			if (x64mc[mc].latency <= 0)
			{
				ERR("%d, %s", x64mc[mc].mc_stamp, x64mc[mc].asm_code.c_str());
			}
		}
		cycle++;
	}

	if (bb.x64mc_schedu.size() != bb.x64mc.size())
		ERR("%lu %lu", bb.x64mc.size(), bb.x64mc_schedu.size());
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
	prev_write.resize(vr_manager.id + 1, -1);
	prev_read.resize(vr_manager.id + 1);

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
