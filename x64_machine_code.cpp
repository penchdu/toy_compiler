/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"
#include "x64_mc.h"

vector<X64_mc> x64mc;
vector<Mc_dep> mc_dep;
vector<Mc_dep> schedule_train;

static void gen_add_sub_mul_div_mc(Machine_code_type mc, string ori_sem, int tac_dst, int tac_s1, int tac_s2)
{
	X64_mc inst;

	inst = X64_mc(mc_assign, tac_dst, tac_s1);
	inst.ori_sem = ori_sem;
	x64mc.push_back(inst);

	inst = X64_mc(mc, tac_dst, tac_s2);
	inst.ori_sem = ori_sem;
	x64mc.push_back(inst);
}
static void _gen_machine_code()
{
	X64_mc inst;
	for (auto &tac : three_addr_code)
	{
		Semantic_type ty = tac->ast->semty;

		switch (ty)
		{
		// for const_num, mc.s2 == mc.const_num
		case sem_const_num:
			inst = X64_mc(mc_li, tac->dst);
			inst.const_num = tac->const_num_value;
			inst.ori_sem = "li";
			x64mc.push_back(inst);
			break;

		case op_assign:
			inst = X64_mc(mc_assign, tac->dst, tac->s1);
			inst.ori_sem = "assign";
			x64mc.push_back(inst);
			break;

		case op_add:
			gen_add_sub_mul_div_mc(mc_add, "add", tac->dst, tac->s1, tac->s2);
			break;

		case op_sub:
			gen_add_sub_mul_div_mc(mc_sub, "sub", tac->dst, tac->s1, tac->s2);
			break;

		case op_mul:
			gen_add_sub_mul_div_mc(mc_imul, "imul", tac->dst, tac->s1, tac->s2);
			break;

		case op_div:
			gen_add_sub_mul_div_mc(mc_div, "div", tac->dst, tac->s1, tac->s2);
			break;

		case sem_return:
			inst = X64_mc(mc_ret, tac->s1);
			inst.ori_sem = "ret";
			x64mc.push_back(inst);
			break;

			// todo
		case sem_func_call:
			ERR("todo sem_func* semty %d \n", ty);
			break;

		default:
			ERR("%d \n", ty);
			break;
		}
	}
}
static void dump_mc()
{
	printf("\n========== mc ==========\n");

	X64_mc inst;

	// push rbp
	// mov rbp, rsp
	// sub rsp, <num>
	printf("push rbp\n");
	printf("mov rbp, rsp\n");
	printf("sub rsp, %d\n\n", vreg.offset);

	for (auto &mc : x64mc)
	{
		Machine_code_type ty = mc.mcty;

		switch (ty)
		{
		case mc_li:
			printf("%s %%%d, num %d ", mc_info[ty].mc_code.c_str(),
					mc.s1, mc.const_num);
			printf("\t%s, %d\n", mc.ori_sem.c_str(), mc.of1);
			break;

		case mc_assign:
		case mc_add:
		case mc_sub:
		case mc_imul:
			printf("%s %%%d, %%%d ", mc_info[ty].mc_code.c_str(), mc.s1, mc.s2);
			printf("\t%s, %d %d\n", mc.ori_sem.c_str(), mc.of1, mc.of2);
			break;

		case mc_div:
			printf("mov eax, %%%d \t %s \n", mc.s1,
					mc.ori_sem.c_str());
			printf("cdq \n");
			printf("idiv %%%d \t div \n", mc.s2);
			printf("mov %%%d, eax \t div \n", mc.s1);
			break;

		case mc_ret:
			printf("mov eax, %%%d \t%s\n", mc.s1, mc.ori_sem.c_str());
			break;

		default:
			ERR("%d \n", ty);
			break;
		}
	}

	printf("\nmov rsp, rbp \n");
	printf("pop rbp \n");
	printf("ret \n\n");
}

void gen_dependency_graph()
{
	vector<int> prev_w_mc_of_vr(vreg.id + 1, -1);
	vector<int> prev_r_mc_of_vr(vreg.id + 1, -1);
	mc_dep.resize(x64mc.size());
	schedule_train.resize(x64mc.size());

	printf("%zu, %zu\n", x64mc.size(), prev_w_mc_of_vr.size());

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
			{
				mc_dep[i].mc_idx.push_back(w1);
				schedule_train[w1].mc_idx.push_back(i);
			}
			if (r1 >= 0)
			{
				mc_dep[i].mc_idx.push_back(r1);
				schedule_train[r1].mc_idx.push_back(i);
			}
			prev_w_mc_of_vr[s1] = i;

			if (w2 >= 0)
			{
				mc_dep[i].mc_idx.push_back(w2);
				schedule_train[w2].mc_idx.push_back(i);
			}
			prev_r_mc_of_vr[s2] = i;
			break;

		case mc_add:
		case mc_sub:
		case mc_imul:
		case mc_div:
			if (w1 >= 0)
			{
				mc_dep[i].mc_idx.push_back(w1);
				schedule_train[w1].mc_idx.push_back(i);
			}
			if (r1 >= 0)
			{
				mc_dep[i].mc_idx.push_back(r1);
				schedule_train[r1].mc_idx.push_back(i);
			}
			prev_w_mc_of_vr[s1] = i;
			prev_r_mc_of_vr[s1] = i;

			if (w2 >= 0)
			{
				mc_dep[i].mc_idx.push_back(w2);
				schedule_train[w2].mc_idx.push_back(i);
			}
			prev_r_mc_of_vr[s2] = i;
			break;

		case mc_ret:
		case mc_st:
			if (w1 >= 0)
			{
				mc_dep[i].mc_idx.push_back(w1);
				schedule_train[w1].mc_idx.push_back(i);
			}
			prev_r_mc_of_vr[s1] = i;
			break;

		default:
			ERR("%d \n", ty);
			break;
		}
	}

	printf("dep\n");
	for (int i = 0; i < mc_dep.size(); i++)
	{
		for (auto r : mc_dep[i].mc_idx)
			printf("%d %d\n", i, r);
	}
	printf("\nbdep\n");
	for (int i = 0; i < schedule_train.size(); i++)
	{
		for (auto r : schedule_train[i].mc_idx)
			printf("%d %d\n", i, r);
	}

}
int get_mc_latency(int mc_id)
{
	int &c = schedule_train[mc_id].chain_latency;
	if(c >= 0)
		return c;

	int &n = schedule_train[mc_id].node_latency;
	assert(n == -1);

	Machine_code_type ty = x64mc[mc_id].mcty;
	c = n = mc_info[ty].mc_latency;

	int mx = 0;
	for(int i = 0; i < schedule_train[mc_id].mc_idx.size(); i++){
		int r = get_mc_latency(i);
		mx = std::max(mx, r);
	}

	c += mx;
	return c;
}
void gen_schedule_train_latency()
{
	for(int i = 0; i < schedule_train.size(); i++)
	{
		get_mc_latency(i);
	}
}

void gen_machine_code()
{
	_gen_machine_code();
	dump_mc();

	gen_dependency_graph();
	gen_schedule_train_latency();


}
