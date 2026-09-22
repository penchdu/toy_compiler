/*
 * mc_x86_64.h
 *
 *  Created on: 2026年9月11日
 *      Author: x
 */

#ifndef X64_BACK_END_H_
#define X64_BACK_END_H_

#include "frontend.h"


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
		printf("\n");
#endif

//#define MC_LIST(X)	\
//	X(OP_ASSIGN,  "assign") \

enum MachineCodeType{
	MC_LI,	// reg-num

	MC_LD,	// reg <- ptr
	MC_ST,	// reg -> ptr

	MC_ASSIGN,	// reg-reg
	MC_ADD,	// reg-reg
	MC_SUB,
	MC_IMUL,
	MC_DIV,

	MC_CMP_LT,
	MC_CMP_LE,
	MC_CMP_E,
	MC_CMP_GE,
	MC_CMP_GT,
	MC_CMP_NE,

	MC_LOGIC_AND,

	MC_RET,	// reg-reg
	MC_INVALID,
};
struct McInfo{
	int mc_latency = -1;
	string mc_code;	// just for print
};
extern const McInfo mc_info[];

struct X64mc{
	X64mc(MachineCodeType ty, int three_addr_code_dst, int three_addr_code_s2)
	{
		mcty = ty;
		s1 = three_addr_code_dst;
		s2 = three_addr_code_s2;

		of1 = vrm.get_vr_off(s1);
		of2 = vrm.get_vr_off(s2);

		asm_code = mc_info[ty].mc_code;
		latency = mc_info[ty].mc_latency;
	}
	X64mc(MachineCodeType ty, int three_addr_code_s1)
	{
		mcty = ty;
		s1 = three_addr_code_s1;
		of1 = vrm.get_vr_off(three_addr_code_s1);

		asm_code = mc_info[ty].mc_code;
		latency = mc_info[ty].mc_latency;
	}
	X64mc(){
		mcty = MC_INVALID;
	}

	MachineCodeType mcty = MC_INVALID;

	int s1 = -1;
	int s2 = -1;

	int of1 = -1;
	int of2 = -1;

	// alloced for vr
	int pr1 = -1;	//todo rename
	int pr2 = -1;

	int const_num = 0;

	// put these here to easy dump
	string asm_code;
	string ori_sem;
	int latency = -1;
	int chain_latency = -1;
	int start_cycle = -1;
};

struct McDepend
{
	vector<int> mcs;
	int edges = 0;
};

extern vector<X64mc> x64mc;
extern vector<X64mc> x64mc_schedued;
extern vector<X64mc> x64mc_alloced;
extern vector<McDepend> mcs_pred;
extern vector<McDepend> mcs_succ;

void dump_mc(vector<X64mc> &v);
void gen_machine_code();
void mc_schedule();
void x64_pr_alloc();

#endif /* X64_BACK_END_H_ */
