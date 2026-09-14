/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"
#include "x64_machine_code.h"

extern vector<X64_mc> x64mc_scheded;
vector<X64_mc> x64mc_alloced;

/*
 * struct X64_pr{
	X64_pr_ id = x64_pr_max;
	char *pr_name = "";

	int vr = -1;
	bool free = 1;
};
vector<X64_pr> x64_pr_state =
		{
				{ r10d, "r10d", 1 },
				{ r11d, "r11d", 1 },
				{ r12d, "r12d", 1 },
		};
 */
enum X64_pr
{
	r10d,
	r11d,
	r12d,
	r13d,
	r14d,
	r15d,

//	eax,

	x64_pr_max,
};
vector<int> x64_pr_state(x64_pr_max, 1);

static const char *pr_name[] =
		{
				[r10d] = "r10d",
				[r11d] = "r11d",
				[r12d] = "r12d",
				[r13d] = "r13d",
				[r14d] = "r14d",
				[r15d] = "r15d",	

//				[eax] = "eax",
		};

enum Vr_useage
{
	vr_useage_read = 1 << 0,
	vr_useage_write = 1 << 1,
	vr_useage_read_write = vr_useage_read | vr_useage_write,

	vr_useage_invalid = 0,
};
struct Vr_pr
{
	X64_pr pr = x64_pr_max;
	int u = vr_useage_invalid;
};
vector<Vr_pr> vr_pr;

static X64_pr get_pr()
{
	for (int i = r10d; i < x64_pr_max; i++)
	{
		if (x64_pr_state[i] == 1)
		{
			x64_pr_state[i] = 0;
			return (X64_pr)i;
		}
	}

	ERR("-O0");
	return x64_pr_max;
}
X64_pr get_pr__load_vr(int vr, int u)
{
	/*
	 * todo
	 * mc_assign %1, %1
	 * s1 == s2
	 */
	bool need_load = 0;
	if (vr_pr[vr].pr == x64_pr_max)
	{
		vr_pr[vr].pr = get_pr();
		vr_pr[vr].u = u;
		need_load = (u & vr_useage_read);
	}
	else if (!(vr_pr[vr].u & vr_useage_read)
			&& (u & vr_useage_read))
	{
		need_load = 1;
	}
	vr_pr[vr].u |= u;

	if (need_load)
	{
		X64_mc mc(mc_ld, vr);
		mc.pr1 = vr_pr[vr].pr;
		mc.ori_sem = "alloc";
		x64mc_alloced.push_back(mc);
	}

	return vr_pr[vr].pr;
}
//X64_pr get_2_pr__load_2_vr(int vr1, Vr_useage u1, int vr2, Vr_useage u2)
//{
//	assert(u1 & vr_useage_write);
//	assert(u2 & vr_useage_read);
//	if(vr1 == vr2)
//	{
//		return get_pr__load_vr(vr1, (u1 | u2));
//	}
//	return x64_pr_max;
//}
void spill_pr(int vr)
{
	X64_pr pr = vr_pr[vr].pr;
	int u = vr_pr[vr].u;

	assert(pr != x64_pr_max);
	assert(u != vr_useage_invalid);

//	mov %dst, dword ptr [n]
//	int off = vreg.get_vr_off(vr);

	if (u & vr_useage_write)
	{
		X64_mc mc(mc_st, vr);
		mc.pr1 = (int)pr;
		mc.ori_sem = "spill";
		x64mc_alloced.push_back(mc);
	}

	vr_pr[vr].pr = x64_pr_max;
	vr_pr[vr].u = vr_useage_invalid;

	assert(x64_pr_state[pr] == 0);
	x64_pr_state[pr] = 1;
}
void spill_pr(int vr1, int vr2)
{
	if(vr1 == vr2){
		spill_pr(vr1);
		return;
	}
	spill_pr(vr1);
	spill_pr(vr2);
}
void x64_pr_alloc_O0()
{
	vr_pr.resize(vreg.id + 1);

	for (auto &r : x64mc_scheded)
	{
		X64_mc mc = r;
		Machine_code_type ty = mc.mcty;

		switch (ty)
		{
		case mc_li:
			mc.pr1 = get_pr__load_vr(mc.s1, vr_useage_write);
			x64mc_alloced.push_back(mc);
			spill_pr(mc.s1);
			break;

		case mc_assign:
//			if (mc.s1 == mc.s2)
//			    break;
			mc.pr1 = get_pr__load_vr(mc.s1, vr_useage_write);
			mc.pr2 = get_pr__load_vr(mc.s2, vr_useage_read);
			x64mc_alloced.push_back(mc);
			spill_pr(mc.s1, mc.s2);
			break;

		case mc_add:
			case mc_sub:
			case mc_imul:

			mc.pr1 = get_pr__load_vr(mc.s1, vr_useage_read_write);
			mc.pr2 = get_pr__load_vr(mc.s2, vr_useage_read);
			x64mc_alloced.push_back(mc);
			spill_pr(mc.s1, mc.s2);
			break;

		case mc_div:
			mc.pr1 = get_pr__load_vr(mc.s1, vr_useage_read_write);
			mc.pr2 = get_pr__load_vr(mc.s2, vr_useage_read);
			x64mc_alloced.push_back(mc);
			spill_pr(mc.s1, mc.s2);
			break;

		case mc_ret:
			mc.pr1 = get_pr__load_vr(mc.s1, vr_useage_read);
			x64mc_alloced.push_back(mc);
			spill_pr(mc.s1);
			break;

		default:
			ERR("%d \n", ty);
			break;
		}
	}
}

