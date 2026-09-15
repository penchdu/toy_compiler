/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"
#include "x64_back_end.h"

Scope file_scope;
Scope *scope;
int scope_id = 0;
VReg vreg;

int main(int argc, char **argv)
{
	FILE *fp = fopen("./t1", "r");
	assert(fp);

	scope = &file_scope;
	scope->id = scope_id++;
	scope->name = "b" + std::to_string(scope->id);
	scope->is_virtual_scope = false;
	scope->parent = 0;

	lexer(fp);
	parser();
	sem_analysis();
	gen_three_address_code();
	gen_mc();
	mc_schedule();
	x64_pr_alloc();		// create a.s in current location

	system("gcc a.s -o a");
	system("./a");




	lbb:
		goto lbc;
		system("./a");

		{
			lba:
//				goto lbb;
			;
		}

		int c;
		lbc:
		 c = 1;










	fclose(fp);
	return 0;
}

