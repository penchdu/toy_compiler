int test(int dummy)
{
    int a = 0;
    int b = 1;
    int c = 2;

	if (1)      // 外层
		a = 1;
	else if (2)  // 内层 elif
		b = 2;
	else         // 内层 else
		c = 3;


	return a + b + c;
}