int align16(int &n)
{
	int align = 16;
	n = ((n / align) + (bool)(n % align)) * align;
	return n;

//	return (n + 15) & ~15;
}

void dump_asm(vector<X64_mc> &v)
{
	FILE *fp = fopen("a.s", "w");
	assert(fp);

//	align16(vreg.offset);

	fprintf(fp, "\n#========== asm ==========\n");
	fprintf(fp, ".intel_syntax noprefix\n");
	fprintf(fp, ".extern printf \n");
	fprintf(fp, ".section .rodata \n");
	fprintf(fp, "fmt: \n\t");
	fprintf(fp, ".string \"Result: %%d\\n\"        \n\n");

	fprintf(fp, ".section .text\n");
	fprintf(fp, ".global main\n\n");
	fprintf(fp, "main: \n\t");

	fprintf(fp, "push rbp\n\t");	// rsp -= 8
	fprintf(fp, "mov rbp, rsp\n\t");	// rbp = rsp  (same -= 8)
//	fprintf(fp, "sub rsp, %d\n\n\t", 8);	// rsp -= 16, align 16

	int rsp_of = vreg.offset;
	align16(rsp_of);
	printf("vreg.offset %d, rsp_of %d\n", vreg.offset, rsp_of);

	fprintf(fp, "sub rsp, %d\n\n\t", rsp_of);

	for (auto &mc : v)
	{
		Machine_code_type ty = mc.mcty;

		switch (ty)
		{
		case mc_ld:
			fprintf(fp, "%s %s, dword ptr [rbp - %d]\n\t",mc.asm_code.c_str(), pr_name[mc.pr1], mc.of1);
			PRINT_MORE
			break;

		case mc_st:
			fprintf(fp, "%s dword ptr [rbp - %d], %s\n\t", mc.asm_code.c_str(), mc.of1, pr_name[mc.pr1]);
			PRINT_MORE
			break;

		case mc_li:
			fprintf(fp, "%s %s, %d\n\t", mc.asm_code.c_str(), pr_name[mc.pr1], mc.const_num);
			PRINT_MORE
			break;

		case mc_assign:
			case mc_add:
			case mc_sub:
			case mc_imul:
			fprintf(fp, "%s %s, %s\n\t", mc.asm_code.c_str(), pr_name[mc.pr1], pr_name[mc.pr2]);
			PRINT_MORE
			break;

		case mc_div:
			fprintf(fp, "mov eax, %s\n\t", pr_name[mc.pr1]);
			PRINT_MORE

			fprintf(fp, "cdq\n\t");
			fprintf(fp, "idiv %s\n\t", pr_name[mc.pr2]);
			fprintf(fp, "mov %s, eax\n\t", pr_name[mc.pr1]);
			break;

		case mc_ret:
			fprintf(fp, "# ---------------- 打印结果 ----------------\n\t");
			fprintf(fp, "mov esi, %s		# 第 2 个参数：要打印的整数 (放在 %%esi / %%rsi)\n\t", pr_name[mc.pr1]);
			fprintf(fp, "lea rdi, [rip + fmt]		# 第 1 个参数：格式化字符串地址 (放在 %%rdi)\n\t");
			fprintf(fp, "mov eax, 0		# x86-64 ABI 规定：变长参数调用前将 eax 清零\n\t");
			fprintf(fp, "call printf@PLT		# 调用 C 语言的 printf\n\t");
			fprintf(fp, "# ------------------------------------------\n\t");

			fprintf(fp, "mov eax, %s\n\t", pr_name[mc.pr1]);
			PRINT_MORE
			break;

		default:
			ERR("%d \n", ty);
			break;
		}
	}

	fprintf(fp, "mov rsp, rbp\n\t");
	fprintf(fp, "pop rbp\n\t");
	fprintf(fp, "ret \n\n");
	fprintf(fp, ".section .note.GNU-stack,\"\",@progbits\n");


	fclose(fp);
}

void x64_pr_alloc_and_dump_asm()
{
	x64_pr_alloc_O0();
//	dump_mc(x64mc_alloced);

	dump_asm(x64mc_alloced);

//	system("gcc a.s -o a");
//	system("./a");
}





