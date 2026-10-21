/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

const char* pr_name(int pr)
{
	switch (pr)
	{
//	case R8D:
//		return "r8b";
	case EDI:
		return "edi";
	case R10D:
		return "r10d";
	case R11D:
		return "r11d";
	case R12D:
		return "r12d";
	case R13D:
		return "r13d";
	case R14D:
		return "r14d";
//	case R15D:
//		return "r15d";
	case PR_IMM:
		return "pr_imm";
	}

	ERR("no byte register");
	return 0;
}

const char* pr_name_byte(int pr)
{
	switch (pr)
	{
//	case R8D:
//		return "r8b";
	case EDI:
		return "dil";
	case R10D:
		return "r10b";
	case R11D:
		return "r11b";
	case R12D:
		return "r12b";
	case R13D:
		return "r13b";
	case R14D:
		return "r14b";
//	case R15D:
//		return "r15b";
//	case EBX:
//		return "bl";
	}

	ERR("no byte register");
	return 0;
}

vector<VrState> vr_state;
vector<PrState> pr_state(X64PR_MAX, {INVALID_VR});

extern Mc2mc fake_cmp_mc_to_real_mc[];

const char* pr_name_asm(int pr, int vr)
{
	if (pr == PR_IMM)
	{
		static string s;
		s = to_string(declare_record.ast[vr]->const_value);
		return s.c_str();
	}
	assert(pr != X64PR_MAX);
	return pr_name(pr);
}

void spill_vr(
    enum AllocMethod method,
    vector<VrState> &vr_st,
    vector<PrState> &pr_st,
    int vr, vector<X64mc> &mcs,
    const string &tag)
{
	int &pr = vr_st[vr].pr;
	int &u = vr_st[vr].u;
	bool &dirty = vr_st[vr].dirty;

	Ast *p = declare_record.ast[vr];
	if (pr > X64PR_MAX)
	{
//		LOG("vr=%%%d, pr=%d", vr, pr);
		assert(p->sem_stamp == SEM_IMMEDIATE && pr == PR_IMM);
		assert(dirty == false);
		return;
	}

	if (pr == X64PR_MAX || u == VR_USAGE_INVALID || pr_st[pr].vr != vr)
		ERR("pr=%d, u=%d, pr_st[%d].vr=%d != %d", pr, u, pr, pr_st[pr].vr, vr);

	// MC_victim
	X64mc mc(MC_victim, vr);
	mc.pr1 = (int) pr;
	mc.ori_sem = string("victim ") + pr_name(pr) + tag;
	mcs.push_back(mc);

	if (method == O0 ||
	    (/*p->sem_stamp != SEM_CONST_NUM &&*/
	    method == WAVE && dirty
	        && p->symb->consume_cnt < p->symb->use_cnt))
	{
		X64mc mc(MC_ST, vr);
		mc.pr1 = (int) pr;
		mc.ori_sem = string("memory spill ") + pr_name(pr) + tag;
		mcs.push_back(mc);
	}

	pr_st[pr].vr = INVALID_VR;
	pr = X64PR_MAX;
	u = VR_USAGE_INVALID;
	dirty = false;
}

static void spill_vr(int vr, vector<X64mc> &mcs)
{
	spill_vr(O0, vr_state, pr_state, vr, mcs);
}
static void spill_vr(const X64mc &mc, vector<X64mc> &mcs)
{
	if (mc.dst >= 0)
		spill_vr(mc.dst, mcs);

	if (mc.s1 >= 0 && mc.s1 == mc.s2)
	{
		spill_vr(mc.s1, mcs);
		return;
	}

	if (mc.s1 >= 0)
		spill_vr(mc.s1, mcs);
	if (mc.s2 >= 0)
		spill_vr(mc.s2, mcs);
}

int get_pr_o0()
{
	for (int i = 0; i < X64PR_MAX; i++)
	{
		if (pr_state[i].vr == INVALID_VR)
			return i;
	}

	ERR("-O0");
	return X64PR_MAX;
}

