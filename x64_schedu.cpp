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

struct ScheduleScore {
	int idx_in_mc_list = -1;
	int critical_path = 0;

	bool can_emit = 0;

//	int left_use = -1;
	int score = 0;
//	int vr = -1;
};

using std::multimap;
vector<ScheduleScore> ready;
vector<int> running;

const int reg_limit = X64PR_MAX;
const int alu_unit = 2;
const int imul_unit = 1;
const int div_unit = 1;

int free_reg = reg_limit;
int free_alu_unit = alu_unit;
int free_imul_unit = imul_unit;
int free_div_unit = div_unit;

bool has_function_unit(MachineCodeStamp stamp)
{
	switch (stamp)
	{
	case MC_LI:
		case MC_ASSIGN:
		case MC_SAVE_RET:
		return true;

	case MC_ADD:
		case MC_SUB:
		case MC_CMP_E:
		case MC_CMP_NE:
		case MC_CMP_L:
		case MC_CMP_LE:
		case MC_CMP_G:
		case MC_CMP_GE:
		return free_alu_unit > 0;

	case MC_IMUL:
		return free_imul_unit > 0;

	case MC_DIV:
		return free_div_unit > 0;

	default:
		ERR("%d \n", stamp);
		break;
	}
	return false;
}
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
	int max_critical_path = 0;
	int can_emit_idx_in_ready = -1;
	vector<ScheduleScore> can_emit;
	for (int i = 0; i < ready.size(); i++)
	{
		int idx_in_mc_list = ready[i].idx_in_mc_list;
		bool b = has_function_unit(x64mc[idx_in_mc_list].mc_stamp);
		if (b)
		{
			can_emit.push_back(ready[i]);
			can_emit_idx_in_ready = i;
			max_critical_path = std::max(max_critical_path, ready[i].critical_path);
		}
	}

	if (can_emit.size() == 1)
	{
		int idx_in_mc_list = ready[can_emit_idx_in_ready].idx_in_mc_list;
		bool r = get_function_unit(x64mc[idx_in_mc_list].mc_stamp);
		assert(r);
		ready.erase(ready.begin() + can_emit_idx_in_ready);
		return can_emit[0].idx_in_mc_list;
	}
	if (can_emit.size() == 0)
		return -1;

	assert(max_critical_path > 0);
//	std::sort(can_emit.begin(), can_emit.end(),
//	    [](const ScheduleScore &a, const ScheduleScore &b)
//	        {
//		        return a.critical_path < b.critical_path;
//	        });
	for (int i = 0; i < can_emit.size(); i++)
	{
		int critical_path_score = ((float)can_emit[i].critical_path / max_critical_path) * can_emit.size();
		can_emit[i].score += critical_path_score;
	}

	int transient_score = can_emit.size() + 1;

	for (auto &r : can_emit)
	{
		int s1 = x64mc[r.idx_in_mc_list].s1;
		if (s1 >= 0)
		{
			Ast *ps1 = vr_manager.ast[s1];
			Scope *scp = ps1->this_scp;
			if (ps1->symb_stamp == SYMB_PRIVATE_transient)
				r.score += transient_score;

			assert(scp->used_cnt_in_bb[s1] > 0);
			int consumed_ratio_score = ((float)scp->consume_cnt_in_bb[s1] / scp->used_cnt_in_bb[s1]) * can_emit.size();
			r.score += consumed_ratio_score;
		}

		int s2 = x64mc[r.idx_in_mc_list].s2;
		if (s2 >= 0)
		{
			Ast *ps2 = vr_manager.ast[s2];
			Scope *scp = ps2->this_scp;
			if (ps2->symb_stamp == SYMB_PRIVATE_transient)
				r.score += transient_score;

			assert(scp->used_cnt_in_bb[s2] > 0);
			int consumed_ratio_score = ((float)scp->consume_cnt_in_bb[s2] / scp->used_cnt_in_bb[s2]) * can_emit.size();
			r.score += consumed_ratio_score;
		}
	}

	std::sort(can_emit.begin(), can_emit.end(),
	    [](const ScheduleScore &a, const ScheduleScore &b)
	        {
		        return a.score > b.score;
	        });

	int ret = can_emit[0].idx_in_mc_list;
	can_emit.erase(can_emit.begin());

	for (auto it = ready.begin(); it != ready.end(); ++it)
	{
		if (it->idx_in_mc_list == ret)
		{
			bool r = get_function_unit(x64mc[ret].mc_stamp);
			assert(r);
			ready.erase(it);
			break;
		}
	}

	return ret;
}

