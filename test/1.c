int test(int dummy)
{
	int a = 3;

	int r = 3;
	while ((r = r - 1) > 0)
	{
		a = a + 6 + a;
	}

	return a;
}
