int test(int n)
{
	int a = 1;
	int b = 2;
	int c = 3;

	int i0 = n * 250000;
	while ((i0 = i0 - 1) > 0)
	{
		a = b + c;
		b = c + a;
		c = a + b;
	}

	return a + b + c;
}
