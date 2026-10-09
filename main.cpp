/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"

VirtualRegDeclareManager vr_declare_manager;
extern int print_test();

int main(int argc, char **argv)
{
	const char *file = 0;
	if(argc == 1)
		file = "./test/1.cpp";
	else
		file = argv[1];

	FILE *fp = fopen(file, "r");
	assert(fp);

	lexer(fp);
	parser();
	sem_analysis();
	gen_three_address_code();
	gen_machine_code();
	gen_liveness();
	mc_schedule();
	x64_reg_alloc();		// create a.s in current location


	if(argc == 1){
		system("gcc a0.s -o a0");
		system("./a0");
		system("gcc a1.s -o a1");
		system("./a1");

		print_test();
	}

	fclose(fp);
	return 0;
}
