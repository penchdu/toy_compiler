/*
 * x64_inst_schdule.cpp
 *
 *  Created on: 2026年9月13日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

//extern vector<int> prev_write;
//extern vector<vector<int>> prev_read;
extern vector<McDepend> mcs_predecessor;
extern vector<McDepend> mcs_successor;

struct ScheduleScore {
	int idx_in_mc_list = -1;
	int critical_path = 0;

	bool can_use_alu = 0;

//	int left_use = -1;
	float score = 0;
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
		case MC_ADD:
		case MC_SUB:
		case MC_CMP_E:
		case MC_CMP_NE:
		case MC_CMP_L:
		case MC_CMP_LE:
		case MC_CMP_G:
		case MC_CMP_GE:
		case MC_SAVE_RET:

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
		case MC_ADD:
		case MC_SUB:
		case MC_CMP_E:
		case MC_CMP_NE:
		case MC_CMP_L:
		case MC_CMP_LE:
		case MC_CMP_G:
		case MC_CMP_GE:
		case MC_SAVE_RET:

		free_alu_unit++;
		break;

	case MC_IMUL:
		free_imul_unit++;
		break;

	case MC_DIV:
		free_div_unit++;
		break;

	default:
		ERR("%d \n", stamp);
		break;
	}

	assert(free_alu_unit <= alu_unit);
	assert(free_imul_unit <= imul_unit);
	assert(free_div_unit <= div_unit);
}

void print_mc_err(const char *prefix, Scope *scp, const X64mc &mc, int idx)
{
	int dst = mc.dst;
	int s1 = mc.s1;
	int s2 = mc.s2;
	MachineCodeStamp stamp = mc.mc_stamp;

	ERR("%s, scp %s, %d, %s %s(%%%d) %s(%%%d) %s(%%%d)",
	    prefix ? prefix : "",
	    scp ? scp->name.c_str() : "unknown scp", idx,
	    mc_info[stamp].mc_code.c_str(),
	    dst >= 0 ? vr_declare_manager.declare_at[dst]->tk.src.c_str() : "", dst,
	    s1 >= 0 ? vr_declare_manager.declare_at[s1]->tk.src.c_str() : "", s1,
	    s2 >= 0 ? vr_declare_manager.declare_at[s2]->tk.src.c_str() : "", s2);
}
static bool is_private_transient(Scope *scp, int vr)
{
	if (vr < 0)
		return false;

	for (Symbol *p : *scp->symb_table)
	{
		if (p->vr == vr && p->stamp == SYMB_PRIVATE_transient)
			return true;
	}
	return false;
}

static int select_pulled = 0;
int mc_select(Scope *scp, const vector<X64mc> &x64mc)
{
	int max_critical_path = 0;
	int idx_emit = -1;
	int can_emit_cnt = 0;
//	vector<ScheduleScore> can_emit;
	for (int i = 0; i < ready.size(); i++)
	{
		int k = ready[i].idx_in_mc_list;
		ready[i].can_use_alu = has_function_unit(x64mc[k].mc_stamp);
		if (ready[i].can_use_alu)
		{
			idx_emit = i;
			can_emit_cnt++;
			max_critical_path = std::max(max_critical_path, ready[i].critical_path);
		}
	}

	if (can_emit_cnt == 1)
		goto end;
	if (can_emit_cnt == 0)
		return -1;

	assert(max_critical_path > 0);

	for (int i = 0; i < ready.size(); i++)
	{
		ready[i].score = 0;
		if (!ready[i].can_use_alu || max_critical_path <= 0)
			continue;

		float critical_path_score = ((float) ready[i].critical_path / max_critical_path);
		ready[i].score += critical_path_score;
	}

	for (auto &r : ready)
	{
		if (!r.can_use_alu)
			continue;

		int mc_idx = r.idx_in_mc_list;
		MachineCodeStamp mc_stamp = x64mc[mc_idx].mc_stamp;
		float consume_transient_score = 1.0;

		if (mc_stamp != MC_LI && mc_stamp != MC_ASSIGN)
		{
			if (is_private_transient(scp, x64mc[mc_idx].s1))
				r.score += consume_transient_score;
		}

		if (is_private_transient(scp, x64mc[mc_idx].s2))
			r.score += consume_transient_score;

		float single_line_score = 1.0;
		if (mc_stamp != MC_LI && mcs_successor[mc_idx].edges == 1)
		{
			int succ = mcs_successor[mc_idx].mcs[0];
//			printf("succ %d\n", succ);
			if (mcs_predecessor[succ].edges == 1)
				r.score += single_line_score;
		}

		int ar[2] = {x64mc[mc_idx].s1, x64mc[mc_idx].s2};
		for (int vr : ar)
		{
			if (vr < 0)
				continue;

			if (scp->use_cnt_in_bb[vr] > 0)
			{
				float last_use_score = 0.5;
				if(scp->use_cnt_in_bb[vr] - scp->consume_cnt_in_bb[vr] == 1)
				r.score += last_use_score;

				float consumed_ratio_score = 0.5 * ((float) scp->consume_cnt_in_bb[vr] / scp->use_cnt_in_bb[vr]);
				r.score += consumed_ratio_score;
			}
			if (scp->use_cnt_in_bb[vr] <= 0 && scp->appear_cnt_in_bb[vr] <= 0)
				print_mc_err("select1", scp, x64mc[mc_idx], mc_idx);
		}
	}

	float mx_score;
	mx_score = -1;

	for (int i = 0; i < ready.size(); i++)
	{
		if (!ready[i].can_use_alu)
			continue;

		if (mx_score < ready[i].score)
		{
			mx_score = ready[i].score;
			idx_emit = i;
		}
	}

end:
	if (idx_emit < 0)
		return -1;

	int mc_idx = ready[idx_emit].idx_in_mc_list;
	bool r = get_function_unit(x64mc[mc_idx].mc_stamp);
	assert(r);

#if 0
	auto mc = x64mc[mc_idx];
	printf("%d: size %lu, emit %d [%s %%%d %%%d %%%d], score: ",
	    select_pulled++, ready.size(), mc_idx, mc.ori_sem.c_str(), mc.dst, mc.s1, mc.s2);
	for (auto &r : ready)
		printf("  %d=%.3f", r.idx_in_mc_list, r.score);
	printf("\n");
#endif

	ready.erase(ready.begin() + idx_emit);
	return mc_idx;
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
//void finish_mc__update_ready_queue(Scope *scp, vector<X64mc> &x64mc, int mc_finished)
//{
//	auto &mcs = mcs_successor[mc_finished].mcs;
//	for (int mc : mcs)
//	{
//		mcs_predecessor[mc].edges--;
//
//		if (mcs_predecessor[mc].edges < 0)
//			print_mc_err("update_ready_queue", scp, x64mc[mc], mc);
//
//		if (mcs_predecessor[mc].edges == 0)
//			ready.push_back({mc, x64mc[mc].chain_latency});
//	}
//}
void finish_mc__update_ready_queue2(Scope *scp, vector<X64mc> &x64mc, int mc_finished)
{
	auto &mcs = mcs_successor[mc_finished].mcs;
	for (int mc_succ : mcs)
	{
		auto &pred = mcs_predecessor[mc_succ].mcs;
		if (std::find(pred.begin(), pred.end(), mc_finished) == pred.end())
			ERR();

		int &edges = mcs_predecessor[mc_succ].edges;
		edges--;

		if (edges < 0)
		{
			auto &v = mcs_predecessor[mc_succ].mcs;
			printf("v %lu \n", v.size());
			for (auto i : v)
				printf("%d ", i);
			printf("\n");

			print_mc_err("update_ready_queue2", scp, x64mc[mc_succ], mc_succ);
		}
		if (edges == 0)
			ready.push_back({mc_succ, x64mc[mc_succ].chain_latency});
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
	 *			if cycle >= mc.start_time + mc.latency
	 *				rm mc in running
	 *				mv mc->edges[i] to ready if dep[i].deps == 0
	 */

