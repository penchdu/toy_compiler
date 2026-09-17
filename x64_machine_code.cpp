/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"
#include "x64_back_end.h"

extern vector<ThreeAddrCode*> three_addr_code;

vector<X64mc> x64mc;
vector<X64mc> x64mc_schedued;
vector<McDepend> mcs_pred;
vector<McDepend> mcs_succ;

static void gen__add_sub_mul_div_mc(MachineCodeType mc, string ori_sem, int tac_dst, int tac_s1, int tac_s2)
{
	X64mc inst;

	inst = X64mc(MC_ASSIGN, tac_dst, tac_s1);
	inst.ori_sem = ori_sem;
	x64mc.push_back(inst);

	inst = X64mc(mc, tac_dst, tac_s2);
	inst.ori_sem = ori_sem;
	x64mc.push_back(inst);
}
static void gen_mc()
{
	X64mc inst;
	for (auto &tac : three_addr_code)
	{
		SemanticType ty = tac->ast->semty;

		switch (ty)
		{
		case SEM_CONST_NUM:
			inst = X64mc(MC_LI, tac->dst);
			inst.const_num = tac->const_num_value;
			inst.ori_sem = "li";
			x64mc.push_back(inst);
			break;

		case OP_ASSIGN:
			inst = X64mc(MC_ASSIGN, tac->dst, tac->s1);
			inst.ori_sem = "assign";
			x64mc.push_back(inst);
			break;

		case OP_ADD:
			gen__add_sub_mul_div_mc(MC_ADD, "add", tac->dst, tac->s1, tac->s2);
			break;

		case OP_SUB:
			gen__add_sub_mul_div_mc(MC_SUB, "sub", tac->dst, tac->s1, tac->s2);
			break;

		case OP_MUL:
			gen__add_sub_mul_div_mc(MC_IMUL, "imul", tac->dst, tac->s1, tac->s2);
			break;

		case OP_DIV:
			gen__add_sub_mul_div_mc(MC_DIV, "div", tac->dst, tac->s1, tac->s2);
			break;

		case SEM_RETURN:
			inst = X64mc(MC_RET, tac->s1);
			inst.ori_sem = "ret";
			x64mc.push_back(inst);
			break;

			// todo
		case SEM_FUNC_CALL:
			ERR("todo sem_func* semty %d \n", ty);
			break;

		default:
			ERR("%d \n", ty);
			break;
		}
	}
}
void dump_mc(vector<X64mc> &v)
{
	printf("\n========== mc ==========\n");

	// push rbp
	// mov rbp, rsp
	// sub rsp, <num>
	printf("push rbp\n");
	printf("mov rbp, rsp\n");
	printf("sub rsp, %d\n\n", vrm.offset);

	for (auto &mc : v)
	{
		MachineCodeType ty = mc.mcty;

		switch (ty)
		{
		case MC_LI:
			printf("%s %%%d, num %d ", mc.asm_code.c_str(), mc.s1, mc.const_num);
			PRINT_MORE
			break;

		case MC_LD:
			printf("%s %%%d, dword ptr [%d] ", mc.asm_code.c_str(), mc.s1, mc.of1);
			PRINT_MORE
			break;

		case MC_ST:
			printf("%s dword ptr [%d], %%%d ", mc.asm_code.c_str(), mc.of1 ,mc.s1);
			PRINT_MORE
			break;

		case MC_ASSIGN:
			case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			printf("%s %%%d, %%%d ", mc.asm_code.c_str(), mc.s1, mc.s2);
			PRINT_MORE
			break;

		case MC_DIV:
			printf("mov eax, %%%d", mc.s1);
			PRINT_MORE

			printf("cdq \n");
			printf("idiv %%%d \t div \n", mc.s2);
			printf("mov %%%d, eax \t div \n", mc.s1);
			break;

		case MC_RET:
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

void gen_machine_code()
{
	gen_mc();
//	dump_mc(x64mc);
}
