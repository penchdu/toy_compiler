/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
#include "basic_block.h"

const char *pr_name[] = {
//    [R8D] = "r8d",
//    [R9D] = "r9d",
    [R10D] = "r10d",
    [R11D] = "r11d",
    [R12D] = "r12d",
    [R13D] = "r13d",
    [R14D] = "r14d",
    [R15D] = "r15d",
//    [EBX] = "ebx",
    };

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

extern Mc2mc fake_cmp_mc_to_real_mc[];

vector<VrToPr> vr2pr;
vector<PrToVr> pr2vr(X64PR_MAX, {INVALID__VR});

static X64pr get_pr()
{
	for (int i = R10D; i < X64PR_MAX; i++)
	{
		if (pr2vr[i].vr == INVALID__VR)
		{
			return (X64pr) i;
		}
	}

	ERR("-O0");
	return X64PR_MAX;
}

//X64pr (*ptr_get_pr)();
static int get_pr__load_vr(vector<X64mc> &x64mc_alloced, int vr, int u)
{
	/*
	 * todo
	 * mc_assign %1, %1
	 * s1 == s2
	 */
	bool need_load = 0;
	if (vr2pr[vr].pr == X64PR_MAX)
	{
		int pr = get_pr();
		vr2pr[vr].pr = pr;
		vr2pr[vr].u = u;
		pr2vr[pr].vr = vr;
		need_load = (u & VR_USAGE_READ);
	}
	else if (!(vr2pr[vr].u & VR_USAGE_READ) && (u & VR_USAGE_READ))
	{
		need_load = 1;
	}

	vr2pr[vr].u |= u;

	if (need_load)
	{
		X64mc mc(MC_LD, vr);
		mc.pr1 = vr2pr[vr].pr;
		mc.ori_sem = "alloc";
		x64mc_alloced.push_back(mc);
	}

	return vr2pr[vr].pr;
}

