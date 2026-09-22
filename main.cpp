/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"
#include "x64_back_end.h"
#include "scope.h"

VirtualRegisterManager vrm;

void func()
{
}

int main(int argc, char **argv)
{
	FILE *fp = fopen("./t1.txt", "r");
	assert(fp);

	lexer(fp);
	parser();
	sem_analysis();
	gen_three_address_code();
	gen_machine_code();
//	mc_schedule();
//	x64_pr_alloc();		// create a.s in current location
//
//	system("gcc a.s -o a");
//	system("./a");


	fclose(fp);
	return 0;
}
