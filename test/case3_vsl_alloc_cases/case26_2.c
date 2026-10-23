int test(int dummy)
{
	int a = 3;
	int b = 6;

	int r = 3;
	while ((r = r - 1) > 0)
	{
		a = (a + b + a);
	}

	return a + b;
}
