/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"
#include "test/print_test.cpp"
#include "x64_back_end.h"

VirtualRegDeclareManager vr_declare_manager;

#define auto_test 0

int main(int argc, char **argv)
{
#if auto_test
	if(argc != 2)
		ERR("argc != 2");
	FILE *fp = fopen(argv[1], "r");
#else
	FILE *fp = fopen("./test/test.cpp", "r");
#endif

	assert(fp);

	lexer(fp);
	parser();
	sem_analysis();
	gen_three_address_code();
	gen_machine_code();
	gen_liveness();
	mc_schedule();
	x64_reg_alloc();		// create a.s in current location

#if !auto_test

	system("gcc a0.s -o a0");
	system("./a0");
	system("gcc a1.s -o a1");
	system("./a1");
	print__test();

#endif

	fclose(fp);
	return 0;
}
