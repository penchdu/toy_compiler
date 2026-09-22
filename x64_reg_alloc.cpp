///*
// * a.cpp
// *
// *  Created on: 2026年9月5日
// *      Author: x
// */
//
//#include "h.h"
//#include "x64_back_end.h"
//#include "scope.h"
//
//enum X64pr
//{
//	R10D,
//	R11D,
//	R12D,
//	R13D,
//	R14D,
//	R15D,
//
////	eax,
//
//	x64pr_max,
//};
//vector<int> x64pr_state(x64pr_max, 1);
//
//static const char *pr_name[] =
//        {
//                [R10D] = "r10d",
//                [R11D] = "r11d",
//                [R12D] = "r12d",
//                [R13D] = "r13d",
//                [R14D] = "r14d",
//                [R15D] = "r15d",
//
////				[eax] = "eax",
//        };
//
//enum VrUsage
//{
//	VR_USAGE_READ = 1 << 0,
//	VR_USAGE_WRITE = 1 << 1,
//	VR_USAGE_READ_WRITE = VR_USAGE_READ | VR_USAGE_WRITE,
//
//	VR_USEAGE_INVALID = 0,
//};
//struct Vr2Pr
//{
//	X64pr pr = x64pr_max;
//	int u = VR_USEAGE_INVALID;
//};
//vector<Vr2Pr> vr2pr;
//
//static X64pr get_pr()
//{
//	for (int i = R10D; i < x64pr_max; i++)
//	{
//		if (x64pr_state[i] == 1)
//		{
//			x64pr_state[i] = 0;
//			return (X64pr) i;
//		}
//	}
//
//	ERR("-O0");
//	return x64pr_max;
//}
//X64pr get_pr__load_vr(vector<X64mc> &x64mc_alloced, int vr, int u)
//{
//	/*
//	 * todo
//	 * mc_assign %1, %1
//	 * s1 == s2
//	 */
//	bool need_load = 0;
//	if (vr2pr[vr].pr == x64pr_max)
//	{
//		vr2pr[vr].pr = get_pr();
//		vr2pr[vr].u = u;
//		need_load = (u & VR_USAGE_READ);
//	}
//	else if (!(vr2pr[vr].u & VR_USAGE_READ)
//	        && (u & VR_USAGE_READ))
//	{
//		need_load = 1;
//	}
//	vr2pr[vr].u |= u;
//
//	if (need_load)
//	{
//		X64mc mc(MC_LD, vr);
//		mc.pr1 = vr2pr[vr].pr;
//		mc.ori_sem = "alloc";
//		x64mc_alloced.push_back(mc);
//	}
//
//	return vr2pr[vr].pr;
//}
//
//void spill_pr(vector<X64mc> &x64mc_alloced, int vr)
//{
//	X64pr pr = vr2pr[vr].pr;
//	int u = vr2pr[vr].u;
//
//	assert(pr != x64pr_max);
//	assert(u != VR_USEAGE_INVALID);
//
//	if (u & VR_USAGE_WRITE)
//	{
//		X64mc mc(MC_ST, vr);
//		mc.pr1 = (int) pr;
//		mc.ori_sem = "spill";
//		x64mc_alloced.push_back(mc);
//	}
//
//	vr2pr[vr].pr = x64pr_max;
//	vr2pr[vr].u = VR_USEAGE_INVALID;
//
//	assert(x64pr_state[pr] == 0);
//	x64pr_state[pr] = 1;
//}
//void spill_pr(vector<X64mc> &x64mc_alloced, int vr1, int vr2)
//{
//	if (vr1 == vr2)
//	{
//		spill_pr(x64mc_alloced, vr1);
//		return;
//	}
//	spill_pr(x64mc_alloced, vr1);
//	spill_pr(x64mc_alloced, vr2);
//}
//
//void x64_pr_alloc_o0(Scope *scp)
//{
//	vr2pr.resize(vrm.id + 1);
//
//	for (auto &r : x64mc_schedued)
//	{
//		X64mc mc = r;
//		MachineCodeType ty = mc.mcty;
//
//		switch (ty)
//		{
//		case MC_LI:
//			mc.pr1 = get_pr__load_vr(x64mc_alloced, mc.s1, VR_USAGE_WRITE);
//			x64mc_alloced.push_back(mc);
//			spill_pr(x64mc_alloced, mc.s1);
//			break;
//
//		case MC_ASSIGN:
//			//			if (mc.s1 == mc.s2)
////			    break;
//			mc.pr1 = get_pr__load_vr(x64mc_alloced, mc.s1, VR_USAGE_WRITE);
//			mc.pr2 = get_pr__load_vr(x64mc_alloced, mc.s2, VR_USAGE_READ);
//			x64mc_alloced.push_back(mc);
//			spill_pr(x64mc_alloced, mc.s1, mc.s2);
//			break;
//
//		case MC_ADD:
//			case MC_SUB:
//			case MC_IMUL:
//			case MC_DIV:
//
//			mc.pr1 = get_pr__load_vr(x64mc_alloced, mc.s1, VR_USAGE_READ_WRITE);
//			mc.pr2 = get_pr__load_vr(x64mc_alloced, mc.s2, VR_USAGE_READ);
//			x64mc_alloced.push_back(mc);
//			spill_pr(x64mc_alloced, mc.s1, mc.s2);
//			break;
//
//		case MC_RET:
//			mc.pr1 = get_pr__load_vr(x64mc_alloced, mc.s1, VR_USAGE_READ);
//			x64mc_alloced.push_back(mc);
//			spill_pr(x64mc_alloced, mc.s1);
//			break;
//
//		default:
//			ERR("%d \n", ty);
//			break;
//		}
//	}
//
//	for (Scope *p : scp->clds)
//		x64_pr_alloc_o0(p);
//}
//
//int align16(int &n)
//{
//	int align = 16;
//	n = ((n / align) + (bool) (n % align)) * align;
//	return n;
//
////	return (n + 15) & ~15;
//}
//
//static void dump_scope(Scope *scp)
//{
////	LOG("scp %s", scp->name.c_str());
//	printf("scp %s:\n", scp->name.c_str());
//
//	dump_mc(x64mc_alloced);
//
//	for (Scope *p : scp->clds)
//		dump_scope(p);
//}
//static void dump_alloced_mc()
//{
//	printf("\n========== mc ==========\n");
//
//	printf("push rbp\n");
//	printf("mov rbp, rsp\n");
//	printf("sub rsp, %d\n\n", vrm.offset);
//
//	dump_scope(&file_scp);
//
//	printf("\nmov rsp, rbp \n");
//	printf("pop rbp \n");
//	printf("ret \n\n");
//}
//
//static void dump_asm_scope(Scope *scp, FILE *fp)
//{
//	for (auto &mc : x64mc_alloced)
//	{
//		MachineCodeType ty = mc.mcty;
//
//		switch (ty)
//		{
//		case MC_LD:
//			fprintf(fp, "%s %s, dword ptr [rbp - %d] \n\t", mc.asm_code.c_str(), pr_name[mc.pr1], mc.of1);
//			break;
//
//		case MC_ST:
//			fprintf(fp, "%s dword ptr [rbp - %d], %s \n\t", mc.asm_code.c_str(), mc.of1, pr_name[mc.pr1]);
//			break;
//
//		case MC_LI:
//			fprintf(fp, "%s %s, %d \n\t", mc.asm_code.c_str(), pr_name[mc.pr1], mc.const_num);
//			break;
//
//		case MC_ASSIGN:
//			case MC_ADD:
//			case MC_SUB:
//			case MC_IMUL:
//			fprintf(fp, "%s %s, %s \n\t", mc.asm_code.c_str(), pr_name[mc.pr1], pr_name[mc.pr2]);
//			break;
//
//		case MC_DIV:
//			fprintf(fp, "mov eax, %s\n\t", pr_name[mc.pr1]);
//
//			fprintf(fp, "cdq \n\t");
//			fprintf(fp, "idiv %s \n\t", pr_name[mc.pr2]);
//			fprintf(fp, "mov %s, eax \n\t", pr_name[mc.pr1]);
//			break;
//
//		case MC_RET:
//			fprintf(fp, "# ---------------- print return value ----------------\n\t");
//			fprintf(fp, "mov esi, %s		# 第 2 个参数：要打印的整数\n\t", pr_name[mc.pr1]);
//			fprintf(fp, "lea rdi, [rip + fmt]		# 第 1 个参数：格式化字符串地址\n\t");
//			fprintf(fp, "mov eax, 0		# x86-64 ABI 规定：变长参数调用前将 eax 清零\n\t");
//			fprintf(fp, "call printf@PLT \n\t");
//			fprintf(fp, "# ------------------------------------------\n\t");
//
//			fprintf(fp, "mov eax, %s \n\t", pr_name[mc.pr1]);
//			break;
//
//		default:
//			ERR("%d", ty);
//			break;
//		}
//	}
//
//	for (Scope *p : scp->clds)
//		dump_asm_scope(p, fp);
//}
//static void dump_asm()
//{
//	int rsp_of = vrm.offset;
//	align16(rsp_of);
//	//	printf("vreg.offset %d, rsp_of %d\n", vreg.offset, rsp_of);
//
//	FILE *fp = fopen("a.s", "w");
//	assert(fp);
//
//	fprintf(fp, "\n#========== asm ==========#\n");
//	fprintf(fp, ".intel_syntax noprefix \n");
//	fprintf(fp, ".extern printf \n");
//
//	fprintf(fp, ".section .rodata \n");
//	fprintf(fp, "fmt: \n\t");
//	fprintf(fp, ".string \"Result: %%d\\n\" \n\n");
//
//	fprintf(fp, ".section .text \n");
//	fprintf(fp, ".global main \n\n");
//
//	fprintf(fp, "main: \n\t");
//	fprintf(fp, "push rbp \n\t");
//	fprintf(fp, "mov rbp, rsp \n\t");
//	fprintf(fp, "sub rsp, %d \n\n\t", rsp_of);
//
//	dump_asm_scope(&file_scp, fp);
//
//	fprintf(fp, "mov rsp, rbp \n\t");
//	fprintf(fp, "pop rbp \n\t");
//	fprintf(fp, "ret \n\n");
//	fprintf(fp, ".section .note.GNU-stack,\"\",@progbits \n");
//
//	fclose(fp);
//}
//void x64_pr_alloc()
//{
//	x64_pr_alloc_o0(&file_scp);
//	dump_alloced_mc();
//
//	dump_asm();
//
////	system("gcc a.s -o a");
////	system("./a");
//}
