/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

//const int X64PR_CONST = X64PR_MAX + 1;

const char* pr_name(int pr)
{
	switch (pr)
	{
//	case R8D:
//		return "r8b";
//	case R9D:
//		return "r9b";
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
	case R15D:
		return "r15d";
	case PR_IMM:
		return "pr_imm";
	}

	ERR("no byte register");
	return 0;
}
;

const char* pr_name_byte(int pr)
{
	switch (pr)
	{
//	case R8D:
//		return "r8b";
//	case R9D:
//		return "r9b";
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
	case R15D:
		return "r15b";
//	case EBX:
//		return "bl";
	}

	ERR("no byte register");
	return 0;
}

vector<VrState> vr_state;
vector<PrState> pr_state(X64PR_MAX, {INVALID_VR});

extern Mc2mc fake_cmp_mc_to_real_mc[];

const char* pr_name_const(int pr, int vr)
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
static int get_pr()
{
	for (int i = R10D; i < X64PR_MAX; i++)
	{
		if (pr_state[i].vr == INVALID_VR)
		{
			return i;
		}
	}

	ERR("-O0");
	return X64PR_MAX;
}

//X64pr (*ptr_get_pr)();
static int get_pr__load_vr(vector<X64mc> &x64mc_alloced, int vr, int u,
    bool immediate_force_alloc = false)
{
	Ast *p = declare_record.ast[vr];
	if (!immediate_force_alloc && p->sem_stamp == SEM_IMMEDIATE)
	{
		vr_state[vr].pr = PR_IMM;
		vr_state[vr].u = VR_USAGE_READ;
		return PR_IMM;
	}

	if (immediate_force_alloc && p->sem_stamp == SEM_IMMEDIATE && vr_state[vr].pr == PR_IMM)
	{
		vr_state[vr].pr = X64PR_MAX;
	}

	/*
	 * todo
	 * mc_assign %1, %1
	 * s1 == s2
	 */
	bool need_load = 0;
	if (vr_state[vr].pr == X64PR_MAX)
	{
		int pr = get_pr();
		vr_state[vr].pr = pr;
		vr_state[vr].u = u;
		pr_state[pr].vr = vr;
		need_load = (u & VR_USAGE_READ);
	}
	else if (!(vr_state[vr].u & VR_USAGE_READ) && (u & VR_USAGE_READ))
	{
		need_load = 1;
	}

	vr_state[vr].u |= u;

	if (need_load)
	{
		X64mc mc(MC_LD, vr);
		mc.pr1 = vr_state[vr].pr;
		mc.ori_sem = "alloc";
		x64mc_alloced.push_back(mc);
	}

	return vr_state[vr].pr;
}

static void spill_pr(vector<X64mc> &x64mc_alloced, int vr)
{
	int pr = vr_state[vr].pr;
	int u = vr_state[vr].u;

	assert(pr != X64PR_MAX);
	assert(u != VR_USAGE_INVALID);

	Ast *p = declare_record.ast[vr];
	if (p->sem_stamp == SEM_IMMEDIATE && pr > X64PR_MAX)
	{
		LOG("vr=%%%d, pr=%d", vr, vr_state[vr].pr);
		assert(vr_state[vr].pr > X64PR_MAX);
		return;
	}

	if (u & VR_USAGE_WRITE)
	{
		X64mc mc(MC_ST, vr);
		mc.pr1 = (int) pr;
		mc.ori_sem = "spill";
		x64mc_alloced.push_back(mc);
	}

	vr_state[vr].pr = X64PR_MAX;
	vr_state[vr].u = VR_USAGE_INVALID;

	assert(pr_state[pr].vr == vr);
	pr_state[pr].vr = INVALID_VR;
}
static void spill_pr(vector<X64mc> &x64mc_alloced, int vr1, int vr2)
{
	if (vr1 == vr2)
	{
		spill_pr(x64mc_alloced, vr1);
		return;
	}
	spill_pr(x64mc_alloced, vr1);
	spill_pr(x64mc_alloced, vr2);
}

static int load_immediate(vector<X64mc> &mcs, int vr)
{
	int value = declare_record.ast[vr]->const_value;

	X64mc mc;
	mc = X64mc(MC_LI);
	mc.s1 = vr;
	mc.pr1 = get_pr__load_vr(mcs, vr, VR_USAGE_READ, true);
	mc.const_num = value;
	mc.ori_sem = "li";
	mcs.push_back(mc);

	return mc.pr1;
}