//int (*func_get_pr)();
int get_pr(enum AllocMethod method,
    vector<X64mc> &mcs,
    BasicBlock *bb, int mc_idx,
    int vr, int u,
    bool alloc_immediate)
{
	Ast *p = declare_record.ast[vr];
	if (p->sem_stamp == SEM_IMMEDIATE)
	{
		assert(u == VR_USAGE_READ);

		if (!alloc_immediate)
		{
			vr_state[vr].pr = PR_IMM;
			vr_state[vr].u = VR_USAGE_READ;

			return PR_IMM;
		}
		else
		{
			vr_state[vr].pr = X64PR_MAX;
		}
	}

	/*
	 * todo
	 * mc_assign %1, %1
	 * s1 == s2
	 */
	assert(u != VR_USAGE_INVALID);
	bool need_load = 0;

	if (vr_state[vr].pr == X64PR_MAX)
	{
		int pr = X64PR_MAX;

		if (method == O0)
			pr = get_pr_o0();
		else if (method == WAVE)
			pr = get_pr_wave(bb, mc_idx);
		else
			ERR();
		assert(pr != X64PR_MAX);

		vr_state[vr].pr = pr;
		vr_state[vr].u = u;
		vr_state[vr].dirty = u & VR_USAGE_WRITE;
		pr_state[pr].vr = vr;
		need_load = (u & VR_USAGE_READ);
	}
	else
	{
		vr_state[vr].dirty |= (u & VR_USAGE_WRITE);

//		if (!(vr_state[vr].u & VR_USAGE_READ) && (u & VR_USAGE_READ))
//		{

		/*
		 * it will be not ok in a=a+1,
		 * but now,  a=a+1;  ->  t=a+1;  a=t;
		 * error will happen when delete t, unless alloc the 2nd a first
		 *
		 */
//			need_load = 1;
//		}
	}

	vr_state[vr].u |= u;

	if (need_load)
	{
		Ast *p = declare_record.ast[vr];
		if (p->sem_stamp == SEM_IMMEDIATE)
		{
			X64mc mc(MC_LI, vr);
			mc.pr1 = vr_state[vr].pr;
			mc.const_num = p->const_value;
			mc.ori_sem = "li";
			mcs.push_back(mc);
		}
		else
		{
			X64mc mc(MC_LD, vr);
			mc.pr1 = vr_state[vr].pr;
			mc.ori_sem = "memory ld";
			mcs.push_back(mc);

//			vr_state[vr].dirty = 0;
		}
	}

	return vr_state[vr].pr;
}

int get_pr_alloc_imm(
    enum AllocMethod method,
    vector<X64mc> &mcs,
    BasicBlock *bb, int mc_idx, int vr)
{
	Ast *p = declare_record.ast[vr];
	if (p->sem_stamp == SEM_IMMEDIATE)
		return get_pr(method, mcs, bb, mc_idx, vr, VR_USAGE_READ, true);

	return get_pr(method, mcs, bb, mc_idx, vr, VR_USAGE_READ);
}

static int get_pr(BasicBlock *bb, int vr, int u)
{
	return get_pr(O0, bb->x64mc_alloc_o0, bb, 0, vr, u);
}
static int get_pr_alloc_imm(BasicBlock *bb, int vr)
{
	return get_pr_alloc_imm(O0, bb->x64mc_alloc_o0, bb, 0, vr);
}

static void x64_reg_alloc_o0(Scope *scp)
{
	X64mc inst;

	BasicBlock *bb = scp->basic_block;
	vector<X64mc> &mcs = bb->x64mc_alloc_o0;

	if (scp->sem_stamp == SEM_FUNC_DEFINE)
	{
		if (scp->arg.size() == 1)
		{
			int vr = scp->arg[0];

			assert(vr_state[vr].pr == X64PR_MAX);
			assert(pr_state[EDI].vr == INVALID_VR);

			vr_state[vr].pr = EDI;
			vr_state[vr].u = VR_USAGE_WRITE;
			vr_state[vr].dirty = 1;

			pr_state[EDI].vr = vr;
		}
	}

	for (int i = 0; i < bb->x64mc_schedu.size(); i++)
	{
		X64mc mc = bb->x64mc_schedu[i];
		MachineCodeStamp mc_stamp = mc.stamp;

		switch (mc_stamp)
		{
		case MC_ASSIGN:
			//			if (mc.s1 == mc.s2)
//				break;
			mc.pr1 = get_pr(bb, mc.s1, VR_USAGE_READ);
			mc.pr_dst = get_pr(bb, mc.dst, VR_USAGE_WRITE);
			mcs.push_back(mc);
			spill_vr(mc, mcs);
			break;

		case MC_ADD:
			case MC_SUB:
			case MC_IMUL:

		case MC_CMP_E:
			case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:
			mc.pr1 = get_pr_alloc_imm(bb, mc.s1);
			mc.pr2 = get_pr(bb, mc.s2, VR_USAGE_READ);
			mc.pr_dst = get_pr(bb, mc.dst, VR_USAGE_WRITE);

			mcs.push_back(mc);
			spill_vr(mc, mcs);
			break;

		case MC_DIV:
			mc.pr1 = get_pr(bb, mc.s1, VR_USAGE_READ);
			mc.pr2 = get_pr_alloc_imm(bb, mc.s2);
			mc.pr_dst = get_pr(bb, mc.dst, VR_USAGE_WRITE);

			mcs.push_back(mc);
			spill_vr(mc, mcs);
			break;

		case MC_TEST:
			mc.pr1 = get_pr_alloc_imm(bb, mc.s1);

			mcs.push_back(mc);
			spill_vr(mc, mcs);
			break;

		case MC_SAVE_RET:
			mc.pr1 = get_pr(bb, mc.s1, VR_USAGE_READ);
			mcs.push_back(mc);
			spill_vr(mc, mcs);
			break;

		case MC_victim:
			default:
			ERR("%d \n", mc_stamp);
			break;
		}
	}

	for (Scope *p : scp->clds)
		x64_reg_alloc_o0(p);
}

