/*
 * scope.h
 *
 *  Created on: 2026年9月21日
 *      Author: x
 */

#ifndef BASIC_BLOCK_H_
#define BASIC_BLOCK_H_

#include "frontend.h"
#include "x64_back_end.h"


struct BasicBlock {
	Scope *entry_label = 0;
	vector<X64mc> x64mc;
	vector<X64mc> x64mc_schedu;
	vector<int> vrids;

//	Scope *exit_jmp = 0;
	Scope *jmp_to = 0;
	MachineCodeStamp jmp_mc_stamp = MC_INVALID;

	//
	vector<X64mc> x64mc_alloc_o0;
	vector<X64mc> x64mc_alloc_wave;
//	float max_wave_value = 0;
};

struct Wave {
	int vr;
	vector<float> score;
	vector<int> insts;		// >=0, <= 3
};
extern vector<Wave> vrwave;

void dump_mc(Scope *scp, string mc_list_name);
void gen_wave(BasicBlock *bb);
void dump_wave(BasicBlock *bb);
void dump_wave_gnuplot(Wave &w, int vr);
//void dump_asm();

#endif /* BASIC_BLOCK_H_ */
