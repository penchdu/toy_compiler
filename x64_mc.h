/*
 * mc_x86_64.h
 *
 *  Created on: 2026年9月11日
 *      Author: x
 */

#ifndef X64_MC_H_
#define X64_MC_H_

#include "h.h"

/*
 *
st[n] = dword ptr [n]

push rbp
mov rbp, rsp
sub rsp N			sub rsp, <num>

sem_const_num,
					load %s1, st[s1]
					mov %dst, <num>

sem_var (load),		mov %dst, st[dst]
sem_var (store),	mov st[dst], %dst

op_assign,
%dst = %s1			load %s1, st[s1]
					mov %dst, %s1
					store st[dst], %dst


op_+-*,
%dst = %s1 + %s2	load %s1, st[s1]
					load %s2, st[s2]
					mov %dst, %s1
					add %dst, %s2
					store st[dst], %dst

op_+-*,
%dst = %s1 + %s2	mov %dst, st[s1]
					mov %s2, st[s2]
					add %dst, %s2
					store st[dst], %dst

op_+-*,
%dst = %s1 + %s2	mov %dst, st[s1]
					add %dst, st[s2]
					store st[dst], %dst

op_+-*,
%dst = %s1 + %s2	mov %dst, %s1
					add %dst, %s2


op_div,
%dst = %s1 / %s2
					load %s1, st[s1]
					mov eax, %s1
					cdq
					load %s2, st[s2]
					idiv %s2
					mov %dst, eax
					store st[dst], %dst

sem_return
					load %dst, st[dst]
					mov eax, %dst
					mov rsp, rbp
					pop rbp
					ret

 */


enum Machine_code_type{
//	push_rbp,
//	mov_rbp_rsp,

	mc_li,
	mc_ld,
	mc_st,

//	mc_assign,
	mc_mov,

	mc_add,
	mc_sub,
	mc_imul,

	mc_div,
//	mc_div_mov_eax_s1_and_cdq,
//	mc_idiv,
//	mc_div_mov_s1_eax,

//	mc_push,
//	mc_pop,
//	mc_jmp,

//	mov_rsp_rbp,
//	pop_rbp,
	mc_ret,

	mc_invalid,
};
struct Mc_info{
	int mc_latency;
	string mc_code;
};
static const Mc_info mc_info[] = {
		[mc_li] = {1, "mov"},
		[mc_ld] = {1, "mov"},
		[mc_st] = {1, "mov"},

		[mc_mov] = {1, "mov"},

		[mc_add] = {1, "add"},
		[mc_sub] = {1, "sub"},
		[mc_imul] = {3, "imul"},

		[mc_div] = {10, "div"},
//		[mc_div_mov_eax_s1_and_cdq] = {1, "mc_div_mov_eax_s1_and_cdq"},
//		[mc_idiv] = {10, "idiv"},
//		[mc_div_mov_s1_eax] = {1, "move"},
};

struct X64_mc{
	X64_mc(Machine_code_type ty, int _s1, int _s2)
	{
		mcty = ty;
		s1 = _s1;
		s2 = _s2;

		of1 = vreg.get_vr_off(s1);
		of2 = vreg.get_vr_off(s2);
	}
	X64_mc(Machine_code_type ty, int _s1)
	{
		mcty = ty;
		s1 = _s1;
		of1 = vreg.get_vr_off(_s1);

	}
	X64_mc(){
		mcty = mc_invalid;
	}
	Machine_code_type mcty = mc_invalid;
	int latency = 0;

	int s1 = -1;
	int s2 = -1;

	int of1 = -1;
	int of2 = -1;

	int const_num = 0;
//	string mc_code;
	string ori_sem;
};

int get_slot(int len = 4);

#endif /* X64_MC_H_ */
