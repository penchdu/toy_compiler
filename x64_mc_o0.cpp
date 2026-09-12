/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"
#include "x64_mc.h"

vector<X64_mc> x64mc;
static void _gen_machine_code()
{
	X64_mc inst;
	// push rbp
	// mov rbp, rsp
	// sub rsp, <num>

	for(auto &tac : three_addr_code)
	{
		Semantic_type ty = tac->ast->semty;

		switch(ty)
		{
		// for const_num, mc.s2 == mc.const_num
		case sem_const_num:
			inst = X64_mc(mc_li, tac->dst, tac->const_num_value);
			inst.ori_sem = "li";
			inst.const_num = tac->const_num_value;
			x64mc.push_back(inst);
			break;

		case op_assign:
			inst = X64_mc(mc_mov, tac->dst, tac->s1);
			inst.ori_sem = "assign";
			x64mc.push_back(inst);
			break;

		case op_add:
			inst = X64_mc(mc_mov, tac->dst, tac->s1);
			inst.ori_sem = "add";
			x64mc.push_back(inst);

			inst = X64_mc(mc_add, tac->dst, tac->s2);
			inst.ori_sem = "add";
			x64mc.push_back(inst);
			break;

		case op_sub:
			inst = X64_mc(mc_mov, tac->dst, tac->s1);
			inst.ori_sem = "sub";
			x64mc.push_back(inst);

			inst = X64_mc(mc_sub, tac->dst, tac->s2);
			inst.ori_sem = "sub";
			x64mc.push_back(inst);
			break;

		case op_mul:
			inst = X64_mc(mc_mov, tac->dst, tac->s1);
			inst.ori_sem = "imul";
			x64mc.push_back(inst);

			inst = X64_mc(mc_imul, tac->dst, tac->s2);
			inst.ori_sem = "imul";
			x64mc.push_back(inst);
			break;

		case op_div:
			inst = X64_mc(mc_mov, tac->dst, tac->s1);
			inst.ori_sem = "div";
			x64mc.push_back(inst);

			inst = X64_mc(mc_div, tac->dst, tac->s2);
			inst.ori_sem = "div";
			x64mc.push_back(inst);
			break;

		case sem_return:
			inst = X64_mc(mc_ret, tac->s1);
			inst.ori_sem = "ret";
			x64mc.push_back(inst);
			break;

			// todo
		case sem_func_call:
			ERR("todo sem_func* semty %d \n", ty);

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

	for(auto &mc : x64mc)
	{
		Machine_code_type ty = mc.mcty;

		switch(ty)
		{
		case mc_li:
//			inst = X64_mc(mc_ld_const, r->dst, r->const_num_value);
			printf("%s %%%d, num %d ", mc_info[ty].mc_code.c_str(), mc.s1, mc.s2);
			printf("\t%s: %d %d\n", mc.ori_sem.c_str(), mc.of1, mc.of2);
			break;

		case mc_mov:
		case mc_add:
		case mc_sub:
		case mc_imul:
			printf("%s %%%d, %%%d ", mc_info[ty].mc_code.c_str(), mc.s1, mc.s2);
			printf("\t%s: %d %d\n", mc.ori_sem.c_str(), mc.of1, mc.of2);
			break;

//		case mc_mov:
//			inst = X64_mc(mc_mov, r->dst, r->src1);
//		case mc_add:
//			inst = X64_mc(mc_mov, r->dst, r->src1);
//			inst = X64_mc(mc_add, r->dst, r->src2);
//		case mc_sub:
//			inst = X64_mc(mc_mov, r->dst, r->src1);
//			inst = X64_mc(mc_sub, r->dst, r->src2);
//		case mc_imul:
//			inst = X64_mc(mc_mov, r->dst, r->src1);
//			inst = X64_mc(mc_imul, r->dst, r->src2);

		case mc_div:
//		case mc_div_mov_eax_s1_and_cdq:
//		case mc_div_mov_s1_eax
//			inst = X64_mc(mc_div_ld, r->src1);
			printf("mov eax, %%%d \t%s\n", mc.s1, mc.ori_sem.c_str());
			printf("cdq \n");
			printf("idiv %%%d \n", mc.s2);
			printf("mov %%%d, eax \n", mc.s1);
			break;

		case mc_ret:
//			inst = X64_mc(mc_ret, r->src1);
			printf("mov eax, %%%d \t%s\n", mc.s1, mc.ori_sem.c_str());
			break;

		default:
			ERR("%d \n", ty);
			break;
		}
	}

	printf("\nmov rsp, rbp \n");
	printf("pop rbp \n");
	printf("ret \n");
}

void gen_machine_code()
{
	_gen_machine_code();
	dump_mc();
}
