/*
 * a.cpp
 *
 *  Created on: 2026�?�?�?
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"

const McInfo mc_info[MC_INVALID + 1] = {
    [MC_LI] = {1, "mov"},
    [MC_LD] = {4, "mov"},
    [MC_victim] = {0, "victim"},
    [MC_ST] = {1, "mov"},

    [MC_ASSIGN] = {1, "mov"},
    [MC_NEG] = {1, "neg"},
    [MC_ADD] = {1, "add"},
    [MC_SUB] = {1, "sub"},
    [MC_IMUL] = {3, "imul"},
    [MC_DIV] = {15, "div"},

    [MC_CMP] = {1, "cmp"},

    [MC_CMP_E] = {1, "cmpe"},
    [MC_CMP_NE] = {1, "cmpne"},
    [MC_CMP_L] = {1, "cmpl"},
    [MC_CMP_LE] = {1, "cmple"},
    [MC_CMP_G] = {1, "cmpg"},
    [MC_CMP_GE] = {1, "cmpge"},
    [MC_TEST] = {1, "test"},

    [MC_SET_E] = {1, "sete"},
    [MC_SET_NE] = {1, "setne"},
    [MC_SET_L] = {1, "setl"},
    [MC_SET_LE] = {1, "setle"},
    [MC_SET_G] = {1, "setg"},
    [MC_SET_GE] = {1, "setge"},
    [MC_SET_TEST] = {1, "sete"},

    [MC_JMP] = {0, "jmp"},
    [MC_JE] = {0, "je"},
    [MC_JNE] = {0, "jne"},
    [MC_JL] = {0, "jl"},
    [MC_JLE] = {0, "jle"},
    [MC_JG] = {0, "jg"},
    [MC_JGE] = {0, "jge"},

    [MC_SAVE_RET] = {1, "save_ret"},
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
    [OP_TEST] = MC_TEST,

    [OP_ALL] = MC_INVALID,
};
MachineCodeStamp op_to_jmp_mc[] = {
    [OP_CMP_E] = MC_JE,
    [OP_CMP_NE] = MC_JNE,
    [OP_CMP_L] = MC_JL,
    [OP_CMP_LE] = MC_JLE,
    [OP_CMP_G] = MC_JG,
    [OP_CMP_GE] = MC_JGE,
    [OP_TEST] = MC_JE,
};

Mc2mc fake_cmp_mc_to_real_mc[] = {
    [MC_CMP_E] = {MC_CMP, MC_SET_E},
    [MC_CMP_NE] = {MC_CMP, MC_SET_NE},
    [MC_CMP_L] = {MC_CMP, MC_SET_L},
    [MC_CMP_LE] = {MC_CMP, MC_SET_LE},
    [MC_CMP_G] = {MC_CMP, MC_SET_G},
    [MC_CMP_GE] = {MC_CMP, MC_SET_GE},
//    [MC_TEST] = {MC_TEST, MC_SET_TEST},
    };

static void gen_op_add_sub_mul_div_mc(vector<X64mc> &x64mc, MachineCodeStamp mc, const string &ori_sem,
    int tac_dst, int tac_s1, int tac_s2)
{
	X64mc inst;

//	inst = X64mc(MC_ASSIGN, tac_dst, tac_s1);
//	inst.ori_sem = ori_sem;
//	x64mc.push_back(inst);
//
//	inst = X64mc(mc, tac_dst, tac_s2);
//	inst.ori_sem = ori_sem;
//	x64mc.push_back(inst);

	inst = X64mc(mc, tac_dst, tac_s1, tac_s2);
	inst.ori_sem = ori_sem;
	x64mc.push_back(inst);
}
static void gen_op_cmp_mc(vector<X64mc> &x64mc, MachineCodeStamp stamp, const string &ori_sem,
    int tac_dst, int tac_s1, int tac_s2)
{
	X64mc inst;

	inst = X64mc(stamp, tac_dst, tac_s1, tac_s2);
	inst.ori_sem = ori_sem;
	x64mc.push_back(inst);
}
static void gen_op_mc(vector<X64mc> &x64mc, Tac &tac)
{
	Operator op = tac.ast->op;
	MachineCodeStamp stamp;
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
		stamp = op_to_mc[op];
		gen_op_cmp_mc(x64mc, stamp, mc_info[stamp].mc_code, tac.dst, tac.s1, tac.s2);
		break;

	case OP_TEST:
		stamp = op_to_mc[op];
		mc = X64mc(stamp, tac.s1);
		mc.ori_sem = mc_info[stamp].mc_code;
		x64mc.push_back(mc);
		break;

	case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
		stamp = op_to_mc[op];
		gen_op_add_sub_mul_div_mc(x64mc, stamp, mc_info[stamp].mc_code, tac.dst, tac.s1, tac.s2);
		break;

	default:
		ERR();
	}
}
static void gen_mc(Scope *scp, Optimize_level level)
{
	assert(scp->bb != 0);
	BasicBlock *bb = scp->bb;
	vector<Tac> *tac = 0;
	vector<X64mc> *mcs = 0;
	if(level == O0)
	{
		tac = &bb->tacO0;
		mcs = &bb->mcO0;
	}
	else if(level == O1)
	{
		tac = &bb->tacO1;
		mcs = &bb->mcO1;
	}

	X64mc mc;

	if (scp->sem_stamp == SEM_FUNC_DEFINE
	    || scp->sem_stamp == SEM_IF_cond
	    || scp->sem_stamp == SEM_IF_then
	    || scp->sem_stamp == SEM_IF_else
	    || scp->sem_stamp == SEM_WHILE_COND
	    || scp->sem_stamp == SEM_WHILE_BODY
	    || scp->sem_stamp == SEM_CONTINUE
	    || scp->sem_stamp == SEM_BREAK
	    || (scp->sem_stamp == SEM_UNNAMED_SCOPE && tac->size())
	    || scp->jmp_in.size() > 0
	    || scp->asts.size() > 0)
	{
		bb->is_entry_label = true;
	}

	for (Tac &code : *tac)
	{
		Semantic sem = code.ast->sem_stamp;
		if (sem == SEM_OPERATOR)
		{
			gen_op_mc(*mcs, code);
			continue;
		}

		switch (sem)
		{
		case SEM_SAVE_RET_VALUE:
			mc = X64mc(MC_SAVE_RET, code.s1);
			mc.ori_sem = "save_ret";
			mcs->push_back(mc);
			break;

			// todo
		case SEM_FUNC_CALL:
			ERR("todo sem_func* semty %d \n", sem);
			break;

		case SEM_IMMEDIATE:
			//			mc = X64mc(MC_LI, code.dst);
//			mc.const_num = code.const_num_value;
//			mc.ori_sem = "li";
//			bb->x64mc.push_back(mc);
//			break;
		default:
			ERR("%d \n", sem);
			break;
		}
	}

//	if (scp->jmp_out != 0)	// || scp->sem == SEM_COND_JMP || scp->sem == SEM_JMP)
//		bb->jmp_to = scp->jmp_out;
//
	if (scp->jmp_out)
	{
		Semantic stamp = scp->sem_stamp;
		if (stamp == SEM_IF_cond || stamp == SEM_WHILE_COND)
		{
			printf("bb.exit_jmp=%s \n", scp->name.c_str());
			assert(scp->asts.size());

			Ast *cond = scp->asts[0];
			bb->jmp_mc_stamp = op_to_jmp_mc[cond->op];
		}
		else if (stamp == SEM_UNNAMED_SCOPE
		    || stamp == SEM_CONTINUE
		    || stamp == SEM_BREAK)
		{
			bb->jmp_mc_stamp = MC_JMP;
		}
		else
		{
			ERR("NOTE: bb.exit_jmp=%s %d\n", scp->name.c_str(), stamp);
		}
	}

	if (tac->size() != mcs->size())
	{
		LOG("%lu %lu", tac->size(), mcs->size());
	}

	for (Scope *p : scp->clds)
		gen_mc(p, level);
}

static int _asm_len = 0;
#define PRINT_ASM_HEAD(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define PRINT_ASM(fmt, ...) _asm_len = 8 + printf("\t" fmt , ##__VA_ARGS__)
#define PRINT_ASM_sem	printf("%*s#%s %s=%%%d, %s=%%%d,  cyc %d", \
        (_asm_len < 48) ? (48 - _asm_len) : 2, "", \
        mc.ori_sem.c_str(),	\
        mc.s1 >= 0 ? vr_declare_manager.declare_at[mc.s1]->tk.src.c_str() : "", mc.s1,	\
        	mc.s2 >= 0 ? vr_declare_manager.declare_at[mc.s2]->tk.src.c_str() : "", mc.s2, 	\
			mc.start_cycle);	\
        printf("\n");

#if 1
#define PRINT_MORE	\
			printf("%*s#%s, cyc %d\n", \
				(_asm_len < 48) ? (48 - _asm_len) : 2, "", \
				mc.ori_sem.c_str(), mc.start_cycle);
#else
	#define PRINT_MORE	\
	//		printf("\n");
#endif

void dump_mc(Scope *scp, string mc_list_name)
{
	BasicBlock *bb = scp->bb;

	if (bb->is_entry_label != 0)
		PRINT_ASM_HEAD("\n%s:", scp->name.c_str());

	vector<X64mc> *mc_list;
	if (mc_list_name == "x64mc")
		mc_list = &bb->mcO0;
	else if (mc_list_name == "x64mc_schedu")
		mc_list = &bb->mc_schedu;
	else
		ERR("%s", mc_list_name.c_str());

	for (auto &mc : *mc_list)
	{
		MachineCodeStamp stamp = mc.stamp;
		MachineCodeStamp set_mc;

		switch (stamp)
		{
		case MC_LD:
			PRINT_ASM("%s %%%d, dword ptr [%d]", mc_info[stamp].mc_code, mc.s1, vr_off(mc.s1));
			PRINT_MORE
			break;

		case MC_ST:
			PRINT_ASM("%s dword ptr [%d], %%%d", mc_info[stamp].mc_code, vr_off(mc.s1), mc.s1);
			PRINT_MORE
			break;

		case MC_ASSIGN:
			PRINT_ASM("%s %%%d, %%%d", mc_info[stamp].mc_code, mc.dst, mc.s1);
			PRINT_MORE
			break;

		case MC_NEG:
			PRINT_ASM("%s %%%d", mc_info[stamp].mc_code, mc.s1);
			PRINT_MORE
			break;

//		case MC_ADD:
//			case MC_SUB:
//			case MC_IMUL:
//			PRINT_ASM("%s %%%d, %%%d", mc_info[stamp].mc_code, mc.s1, mc.s2);
//			PRINT_MORE
//			break;
//
//		case MC_DIV:
//			PRINT_ASM("mov eax, %%%d", mc.s1);
//			PRINT_MORE
//
//			PRINT_ASM("cdq \n");
//			PRINT_ASM("idiv %%%d div \n", mc.s2);
//			PRINT_ASM("mov %%%d, eax div \n", mc.s1);
			break;

		case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			case MC_DIV:

		case MC_CMP_E:
			case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:

			PRINT_ASM("%%%d = %s %%%d, %%%d", mc.dst, mc_info[stamp].mc_code, mc.s1, mc.s2);

//			set_mc = fake_cmp_mc_to_real_mc[mc_stamp].mc_set;
//			PRINT_ASM("%s %%%d", mc_info[set_mc].mc_code, mc.dst);
			PRINT_MORE
			break;

		case MC_TEST:
			PRINT_ASM("%s %%%d", mc_info[stamp].mc_code, mc.s1);
			PRINT_MORE
			break;

		case MC_SAVE_RET:
			PRINT_ASM("mov eax, %%%d", mc.s1);
			PRINT_MORE
			break;

//		case MC_LI:
			//			PRINT_ASM("%s %%%d, num %d", mc_info[stamp].mc_code, mc.s1, mc.const_num);
//			PRINT_MORE
//			break;
		default:
			ERR("%d \n", stamp);
			break;
		}
	}

	if (scp->jmp_out != 0)
	{
		PRINT_ASM("%s %s \n", mc_info[bb->jmp_mc_stamp].mc_code,
		    scp->jmp_out->name.c_str());
	}

	for (Scope *p : scp->clds)
		dump_mc(p, mc_list_name);
}

void gen_machine_code()
{
	gen_mc(&file_scp, O0);
	gen_mc(&file_scp, O1);

	PRINT_ASM_HEAD("========== mc ==========");
	dump_mc(&file_scp, "x64mc");
}

#undef PRINT_ASM_HEAD
#undef PRINT_ASM
