int test()
{
	int a = 8;
	int b = 3;
	int c = 0;

	if (a > b)
	{
		int b = 10;
		c = a + b;

		if (c > 15)
		{
			int a = 2;
			c = c + a;
		}
		else
		{
			c = c - 1;
		}

		b = c - 5;
	}
	else
	{
		c = a - b;
	}

	{
		int a = 4;
		int c = 7;

		if (a < c)
		{
			int b = 2;
			c = c + b;

			if (c >= a)
				a = a + 3;
		}

		b = b + c;
	}

	if (c > a)
	{
		int c = 2;
		c = c + b;

		if (c < a)
			b = b + c;
		else
			b = b - c;

		if (b == 8)
			a = a + 10;
	}
	else
	{
		c = c + 100;
	}

	return a + b + c;
}