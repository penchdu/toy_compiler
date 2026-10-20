int test(int dummy)
{
	int a = 1;
	int b = a + 10;
	int c = a + 100;
	int d = a + 1000;
	int e = a + 0;
	int f = 0;
	int sum = 0;
	int i = 0;

	while (i < 12)
	{
		if (a < b)
		{
			a = a + 3;
			e = e + 1;

			if (c < d)
			{
				c = c + 5;
				f = f + 1;
			}
			else
			{
				d = d - 2;
				f = f + 2;
			}
		}
		else
		{
			b = b - 2;
			e = e + 2;
			if (c > d)
			{
				c = c - 3;
				f = f + 3;
			}
			else
			{
				d = d + 4;
				f = f + 4;
			}
		}

		sum = sum + a + b + c + d + e + f;
		a = a + e;
		b = b + f;

		i = i + 1;
	}

	sum = sum + 1;
	return sum;
}