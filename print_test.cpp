/*
 */

#include <stdio.h>

#include "./test/1.cpp"

int print_test()
{
	int r = test();
	printf("Result: %d\n", r);
	return 0;
}
