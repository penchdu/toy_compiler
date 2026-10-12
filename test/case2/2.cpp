int test()
{
	int a = 1;
	int b = 200;
	int sum = 0;
	int i = 0;

	if (a < b)
	{
		int a = 10;
		int e = 5;
		while (i < 120)
		{
			if (a < 50)
			{
				e = e + 6;
				break;

				e = e + 7;
			}
			if (b > 0)
			{
				i = i + 1;
				continue;

				i = i + 2;
			}
			sum = sum + a + e;
			i = i + 1;
		}
	}

	sum = sum + a;

	while (i < 100)
	{
		int e = 12;
		while (i < 10)
		{
			int a = 1;
			a = a + e;
			if (i > 5)
				break;
			i = i + a;
		}
		i = i + 1;
		sum = sum + i;
	}
	return sum;
}
