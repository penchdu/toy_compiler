/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"
#include "x64_back_end.h"

extern vector<Three_addr_code*> three_addr_code;

vector<X64_mc> x64mc;
vector<X64_mc> x64mc_schedued;
vector<Mc_dep> mcs_pred;
vector<Mc_dep> mcs_succ;

static void gen__add_sub_mul_div_mc(Machine_code_type mc, string ori_sem, int tac_dst, int tac_s1, int tac_s2)
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
			gen__add_sub_mul_div_mc(mc_add, "add", tac->dst, tac->s1, tac->s2);
			break;

		case op_sub:
			gen__add_sub_mul_div_mc(mc_sub, "sub", tac->dst, tac->s1, tac->s2);
			break;

		case op_mul:
			gen__add_sub_mul_div_mc(mc_imul, "imul", tac->dst, tac->s1, tac->s2);
			break;

		case op_div:
			gen__add_sub_mul_div_mc(mc_div, "div", tac->dst, tac->s1, tac->s2);
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
void dump_mc(vector<X64_mc> &v)
{
	printf("\n========== mc ==========\n");

	// push rbp
	// mov rbp, rsp
	// sub rsp, <num>
	printf("push rbp\n");
	printf("mov rbp, rsp\n");
	printf("sub rsp, %d\n\n", vreg.offset);

	for (auto &mc : v)
	{
		Machine_code_type ty = mc.mcty;

		switch (ty)
		{
		case mc_li:
			printf("%s %%%d, num %d ", mc.asm_code.c_str(), mc.s1, mc.const_num);
			PRINT_MORE
			break;

		case mc_ld:
			printf("%s %%%d, dword ptr [%d] ", mc.asm_code.c_str(), mc.s1, mc.of1);
			PRINT_MORE
			break;

		case mc_st:
			printf("%s dword ptr [%d], %%%d ", mc.asm_code.c_str(), mc.of1 ,mc.s1);
			PRINT_MORE
			break;

		case mc_assign:
			case mc_add:
			case mc_sub:
			case mc_imul:
			printf("%s %%%d, %%%d ", mc.asm_code.c_str(), mc.s1, mc.s2);
			PRINT_MORE
			break;

		case mc_div:
			printf("mov eax, %%%d", mc.s1);
			PRINT_MORE

			printf("cdq \n");
			printf("idiv %%%d \t div \n", mc.s2);
			printf("mov %%%d, eax \t div \n", mc.s1);
			break;

		case mc_ret:
			printf("mov eax, %%%d", mc.s1);
			PRINT_MORE
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

void gen_mc()
{
	_gen_machine_code();
//	dump_mc(x64mc);
}
