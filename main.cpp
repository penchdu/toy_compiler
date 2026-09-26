/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"
//#include "test/test.cpp"

VirtualRegManager vregm;


int main(int argc, char **argv)
{
	FILE *fp = fopen("./t1.txt", "r");
	assert(fp);

	lexer(fp);
	parser();
	sem_analysis();
	gen_three_address_code();
	gen_machine_code();
	mc_schedule();
	x64_reg_alloc();		// create a.s in current location


//	test();
	fclose(fp);
	return 0;
}
