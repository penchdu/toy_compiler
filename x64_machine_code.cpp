/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

vector<McDepend> mcs_pred;
vector<McDepend> mcs_succ;

const McInfo mc_info[MC_INVALID + 1] = {
    [MC_LI] = { 1, "mov" },
    [MC_LD] = { 3, "mov" },
    [MC_ST] = { 3, "mov" },

    [MC_ASSIGN] = { 1, "mov" },
    [MC_ADD] = { 1, "add" },
    [MC_SUB] = { 1, "sub" },
    [MC_IMUL] = { 3, "imul" },
    [MC_DIV] = { 10, "div" },

    [MC_LOGIC_AND] = { 1, "and" },

    [MC_CMP] = { 1, "cmp" },

    [MC_SET_E] = { 1, "sete" },
    [MC_SET_NE] = { 1, "setne" },
    [MC_SET_L] = { 1, "setl" },
    [MC_SET_LE] = { 1, "setle" },
    [MC_SET_G] = { 1, "setg" },
    [MC_SET_GE] = { 1, "setge" },

    [MC_JMP] = { 0, "jmp" },
    [MC_JE] = { 0, "je" },
    [MC_JNE] = { 0, "jne" },
    [MC_JL] = { 0, "jl" },
    [MC_JLE] = { 0, "jle" },
    [MC_JG] = { 0, "jg" },
    [MC_JGE] = { 0, "jge" },

    [MC_RET] = { 0, "ret" },
};