//	LOG("scp %s", scp->name.c_str());
	BasicBlock *bb = (BasicBlock*) scp->basic_block;
	vector<X64mc> &x64mc = bb->x64mc;
	vector<X64mc> &x64mc_schedu = bb->x64mc_schedu;

	scp->consume_cnt_in_bb.clear();
	scp->consume_cnt_in_bb.resize(vr_declare_manager.size());

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
			int mc_idx = *it;
			assert(x64mc[mc_idx].start_cycle >= 0);
			assert(x64mc[mc_idx].latency > 0);

			if (cycle >= x64mc[mc_idx].start_cycle + x64mc[mc_idx].latency)
			{
				free_function_unit(x64mc[mc_idx].mc_stamp);
				update_mc_consume_cnt(scp, x64mc[mc_idx]);
				finish_mc__update_ready_queue2(scp, x64mc, mc_idx);

				it = running.erase(it);
				continue;
			}
			it++;
		}

		// select from ready[]
		while (1)
		{
			int mc = mc_select(scp, x64mc);
			if (mc < 0)
				break;

			x64mc[mc].start_cycle = cycle;
			x64mc_schedu.push_back(x64mc[mc]);
			running.push_back(mc);

			for (int vr : x64mc[mc].vr_list)
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

	if (x64mc_schedu.size() != x64mc.size())
		ERR("%lu %lu", x64mc.size(), x64mc_schedu.size());

	for (int i = 0; i < vr_declare_manager.size(); i++)
	{
		int use = scp->use_cnt_in_bb[i];
		int consume = scp->consume_cnt_in_bb[i];
		if (use != consume)
		{
			Ast *ast = vr_declare_manager.declare_at[i];
			LOG("MISMATCH vr=%d use=%d consume=%d stamp=%d scp=%s",
				i, use, consume, ast->symb_stamp, scp->name.c_str());
			ERR("%%%d, %d %d", i, use, consume);
		}
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

static void _mc_schedule(Scope *scp)
{
	select_pulled = 0;
	BasicBlock *bb = (BasicBlock*) scp->basic_block;

	if (bb->x64mc.size())
	{
		gen_use_def_chain(bb->x64mc);
//		dump_chain(bb->x64mc);
		gen_schdu_chain_latency(bb->x64mc);

		mc_schdu(scp);
	}

	for (Scope *p : scp->clds)
		_mc_schedule(p);
}
void mc_schedule()
{
	clear_vr_consume_cnt(&file_scp);

	_mc_schedule(&file_scp);

	printf("\n========== mc schedu ==========\n");
	dump_mc(&file_scp, "x64mc_schedu");
}