static void x64_reg_alloc_o0(Scope *scp)
{
	X64mc inst;

	BasicBlock *bb = scp->basic_block;
	vector<X64mc> &mcs = bb->x64mc_alloc_o0;

	for (X64mc &mc_schedu : bb->x64mc_schedu)
	{
		X64mc mc_li;
		X64mc mc = mc_schedu;
		MachineCodeStamp mc_stamp = mc.stamp;

		switch (mc_stamp)
		{
		case MC_ASSIGN:
			//			if (mc.s1 == mc.s2)
//				break;

			mc.pr_dst = get_pr__load_vr(mcs, mc.dst, VR_USAGE_WRITE);
			mc.pr1 = get_pr__load_vr(mcs, mc.s1, VR_USAGE_READ);
			mcs.push_back(mc);
			spill_pr(mcs, mc.dst, mc.s1);
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
			mc.pr_dst = get_pr__load_vr(mcs, mc.dst, VR_USAGE_WRITE);

			if (declare_record.ast[mc.s1]->sem_stamp == SEM_IMMEDIATE)
				mc.pr1 = load_immediate(mcs, mc.s1);
			else
				mc.pr1 = get_pr__load_vr(mcs, mc.s1, VR_USAGE_READ);

			mc.pr2 = get_pr__load_vr(mcs, mc.s2, VR_USAGE_READ);

			mcs.push_back(mc);
			spill_pr(mcs, mc.dst);
			spill_pr(mcs, mc.s1, mc.s2);
			break;

		case MC_DIV:
			mc.pr_dst = get_pr__load_vr(mcs, mc.dst, VR_USAGE_WRITE);

			mc.pr1 = get_pr__load_vr(mcs, mc.s1, VR_USAGE_READ);

			if (declare_record.ast[mc.s2]->sem_stamp == SEM_IMMEDIATE)
				mc.pr2 = load_immediate(mcs, mc.s2);
			else
				mc.pr2 = get_pr__load_vr(mcs, mc.s2, VR_USAGE_READ);

			mcs.push_back(mc);
			spill_pr(mcs, mc.dst);
			spill_pr(mcs, mc.s1, mc.s2);
			break;

		case MC_CMP_TEST:
			mc.pr_dst = get_pr__load_vr(mcs, mc.dst, VR_USAGE_WRITE);

			if (declare_record.ast[mc.s1]->sem_stamp == SEM_IMMEDIATE)
				mc.pr1 = load_immediate(mcs, mc.s1);
			else
				mc.pr1 = get_pr__load_vr(mcs, mc.s1, VR_USAGE_READ);

			mcs.push_back(mc);
			spill_pr(mcs, mc.dst, mc.s1);
			break;

		case MC_SAVE_RET:
			mc.pr1 = get_pr__load_vr(mcs, mc.s1, VR_USAGE_READ);
			mcs.push_back(mc);
			spill_pr(mcs, mc.s1);
			break;

//		case MC_LI:
			//			mc.pr1 = get_pr__load_vr(x64mc_alloc, mc.s1, VR_USAGE_WRITE);
//			x64mc_alloc.push_back(mc);
//			spill_pr(x64mc_alloc, mc.s1);
//			break;
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
#define PRINT_ASM_HEAD(fmt, ...) fprintf(fp, fmt "\n", ##__VA_ARGS__)
#define PRINT_ASM(fmt, ...) _asm_len = 8 + fprintf(fp, "\t" fmt , ##__VA_ARGS__)
#define PRINT_ASM_sem	fprintf(fp, "%*s#%s %s=%%%d, %s=%%%d,  cyc %d", \
	        (_asm_len < 48) ? (48 - _asm_len) : 2, "", \
	        mc.ori_sem.c_str(),	\
	        mc.s1 >= 0 ? declare_record.ast[mc.s1]->tk.src.c_str() : "", mc.s1,	\
        	mc.s2 >= 0 ? declare_record.ast[mc.s2]->tk.src.c_str() : "", mc.s2, 	\
			mc.start_cycle);	\
	        fprintf(fp, "\n");

static void dump_bb_asm(FILE *fp, Scope *scp, const string &asm_file)
{
//	LOG("scp: %s", scp->name.c_str());
	BasicBlock *bb = scp->basic_block;
	if ((
	    bb->is_entry_label
	        || bb->x64mc_alloc_o0.size()
	        || bb->x64mc_alloc_wave.size()
	    )
	    && scp->name != "test")
		PRINT_ASM_HEAD("\n\n%s:", scp->name.c_str());

	vector<X64mc> *mc_list;
	if (asm_file == "a0.s")
		mc_list = &bb->x64mc_alloc_o0;
	else if (asm_file == "a1.s")
		mc_list = &bb->x64mc_alloc_wave;
	else
		ERR("%s", asm_file.c_str());

	for (X64mc &r : *mc_list)
	{
		X64mc mc = r;
		MachineCodeStamp stamp = mc.stamp;
		MachineCodeStamp cmp_mc;
		MachineCodeStamp set_mc;

		switch (stamp)
		{
		case MC_LI:
			PRINT_ASM("%s %s, %d", mc.asm_code.c_str(), pr_name_const(mc.pr1, mc.s1), mc.const_num);
			PRINT_ASM_sem
			break;

		case MC_LD:
			PRINT_ASM("%s %s, dword ptr [rbp - %d]", mc.asm_code.c_str(), pr_name_const(mc.pr1, mc.s1), mc.of1);
			PRINT_ASM_sem
			break;

		case MC_victim:
			PRINT_ASM("");
			PRINT_ASM_sem
			break;

		case MC_ST:
			PRINT_ASM("%s dword ptr [rbp - %d], %s ", mc.asm_code.c_str(), mc.of1, pr_name_const(mc.pr1, mc.s1));
			PRINT_ASM_sem
			break;

		case MC_ASSIGN:
			PRINT_ASM("%s %s, %s", mc.asm_code.c_str(), pr_name_const(mc.pr_dst, mc.dst), pr_name_const(mc.pr1, mc.s1));
			PRINT_ASM_sem
			break;

		case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			PRINT_ASM("mov %s, %s \n", pr_name_const(mc.pr_dst, mc.dst), pr_name_const(mc.pr1, mc.s1));
			PRINT_ASM("%s %s, %s", mc.asm_code.c_str(), pr_name_const(mc.pr_dst, mc.dst), pr_name_const(mc.pr2, mc.s2));
			PRINT_ASM_sem
			break;

		case MC_DIV:
			PRINT_ASM("mov eax, %s \n", pr_name_const(mc.pr1, mc.s1));
			PRINT_ASM("cdq \n");
			PRINT_ASM("idiv %s \n", pr_name_const(mc.pr2, mc.s2));
			PRINT_ASM("mov %s, eax", pr_name_const(mc.pr_dst, mc.dst));
			PRINT_ASM_sem
			break;

		case MC_CMP_E:
			case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:

			cmp_mc = fake_cmp_mc_to_real_mc[stamp].mc_cmp;
			PRINT_ASM("%s %s, %s", mc_info[cmp_mc].mc_code.c_str(),
				pr_name_const(mc.pr1, mc.s1),
			    pr_name_const(mc.pr2, mc.s2));
			PRINT_ASM_sem

			set_mc = fake_cmp_mc_to_real_mc[stamp].mc_set;
			PRINT_ASM("%s %s", mc_info[set_mc].mc_code.c_str(), pr_name_byte(mc.pr_dst));
			PRINT_ASM_sem
			PRINT_ASM("movzx %s, %s", pr_name(mc.pr_dst), pr_name_byte(mc.pr_dst));
			PRINT_ASM_sem
			break;

		case MC_CMP_TEST:
			PRINT_ASM("test %s, %s", pr_name_const(mc.pr1, mc.s1), pr_name_const(mc.pr1, mc.s1));
			PRINT_ASM_sem

			set_mc = fake_cmp_mc_to_real_mc[stamp].mc_set;
			PRINT_ASM("%s %s", mc_info[set_mc].mc_code.c_str(), pr_name_byte(mc.pr_dst));
			PRINT_ASM_sem
			PRINT_ASM("movzx %s, %s", pr_name(mc.pr_dst), pr_name_byte(mc.pr_dst));
			PRINT_ASM_sem
			break;

		case MC_SAVE_RET:
			PRINT_ASM("#---------------- print ret ----------------# \n");
			PRINT_ASM("mov esi, %s \n", pr_name_const(mc.pr1, mc.s1));
			PRINT_ASM("lea rdi, [rip + fmt] \n");
			PRINT_ASM("mov eax, 0 \n");
			PRINT_ASM("call printf@PLT \n");
			PRINT_ASM("#------------------------------------------# \n");

			PRINT_ASM("mov eax, %s \n", pr_name_const(mc.pr1, mc.s1));
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
}
static void dump_asm(const string &asm_file)
{
	int rsp_off = declare_record.offset;
	align16(rsp_off);
	//	printf("vreg.offset %d, rsp_of %d\n", vreg.offset, rsp_of);

	FILE *fp = fopen(asm_file.c_str(), "w");
	assert(fp);
	string s = "#========== " + asm_file + " ==========#";

	PRINT_ASM_HEAD("%s", s.c_str());
	PRINT_ASM_HEAD(".intel_syntax noprefix");
	PRINT_ASM_HEAD(".extern printf");

	PRINT_ASM_HEAD(".section .rodata");
	PRINT_ASM_HEAD("fmt:");
	PRINT_ASM(".string \"Result: %%d\\n\" \n");

	PRINT_ASM_HEAD(".section .text");
	PRINT_ASM_HEAD(".global main");

	PRINT_ASM_HEAD("\nmain:");
	PRINT_ASM("push rbp \n");
	PRINT_ASM("mov rbp, rsp \n");
	PRINT_ASM("sub rsp, %d \n\n", rsp_off);

	dump_bb_asm(fp, &file_scp, asm_file);

	PRINT_ASM_HEAD("\n\n.L_return:");
	PRINT_ASM("mov rsp, rbp \n");
	PRINT_ASM("pop rbp \n");
	PRINT_ASM("ret \n\n");
	PRINT_ASM_HEAD(".section .note.GNU-stack,\"\",@progbits");

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
