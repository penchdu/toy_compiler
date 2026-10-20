/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"

VrDeclareRecord declare_record;
extern int print_test();

int main(int argc, char **argv)
{
	const char *file = 0;
	if (argc == 1)
		file = "./test/1.c";
	else
		file = argv[1];

	FILE *fp = fopen(file, "r");
	assert(fp);

	lexer(fp);
	parser();
	sem_analysis();
	gen_three_address_code();
	gen_machine_code();
	gen_use_cnt();
	mc_schedule();
	x64_reg_alloc();		// create a.s in current location

	if (argc == 1)
	{
		system("gcc main_wrapper.c a0.s -o a0 -O0");
		system("objdump -d -M intel a0 > a0_final.s");
		system("gcc main_wrapper.c a1.s -o a1 -O0");
		system("objdump -d -M intel a1 > a1_final.s");

		system("./a0");
		system("./a1");

		system("gcc -S ./test/1.c -o print_test.s");
		system("gcc main_wrapper.c print_test.s -o print_test");
		system("./print_test");
//		print_test();
	}

	fclose(fp);
	return 0;
}