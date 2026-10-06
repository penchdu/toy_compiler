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


int main(int argc, char **argv)
{
	FILE *fp = fopen("./test/test.cpp", "r");
	assert(fp);

	lexer(fp);
	parser();
	sem_analysis();
	gen_three_address_code();
	gen_machine_code();
	gen_liveness();
	mc_schedule();
	x64_reg_alloc();		// create a.s in current location


	print__test();
	fclose(fp);
	return 0;
}