static void spill_pr(vector<X64mc> &x64mc_alloced, int vr)
{
	int pr = vr2pr[vr].pr;
	int u = vr2pr[vr].u;

	assert(pr != X64PR_MAX);
	assert(u != VR_USAGE_INVALID);

	if (u & VR_USAGE_WRITE)
	{
		X64mc mc(MC_ST, vr);
		mc.pr1 = (int) pr;
		mc.ori_sem = "spill";
		x64mc_alloced.push_back(mc);
	}

	vr2pr[vr].pr = X64PR_MAX;
	vr2pr[vr].u = VR_USAGE_INVALID;

	assert(pr2vr[pr].vr == vr);
	pr2vr[pr].vr = INVALID__VR;
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

static void x64_reg_alloc_o0(Scope *scp)
{
	X64mc inst;

	BasicBlock *bb = (BasicBlock*) scp->basic_block;
	vector<X64mc> &x64mc_alloc = bb->x64mc_alloc_o0;

	for (X64mc &mc_schedu : bb->x64mc_schedu)
	{
		X64mc mc = mc_schedu;
		MachineCodeStamp mc_stamp = mc.mc_stamp;

		switch (mc_stamp)
		{
		case MC_LI:
			mc.pr1 = get_pr__load_vr(x64mc_alloc, mc.s1, VR_USAGE_WRITE);
			x64mc_alloc.push_back(mc);
			spill_pr(x64mc_alloc, mc.s1);
			break;

		case MC_ASSIGN:
//			if (mc.s1 == mc.s2)
//				break;

			mc.pr1 = get_pr__load_vr(x64mc_alloc, mc.s1, VR_USAGE_WRITE);
			mc.pr2 = get_pr__load_vr(x64mc_alloc, mc.s2, VR_USAGE_READ);
			x64mc_alloc.push_back(mc);
			spill_pr(x64mc_alloc, mc.s1, mc.s2);
			break;

		case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			case MC_DIV:

			mc.pr1 = get_pr__load_vr(x64mc_alloc, mc.s1, VR_USAGE_READ_WRITE);
			mc.pr2 = get_pr__load_vr(x64mc_alloc, mc.s2, VR_USAGE_READ);
			x64mc_alloc.push_back(mc);
			spill_pr(x64mc_alloc, mc.s1, mc.s2);
			break;

		case MC_CMP_E:
			case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:
			mc.pr_dst = get_pr__load_vr(x64mc_alloc, mc.dst, VR_USAGE_WRITE);
			mc.pr1 = get_pr__load_vr(x64mc_alloc, mc.s1, VR_USAGE_READ);
			mc.pr2 = get_pr__load_vr(x64mc_alloc, mc.s2, VR_USAGE_READ);

			x64mc_alloc.push_back(mc);
			spill_pr(x64mc_alloc, mc.dst);
			spill_pr(x64mc_alloc, mc.s1, mc.s2);
			break;

		case MC_SAVE_RET:
			mc.pr1 = get_pr__load_vr(x64mc_alloc, mc.s1, VR_USAGE_READ);
			x64mc_alloc.push_back(mc);
			spill_pr(x64mc_alloc, mc.s1);
			break;

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


int _asm_len = 0;
#define PRINT_ASM_HEAD(fmt, ...) fprintf(fp, fmt "\n", ##__VA_ARGS__)
#define PRINT_ASM(fmt, ...) _asm_len = 8 + fprintf(fp, "\t" fmt , ##__VA_ARGS__)
#define PRINT_ASM_sem	fprintf(fp, "%*s#%s %s=%%%d, %s=%%%d,  idx %d, cyc %d", \
	        (_asm_len < 48) ? (48 - _asm_len) : 2, "", \
	        mc.ori_sem.c_str(),	\
	        mc.s1 >= 0 ? vr_declare_manager.declare_at[mc.s1]->tk.src.c_str() : "", mc.s1,	\
        	mc.s2 >= 0 ? vr_declare_manager.declare_at[mc.s2]->tk.src.c_str() : "", mc.s2, 	\
			mc.idx, mc.start_cycle);	\
	        fprintf(fp, "\n");

static void dump_bb_asm(FILE *fp, Scope *scp, const string &asm_file)
{
//	LOG("scp: %s", scp->name.c_str());
	BasicBlock *bb = (BasicBlock*) scp->basic_block;
	if ((bb->entry_label != 0 /*&& (bb->x64mc_alloc_o0.size() || bb->x64mc_alloc_wave.size())*/)
		&& bb->entry_label->name != "test")
		PRINT_ASM_HEAD("\n\n%s:", bb->entry_label->name.c_str());

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
		MachineCodeStamp mc_stamp = mc.mc_stamp;
		MachineCodeStamp cmp_mc;
		MachineCodeStamp set_mc;

		switch (mc_stamp)
		{
		case MC_LD:
			PRINT_ASM("%s %s, dword ptr [rbp - %d]", mc.asm_code.c_str(), pr_name[mc.pr1], mc.of1);
			PRINT_ASM_sem
			break;

		case MC_ST:
			PRINT_ASM("%s dword ptr [rbp - %d], %s ", mc.asm_code.c_str(), mc.of1, pr_name[mc.pr1]);
			PRINT_ASM_sem
			break;

		case MC_LI:
			PRINT_ASM("%s %s, %d", mc.asm_code.c_str(), pr_name[mc.pr1], mc.const_num);
			PRINT_ASM_sem
			break;

		case MC_ASSIGN:
			case MC_ADD:
			case MC_SUB:
			case MC_IMUL:
			PRINT_ASM("%s %s, %s", mc.asm_code.c_str(), pr_name[mc.pr1], pr_name[mc.pr2]);
			PRINT_ASM_sem
			break;

		case MC_DIV:
			PRINT_ASM("mov eax, %s \n", pr_name[mc.pr1]);

			PRINT_ASM("cdq \n");
			PRINT_ASM("idiv %s \n", pr_name[mc.pr2]);
			PRINT_ASM("mov %s, eax", pr_name[mc.pr1]);
			PRINT_ASM_sem
			break;

		case MC_CMP_E:
			case MC_CMP_NE:
			case MC_CMP_L:
			case MC_CMP_LE:
			case MC_CMP_G:
			case MC_CMP_GE:

			cmp_mc = fake_cmp_mc_to_real_mc[mc_stamp].mc_cmp;
			PRINT_ASM("%s %s, %s", mc_info[cmp_mc].mc_code.c_str(), pr_name[mc.pr1], pr_name[mc.pr2]);
			PRINT_ASM_sem

			set_mc = fake_cmp_mc_to_real_mc[mc_stamp].mc_set;
			PRINT_ASM("%s %s", mc_info[set_mc].mc_code.c_str(), pr_name_byte(mc.pr_dst));
			PRINT_ASM_sem
			PRINT_ASM("movzx %s, %s", pr_name[mc.pr_dst], pr_name_byte(mc.pr_dst));
			PRINT_ASM_sem
			break;

		case MC_SAVE_RET:
			PRINT_ASM("#---------------- print ret ----------------# \n");
			PRINT_ASM("mov esi, %s \n", pr_name[mc.pr1]);
			PRINT_ASM("lea rdi, [rip + fmt] \n");
			PRINT_ASM("mov eax, 0 \n");
			PRINT_ASM("call printf@PLT \n");
			PRINT_ASM("#------------------------------------------# \n");

			PRINT_ASM("mov eax, %s \n", pr_name[mc.pr1]);
			break;

		default:
			ERR("%d", mc_stamp);
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
	int rsp_off = vr_declare_manager.offset;
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
	vr2pr.resize(vr_declare_manager.size());

	x64_reg_alloc_o0(&file_scp);
	dump_asm("a0.s");

	usleep(1000);
	x64_reg_alloc_wave();
	dump_asm("a1.s");
}