void init_ready_queue(vector<X64mc> &x64mc)
{
	for (int i = 0; i < mcs_predecessor.size(); i++)
	{
		if (mcs_predecessor[i].edges == 0)
			ready.push_back(
			    {i, x64mc[i].chain_latency});
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
			ready.push_back(
			    {mc, x64mc[mc].chain_latency});
	}
}
static void update_bb_consume_cnt(Scope *scp, X64mc &mc)
{
	MachineCodeStamp mc_stamp = mc.mc_stamp;

	switch (mc_stamp)
	{
	case MC_LI:
		break;

	case MC_LD:
		break;

	case MC_ST:
		scp->consume_cnt_in_bb[mc.s1]++;
		break;

	case MC_ASSIGN:
		case MC_ADD:
		case MC_SUB:
		case MC_IMUL:
		case MC_DIV:
		scp->consume_cnt_in_bb[mc.s1]++;
		scp->consume_cnt_in_bb[mc.s2]++;
		break;

	case MC_CMP_E:
		case MC_CMP_NE:
		case MC_CMP_L:
		case MC_CMP_LE:
		case MC_CMP_G:
		case MC_CMP_GE:
		scp->consume_cnt_in_bb[mc.s1]++;
		scp->consume_cnt_in_bb[mc.s2]++;
		break;

	case MC_SAVE_RET:
		scp->consume_cnt_in_bb[mc.s1]++;
		break;

	default:
		ERR("%d \n", mc_stamp);
		break;
	}
}

static void mc_schdu(Scope *scp)
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

	LOG("scp %s", scp->name.c_str());
	BasicBlock *bb = (BasicBlock*) scp->basic_block;
	vector<X64mc> &x64mc = bb->x64mc;
	vector<X64mc> &x64mc_schedu = bb->x64mc_schedu;

	scp->consume_cnt_in_bb.resize(vr_manager.id + 1);

	int cycle = 0;
	init_ready_queue(x64mc);

	if (ready.size() == 0)
	{
		if (bb->entry_label)
			LOG("bb %s == 0", bb->entry_label->name.c_str());
		if (x64mc.size() || scp->tac.size())
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
				update_bb_consume_cnt(scp, x64mc[mc]);
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

			for (int vr : x64mc[mc].vr)
			{
				if (vr >= 0)
					bb->vrids.push_back(vr);
			}

			assert(x64mc[mc].start_cycle >= 0);
			if (x64mc[mc].latency <= 0)
			{
				ERR("%d, %s", x64mc[mc].mc_stamp, x64mc[mc].asm_code.c_str());
			}
		}
		cycle++;
	}

	if (bb->x64mc_schedu.size() != bb->x64mc.size())
		ERR("%lu %lu", bb->x64mc.size(), bb->x64mc_schedu.size());
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

static void _mc_schedule(Scope *scp)
{
	BasicBlock *bb = (BasicBlock*) scp->basic_block;

	if (bb->x64mc.size())
	{
		gen_use_def_chain(bb->x64mc);
		dump_chain();
		gen_schdu_chain_latency(bb->x64mc);

		mc_schdu(scp);
	}

	for (Scope *p : scp->clds)
		_mc_schedule(p);
}
void mc_schedule()
{
	prev_write.resize(vr_manager.id + 1, -1);
	prev_read.resize(vr_manager.id + 1);

	_mc_schedule(&file_scp);

	mc_list_name = "x64mc_schedu";
	printf("\n========== mc schedu ==========\n");
	dump_mc(&file_scp);
}
