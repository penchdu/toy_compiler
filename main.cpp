/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"
#include "x64_back_end.h"

VrDeclareRecord declare_record;

int main(int argc, char **argv)
{
	const char *file = 0;
	if (argc == 1)
		file = "./test/1.c";
	else
		file = argv[1];

	FILE *fp = fopen(file, "r");
	assert(fp);
	setbuf((FILE *)stdout, 0);

	lexer(fp);
	parser();
	sem_analysis();
	gen_tac_O0();
	gen_mc_O0();
	x64_ra_O0();		// create a.s in current location

	gen_tac_O1();
	gen_mc_O1();
	mc_schedule();
	x64_ra_O1();

	if (argc == 1)
	{
		system("gcc  print_test.c  a0.s  -o  a0  -O0");
		system("gcc  print_test.c  a1.s  -o  a1  -O0");
		system("./a0");
		system("./a1");

		system("gcc  -S  ./test/1.c  -o  print_test.s");
		system("gcc  print_test.c  print_test.s  -o  print_test");
		system("./print_test");
	}

	fclose(fp);
	return 0;
}