int align16(int &n)
{
	int align = 16;
	n = ((n / align) + (bool) (n % align)) * align;
	return n;

//	return (n + 15) & ~15;
}

//static void dump()
//{
//	printf("\n========== mc alloc ==========\n");
//	dump_mc(&file_scp);
//}

static int _asm_len = 0;
static void print_asm_sem(FILE *fp, X64mc &mc)
{
	fprintf(fp, "%*s#%s", (_asm_len < 48) ? (48 - _asm_len) : 2, "", mc.ori_sem.c_str());

	if (mc.dst >= 0)
		fprintf(fp, ", %s %%%d", vr_name(mc.dst), mc.dst);
	if (mc.s1 >= 0)
		fprintf(fp, ", %s %%%d", vr_name(mc.s1), mc.s1);
	if (mc.s2 >= 0)
		fprintf(fp, ", %s %%%d", vr_name(mc.s2), mc.s2);
	fprintf(fp, "  cyc %d\n", mc.start_cycle);
}

#define PRINT_ASM_HEAD(fmt, ...) fprintf(fp, fmt "\n", ##__VA_ARGS__)
#define PRINT_ASM(fmt, ...) _asm_len = 8 + fprintf(fp, "\t" fmt , ##__VA_ARGS__)
#define PRINT_ASM_sem	print_asm_sem(fp, mc);

