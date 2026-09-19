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
//	if (1)
//	{
//	}
//
//	if (1)
//		if (1)
//		{
//			1;
//		}
//		else if (1)
//			2;
//		else if (2)
//			3;
//		else
//			3;
//	else
//		(1);
//
//	if (3)
//		if (4)
//			4;
//
//	printf("\n");
//
//	if (printf("1\n"))
//		if (printf("2\n"))
//			if (printf("3\n"))
//				if (printf("4\n"))
//					4;
//				else if (printf("5\n"))
//					if (printf("6\n"))
//						6;
//					else
//						printf("7\n");
//
//	int a = 0;
//	a = (a + 3);
//	for(a;;)
//			a;;
//	;3;;

	fclose(fp);
	return 0;
}

