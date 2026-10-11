int test()
{
	int i = 5;
	int a = 0;
	int b = 1;
	int c = 2;

	while ((i = i - 1) > 0)
	{
		a = a + i;

		if (i == 3)
		{
			b = b + a;
			continue;
			a = 100;
		}
		else
		{
			c = c + i;
		}

		if (i == 2)
		{
			b = b + 10;
			break;
			c = 200;
		}
		else
		{
			a = a + c;
		}

		c = c + 1;
	}

	return a + b + c + i;
}
