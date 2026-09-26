/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

const McInfo mc_info[MC_INVALID + 1] = {
    [MC_LI] = { 1, "mov" },
    [MC_LD] = { 3, "mov" },
    [MC_ST] = { 3, "mov" },

    [MC_ASSIGN] = { 1, "mov" },
    [MC_ADD] = { 1, "add" },
    [MC_SUB] = { 1, "sub" },
    [MC_IMUL] = { 3, "imul" },
    [MC_DIV] = { 10, "div" },

    [MC_CMP] = { 1, "cmp" },

    [MC_CMP_E] = { 1, "cmpe" },
    [MC_CMP_NE] = { 1, "cmpne" },
    [MC_CMP_L] = { 1, "cmpl" },
    [MC_CMP_LE] = { 1, "cmple" },
    [MC_CMP_G] = { 1, "cmpg" },
    [MC_CMP_GE] = { 1, "cmpge" },

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

    [MC_SAVE_RET_VALUE] = { 1, "save_ret" },
};

MachineCodeStamp op_to_mc[] = {
    [OP_ASSIGN] = MC_ASSIGN,
    [OP_ADD]= MC_ADD,
    [OP_SUB] = MC_SUB,
    [OP_MUL] = MC_IMUL,
    [OP_DIV] = MC_DIV,

    [OP_CMP_E] = MC_CMP_E,
    [OP_CMP_NE] = MC_CMP_NE,
    [OP_CMP_L] = MC_CMP_L,
    [OP_CMP_LE] = MC_CMP_LE,
    [OP_CMP_G] = MC_CMP_G,
    [OP_CMP_GE] = MC_CMP_GE,

    [OP_ALL] = MC_INVALID,
};
MachineCodeStamp op2jmp_mc[] = {
    [OP_CMP_E] = MC_JE,
    [OP_CMP_NE] = MC_JNE,
    [OP_CMP_L] = MC_JL,
    [OP_CMP_LE] = MC_JLE,
    [OP_CMP_G] = MC_JG,
    [OP_CMP_GE] = MC_JGE,
};

Mc2mc fake_cmp_mc_to_real_mc[] = {
    [MC_CMP_E] = { MC_CMP, MC_SET_E },
    [MC_CMP_NE] = { MC_CMP, MC_SET_NE },
    [MC_CMP_L] = { MC_CMP, MC_SET_L },
    [MC_CMP_LE] = { MC_CMP, MC_SET_LE },
    [MC_CMP_G] = { MC_CMP, MC_SET_G },
    [MC_CMP_GE] = { MC_CMP, MC_SET_GE },
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
static void gen_op_cmp_mc(vector<X64mc> &x64mc, MachineCodeStamp mc, const string &ori_sem,
    int tac_dst, int tac_s1, int tac_s2)
{
	X64mc inst;

	inst = X64mc(mc, tac_dst, tac_s1, tac_s2);
	inst.ori_sem = ori_sem;
	x64mc.push_back(inst);
}
static void gen_op_mc(vector<X64mc> &x64mc, Tac &tac)
{
	Operator op = tac.ast->op;
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
		mc_stamp = op_to_mc[op];
		gen_op_cmp_mc(x64mc, mc_stamp, mc_info[mc_stamp].mc_code, tac.dst, tac.s1, tac.s2);
		break;

	case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
		mc_stamp = op_to_mc[op];
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
			Semantic sem = tac.ast->sem_stamp;
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

			case SEM_SAVE_RET_VALUE:
				mc = X64mc(MC_SAVE_RET_VALUE, tac.s1);
				mc.ori_sem = "save_ret";
				bb.x64mc.push_back(mc);
				break;

				// todo
			case SEM_FUNC_CALL:
				ERR("todo sem_func* semty %d \n", sem);
				break;

				default:
				ERR("%d \n", sem);
				break;
			}
		}

		if (bb.exit_jmp)
		{
			if (bb.exit_jmp->sem_stamp == SEM_COND_JMP)
			{
				Ast *cond = bb.exit_jmp->asts[0];
				bb.mc_jmp = op2jmp_mc[cond->op];
			}
			else if (bb.exit_jmp->sem_stamp == SEM_JMP)
				bb.mc_jmp = MC_JMP;
			else if (bb.exit_jmp->sem_stamp == SEM_SAVE_RET_VALUE)
			{
				ERR();
				bb.mc_jmp = MC_JMP;
			}
			else
				ERR();
		}

		if (bb.tacs.size() != bb.x64mc.size())
		{
			LOG("%lu %lu", bb.tacs.size(), bb.x64mc.size());
		}
	}
}

#define PRINT_ASM_HEAD(fmt, ...) printf(fmt "\n", ##__VA_ARGS__);
#define PRINT_ASM(fmt, ...) printf("\t" fmt "\n", ##__VA_ARGS__);

void dump_mc(BasicBlock &bb, vector<X64mc> &v)
{
	if (bb.entry_label != 0)
		PRINT_ASM_HEAD("\n%s:", bb.entry_label->name.c_str());

	for (auto &mc : v)
	{
		MachineCodeStamp mc_stamp = mc.mc_stamp;
		MachineCodeStamp set_mc;

		switch (mc_stamp)
		{
		case MC_LI:
			PRINT_ASM("%s %%%d, num %d", mc.asm_code.c_str(), mc.s1, mc.const_num);
			PRINT_MORE
			break;

		case MC_LD:
			PRINT_ASM("%s %%%d, dword ptr [%d]", mc.asm_code.c_str(), mc.s1, mc.of1);
			PRINT_MORE
			break;

		case MC_ST:
			PRINT_ASM("%s dword ptr [%d], %%%d", mc.asm_code.c_str(), mc.of1, mc.s1);
			PRINT_MORE
			break;

		case MC_ASSIGN:
			case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			PRINT_ASM("%s %%%d, %%%d", mc.asm_code.c_str(), mc.s1, mc.s2);
			PRINT_MORE
			break;

		case MC_CMP_E:
			case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:

			PRINT_ASM("%%%d = %s %%%d, %%%d", mc.dst, mc.asm_code.c_str(), mc.s1, mc.s2);

//			set_mc = fake_cmp_mc_to_real_mc[mc_stamp].mc_set;
//			PRINT_ASM("%s %%%d", mc_info[set_mc].mc_code.c_str(), mc.dst);
			PRINT_MORE
			break;

		case MC_DIV:
			PRINT_ASM("mov eax, %%%d", mc.s1);
			PRINT_MORE

			PRINT_ASM("cdq");
			PRINT_ASM("idiv %%%d \t div", mc.s2);
			PRINT_ASM("mov %%%d, eax \t div", mc.s1);
			break;

		case MC_SAVE_RET_VALUE:
			PRINT_ASM("mov eax, %%%d", mc.s1);
			PRINT_MORE
			break;

		default:
			ERR("%d \n", mc_stamp);
			break;
		}
	}

	if (bb.exit_jmp != 0)
	{
		PRINT_ASM("%s %s", mc_info[bb.mc_jmp].mc_code.c_str(),
			bb.exit_jmp->jmp_out->name.c_str());
	}
}
static void dump()
{
	PRINT_ASM_HEAD("========== mc ==========");

	for (BasicBlock &bb : basic_blocks)
		dump_mc(bb, bb.x64mc);
}
#undef PRINT_ASM_HEAD
#undef PRINT_ASM

void gen_machine_code()
{
	gen_mc();
//	dump();
}
