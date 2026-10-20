/*
 */

#include <stdio.h>

#include "./test/1.c"

int print_test()
{
	int r = test(2000);
	printf("print_test: %d\n", r);
	return 0;
}