static void dump_bb_asm(FILE *fp, Scope *scp, const string &asm_file)
{
//	LOG("scp: %s", scp->name.c_str());
	if (scp->sem_stamp == SEM_FUNC_DEFINE)
	{
		PRINT_ASM_HEAD(".intel_syntax noprefix");
		PRINT_ASM_HEAD(".globl %s \n", scp->name.c_str());
		PRINT_ASM_HEAD("%s:", scp->name.c_str());

		int rsp_off = declare_record.offset;
		align16(rsp_off);
		//	printf("vreg.offset %d, rsp_of %d\n", vreg.offset, rsp_of);
		PRINT_ASM("push rbp \n");
		PRINT_ASM("mov rbp, rsp \n");
		PRINT_ASM("sub rsp, %d \n\n", rsp_off);

	}
	//////////////////////////////////////////////////

	BasicBlock *bb = scp->basic_block;
	if ((
	    bb->is_entry_label
	        || bb->x64mc_alloc_o0.size()
	        || bb->x64mc_alloc_wave.size()
	    )
	    && scp->sem_stamp != SEM_FUNC_DEFINE
	        )
	{
		PRINT_ASM_HEAD("\n%s:", scp->name.c_str());
	}

	vector<X64mc> *mc_list;
	if (asm_file == "a0.s")
		mc_list = &bb->x64mc_alloc_o0;
	else if (asm_file == "a1.s")
		mc_list = &bb->x64mc_alloc_wave;
	else
		ERR("%s", asm_file.c_str());

//	if(scp->sem_stamp == SEM_FUNC_DEFINE)
//	{
//		assert(scp->arg.size() == 1);
//		int vr = scp->arg[0];
//	}

	for (X64mc &r : *mc_list)
	{
		X64mc mc = r;
		MachineCodeStamp stamp = mc.stamp;
		MachineCodeStamp mc_cmp;
		MachineCodeStamp mc_set;

		switch (stamp)
		{
		case MC_LI:
			PRINT_ASM("%s %s, %d", mc.asm_code.c_str(), pr_name_asm(mc.pr1, mc.s1), mc.const_num);
			PRINT_ASM_sem
			break;

		case MC_LD:
			PRINT_ASM("%s %s, dword ptr [rbp - %d]", mc.asm_code.c_str(), pr_name_asm(mc.pr1, mc.s1), mc.of1);
			PRINT_ASM_sem
			break;

		case MC_victim:
			PRINT_ASM("");
			PRINT_ASM_sem
			break;

		case MC_ST:
			PRINT_ASM("%s dword ptr [rbp - %d], %s ", mc.asm_code.c_str(), mc.of1, pr_name_asm(mc.pr1, mc.s1));
			PRINT_ASM_sem
			break;

		case MC_ASSIGN:
			PRINT_ASM("%s %s, %s", mc.asm_code.c_str(), pr_name_asm(mc.pr_dst, mc.dst), pr_name_asm(mc.pr1, mc.s1));
			PRINT_ASM_sem
			break;

		case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			PRINT_ASM("mov %s, %s \n", pr_name_asm(mc.pr_dst, mc.dst), pr_name_asm(mc.pr1, mc.s1));
			PRINT_ASM("%s %s, %s", mc.asm_code.c_str(), pr_name_asm(mc.pr_dst, mc.dst), pr_name_asm(mc.pr2, mc.s2));
			PRINT_ASM_sem
			break;

		case MC_DIV:
			PRINT_ASM("mov eax, %s \n", pr_name_asm(mc.pr1, mc.s1));
			PRINT_ASM("cdq \n");
			PRINT_ASM("idiv %s \n", pr_name_asm(mc.pr2, mc.s2));
			PRINT_ASM("mov %s, eax", pr_name_asm(mc.pr_dst, mc.dst));
			PRINT_ASM_sem
			break;

		case MC_CMP_E:
			case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:

			mc_cmp = fake_cmp_mc_to_real_mc[stamp].mc_cmp;
			PRINT_ASM("%s %s, %s", mc_info[mc_cmp].mc_code.c_str(),
			    pr_name_asm(mc.pr1, mc.s1), pr_name_asm(mc.pr2, mc.s2));
			PRINT_ASM_sem

			mc_set = fake_cmp_mc_to_real_mc[stamp].mc_set;
			PRINT_ASM("%s %s", mc_info[mc_set].mc_code.c_str(), pr_name_byte(mc.pr_dst));
			PRINT_ASM_sem

			PRINT_ASM("movzx %s, %s", pr_name(mc.pr_dst), pr_name_byte(mc.pr_dst));
			PRINT_ASM_sem
			break;

		case MC_TEST:
			PRINT_ASM("%s %s, %s", mc_info[stamp].mc_code.c_str(),
			    pr_name_asm(mc.pr1, mc.s1), pr_name_asm(mc.pr1, mc.s1));
			PRINT_ASM_sem

			break;

		case MC_SAVE_RET:
			//			PRINT_ASM("#---------------- print ret ----------------# \n");
//			PRINT_ASM("mov esi, %s \n", pr_name_asm(mc.pr1, mc.s1));
//			PRINT_ASM("lea rdi, [rip + fmt] \n");
//			PRINT_ASM("mov eax, 0 \n");
//			PRINT_ASM("call printf@PLT \n");
//			PRINT_ASM("#------------------------------------------# \n");

			PRINT_ASM("mov eax, %s \n", pr_name_asm(mc.pr1, mc.s1));
			break;

		default:
			ERR("%d", stamp);
			break;
		}
	}

	if (scp->jmp_out != 0)
	{
		PRINT_ASM("%s %s \n", mc_info[bb->jmp_mc_stamp].mc_code.c_str(),
		    scp->jmp_out->name.c_str());
	}

	for (Scope *p : scp->clds)
		dump_bb_asm(fp, p, asm_file);

	//////////////////////////////////////////////////

	if (scp->sem_stamp == SEM_FUNC_DEFINE)
	{
		PRINT_ASM_HEAD("\n\n.L_return:");
		PRINT_ASM("mov rsp, rbp \n");
		PRINT_ASM("pop rbp \n");
		PRINT_ASM("ret \n\n");
		PRINT_ASM_HEAD(".section .note.GNU-stack,\"\",@progbits");
	}
}
static void dump_asm(const string &asm_file)
{
	FILE *fp = fopen(asm_file.c_str(), "w");
	assert(fp);
	// string s = "#========== " + asm_file + " ==========#";

	// PRINT_ASM_HEAD("%s", s.c_str());
//PRINT_ASM_HEAD(".intel_syntax noprefix");
//	PRINT_ASM_HEAD(".extern printf");
//
//	PRINT_ASM_HEAD(".section .rodata");
//	PRINT_ASM_HEAD("fmt:");
//	PRINT_ASM(".string \"Result: %%d\\n\" \n");
//
//	PRINT_ASM_HEAD(".section .text");

	dump_bb_asm(fp, &file_scp, asm_file);
	fclose(fp);
}
#undef PRINT_ASM_HEAD
#undef PRINT_ASM
#undef PRINT_ASM_sem

void x64_reg_alloc()
{
	vr_state.resize(vr_size);

	x64_reg_alloc_o0(&file_scp);
	dump_asm("a0.s");

	usleep(1000);
	x64_reg_alloc_wave();
	dump_asm("a1.s");
}