struct Op2mc {
	enum SemOperator op;
	MachineCodeStamp mc;
	char *mc_code = 0;
};
Op2mc op2mc[] = {
    { OP_ASSIGN, MC_ASSIGN },
    { OP_ADD, MC_ADD },
    { OP_SUB, MC_SUB },
    { OP_MUL, MC_IMUL },
    { OP_DIV, MC_DIV },

    { OP_CMP_E, MC_SET_E },
    { OP_CMP_NE, MC_SET_NE },
    { OP_CMP_L, MC_SET_L },
    { OP_CMP_LE, MC_SET_LE },
    { OP_CMP_G, MC_SET_G },
    { OP_CMP_GE, MC_SET_GE },

    { OP_LOGIC_AND, MC_LOGIC_AND },
    { OP_LOGIC_OR, MC_LOGIC_OR },
    { OP_ALL, MC_INVALID },
};
MachineCodeStamp op2jmp_mc[] = {
    [OP_CMP_E] = MC_JE,
    [OP_CMP_NE] = MC_JNE,
    [OP_CMP_L] = MC_JL,
    [OP_CMP_LE] = MC_JLE,
    [OP_CMP_G] = MC_JG,
    [OP_CMP_GE] = MC_JGE,
};
static void gen_op_add_sub_mul_div_mc(vector<X64mc> &x64mc, MachineCodeStamp mc, const string &ori_sem,
    int tac_dst, int tac_s1, int tac_s2)
{
	X64mc inst;

	inst = X64mc(MC_ASSIGN, tac_dst, tac_s1);
	inst.ori_sem = ori_sem;
	x64mc.push_back(inst);

	inst = X64mc(mc, tac_dst, tac_s2);
	inst.ori_sem = ori_sem;
	x64mc.push_back(inst);
}
static void gen_op_cmp_mc(vector<X64mc> &x64mc, MachineCodeStamp set_mc, const string &ori_sem,
    int tac_dst, int tac_s1, int tac_s2)
{
	X64mc inst;

	inst = X64mc(MC_CMP, tac_s1, tac_s2);
	inst.ori_sem = ori_sem;
	x64mc.push_back(inst);

	inst = X64mc(set_mc, tac_dst);
	inst.ori_sem = ori_sem;
	x64mc.push_back(inst);
}
static void gen_op_mc(vector<X64mc> &x64mc, Tac &tac)
{
	SemOperator op = tac.ast->op;
	MachineCodeStamp mc_stamp;
	X64mc mc;

	switch (op)
	{
	case OP_ASSIGN:
		mc = X64mc(MC_ASSIGN, tac.dst, tac.s1);
		mc.ori_sem = "assign";
		x64mc.push_back(mc);
		break;

	case OP_CMP_E:
		case OP_CMP_NE:
		case OP_CMP_L:
		case OP_CMP_LE:
		case OP_CMP_G:
		case OP_CMP_GE:
		mc_stamp = op2mc[op].mc;
		gen_op_cmp_mc(x64mc, mc_stamp, mc_info[mc_stamp].mc_code, tac.dst, tac.s1, tac.s2);
		break;

	case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
		mc_stamp = op2mc[op].mc;
		gen_op_add_sub_mul_div_mc(x64mc, mc_stamp, mc_info[mc_stamp].mc_code, tac.dst, tac.s1, tac.s2);
		break;

	default:
		ERR();
	}
}
static void gen_mc()
{
	X64mc mc;

	for (BasicBlock &bb : basic_blocks)
	{
		for (Tac &tac : bb.tacs)
		{
			Semantic sem = tac.ast->sem;
			if (sem == SEM_OPERATOR)
			{
				gen_op_mc(bb.x64mc, tac);
				continue;
			}

			switch (sem)
			{
			case SEM_CONST_NUM:
				mc = X64mc(MC_LI, tac.dst);
				mc.const_num = tac.const_num_value;
				mc.ori_sem = "li";
				bb.x64mc.push_back(mc);
				break;

			case SEM_RETURN:
				mc = X64mc(MC_RET, tac.s1);
				mc.ori_sem = "ret";
				bb.x64mc.push_back(mc);
				break;

				// todo
			case SEM_FUNC_CALL:
				ERR("todo sem_func* semty %d \n", sem);
				break;

			case SEM_OPERATOR:
				default:
				ERR("%d \n", sem);
				break;
			}
		}

		if (bb.exit_jmp)
		{
			if (bb.exit_jmp->sem == SEM_COND_JMP)
			{
				Ast *cond = bb.exit_jmp->asts[0];
				bb.mc_jmp = op2jmp_mc[cond->op];
			}
			else if (bb.exit_jmp->sem == SEM_JMP)
			{
				bb.mc_jmp = MC_JMP;
			}
		}
	}
}
void dump_mc(vector<X64mc> &v)
{
	for (auto &mc : v)
	{
		MachineCodeStamp mc_stamp = mc.mc_stamp;

		switch (mc_stamp)
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
			printf("%s dword ptr [%d], %%%d ", mc.asm_code.c_str(), mc.of1, mc.s1);
			PRINT_MORE
			break;

		case MC_ASSIGN:
			case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			printf("%s %%%d, %%%d ", mc.asm_code.c_str(), mc.s1, mc.s2);
			PRINT_MORE
			break;

		case MC_CMP:
			printf("%s %%%d, %%%d ", mc.asm_code.c_str(), mc.s1, mc.s2);
			PRINT_MORE
			break;

		case MC_SET_E:
			case MC_SET_NE:
			case MC_SET_L:
			case MC_SET_LE:
			case MC_SET_G:
			case MC_SET_GE:
			printf("%s %%%d ", mc.asm_code.c_str(), mc.s1);
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
			ERR("%d \n", mc_stamp);
			break;
		}
	}
}
static void dump()
{
	printf("\n========== mc ==========\n");
	printf("push rbp\n");
	printf("mov rbp, rsp\n");
	printf("sub rsp, %d\n\n", vrm.offset);

	for (BasicBlock &bb : basic_blocks)
	{
		if (bb.entry_label != 0)
			printf("\nlabel %s:\n", bb.entry_label->name.c_str());

		dump_mc(bb.x64mc);

		if (bb.exit_jmp != 0)
		{
			printf("%s %s:\n\n", mc_info[bb.mc_jmp].mc_code.c_str(),
			    bb.exit_jmp->jmp_out->name.c_str());
		}
	}
	printf("\nmov rsp, rbp \n");
	printf("pop rbp \n");
	printf("ret \n\n");
}
void gen_machine_code()
{
	gen_mc();
	dump();
}
