/*
 * mc_x86_64.h
 *
 *  Created on: 2026年9月11日
 *      Author: x
 */

#ifndef X64_MACHINE_CODE_H_
#define X64_MACHINE_CODE_H_

#include "h.h"

/*
 *
st[n] = dword ptr [n]

push rbp
mov rbp, rsp
sub rsp, N


li
sem_const_num,
					load %s1, st[s1]
					mov %dst, <num>

load,
					mov %dst, st[dst]

store,
					mov st[dst], %dst

op_assign,
%dst = %s1
					mov %dst, %s1

op_+-*,
%dst = %s1 + %s2
					mov %dst, %s1
					add %dst, %s2

op_+-*,
%dst = %s1 + %s2	mov %dst, st[s1]
					add %dst, st[s2]

op_div,
%dst = %s1 / %s2
					mov %dst, %s1
					mov eax, %dst
					cdq
					idiv %s2
					mov %dst, eax

sem_return
					mov eax, %dst
					mov rsp, rbp
					pop rbp
					ret
 */


#if 0
	#define PRINT_MORE	\
			printf("\t%s", mc.ori_sem.c_str());	\
			printf(", cyc %d", mc.start_cycle);	\
//			printf(", off %d\n", mc.of1);
#else
	#define PRINT_MORE	\
//		printf("\n\t");
#endif


enum Machine_code_type{
	mc_li,	// reg-num

	mc_ld,	// reg <- ptr
//	mc_spill,	// reg -> ptr, lost pr
	mc_st,	// reg -> ptr

	mc_assign,	// reg-reg
	mc_add,	// reg-reg
	mc_sub,
	mc_imul,
	mc_div,

	mc_ret,	// reg-reg
	mc_invalid,
};
struct Mc_info{
	int mc_latency = -1;
	string mc_code;	// just for print
};
static const Mc_info mc_info[mc_invalid] = {
		[mc_li] = {1, "mov"},
		[mc_ld] = {3, "mov"},
		[mc_st] = {3, "mov"},

		[mc_assign] = {1, "mov"},
		[mc_add] = {1, "add"},
		[mc_sub] = {1, "sub"},
		[mc_imul] = {3, "imul"},
		[mc_div] = {10, "div"},
		[mc_ret] = {1, "ret"},
};

struct X64_mc{
	X64_mc(Machine_code_type ty, int three_addr_code_dst, int three_addr_code_s2)
	{
		mcty = ty;
		s1 = three_addr_code_dst;
		s2 = three_addr_code_s2;

		of1 = vreg.get_vr_off(s1);
		of2 = vreg.get_vr_off(s2);

		asm_code = mc_info[ty].mc_code;
		latency = mc_info[ty].mc_latency;
	}
	X64_mc(Machine_code_type ty, int three_addr_code_s1)
	{
		mcty = ty;
		s1 = three_addr_code_s1;
		of1 = vreg.get_vr_off(three_addr_code_s1);

		asm_code = mc_info[ty].mc_code;
		latency = mc_info[ty].mc_latency;
	}
	X64_mc(){
		mcty = mc_invalid;
	}

	Machine_code_type mcty = mc_invalid;

	int s1 = -1;
	int s2 = -1;

	int of1 = -1;
	int of2 = -1;

	// alloced for vr
	int pr1 = -1;	//todo rename
	int pr2 = -1;

	int const_num = 0;

	// put them here to easy dump
	string asm_code;
	string ori_sem;
	int latency = -1;
	int chain_latency = -1;
	int start_cycle = -1;
};

struct Mc_dep
{
	vector<int> mcs;
	int edges = 0;
};

extern vector<X64_mc> x64mc;
extern vector<X64_mc> x64mc_scheded;
extern vector<X64_mc> x64mc_alloced;
extern vector<Mc_dep> mcs_pred;
extern vector<Mc_dep> mcs_succ;

int get_slot(int len = 4);
void dump_mc(vector<X64_mc> &v);
void mc_schedule();
void gen_x64_asm();

#endif /* X64_MACHINE_CODE_H_ */
