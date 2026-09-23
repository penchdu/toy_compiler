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
	vector<Tac> tacs;
	vector<X64mc> x64mc;
	vector<X64mc> x64mc_schedu;
	vector<X64mc> x64mc_alloc;

	Scope *exit_jmp = 0;
	MachineCodeStamp mc_jmp = MC_INVALID;
};

extern vector<BasicBlock> basic_blocks;

#endif /* BASIC_BLOCK_H_ */
