#include <stdio.h>

extern int test(int);

int main()
{
	int r = test(2000);
	printf("Result: %d\n", r);
	return r;
}
