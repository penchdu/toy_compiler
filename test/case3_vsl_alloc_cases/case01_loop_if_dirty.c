int test(int dummy)
{
	int i = 0;
	int a = 2;
	int b = 5;
	int c = 7;
	int d = 11;
	while (i < 6)
	{
		if (i < 3)
		{
			a = a + b;
			b = b + 1;
		}
		else
		{
			c = c + d;
			d = d - 1;
		}
		i = i + 1;
	}
	return a + b + c + d + i;
}
