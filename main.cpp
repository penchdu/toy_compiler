/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"
#include "x64_back_end.h"

VirtualRegisterManager vrm;

void func()
{
}


int main(int argc, char **argv)
{
	FILE *fp = fopen("./t1", "r");
	assert(fp);

	lexer(fp);
	parser();
//	sem_analysis();
//	gen_three_address_code();
//	gen_machine_code();
//	mc_schedule();
//	x64_pr_alloc();		// create a.s in current location
//
//	system("gcc a.s -o a");
//	system("./a");

//	func();
//	int func = 0;
//	func;
//	if(1)
//		int func = 2;
//	else
//	{
//		int func;
//		if(1){
//
//		}
//	}
//
//	if(1)
//	{
//
//	}
//	int a = 0;
//	a = (a + 3);
//	for(a;;)
//			a;;
//	;3;;

	fclose(fp);
	return 0;
}

