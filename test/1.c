int test(int n)
{
	int a = 1;
	int b = 2;
	int c = 3;
	int d = 4;
	int e = 5;

	int i0 = n * 340000;
	while ((i0 = i0 - 1) > 0)
	{
		a = b + c;
		b = c + d;
		c = d + e;
		d = e + a;
		e = a + b;
	}

	return a + b + c + d + e;
}
