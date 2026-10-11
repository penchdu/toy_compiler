int test()
{
	int i = 0;
	int a = 3;
	int b = 4;
	int c = 5;
	int d = 6;
	int e = 7;
	int f = 8;
	int g = 9;
	while (i < 5)
	{
		if (i < 2)
		{
			a = a + b;
			c = c + a;
		}
		else
		{
			if (i < 4)
			{
				b = b + d;
				e = e + b;
			}
			else
			{
				d = d + f;
				g = g + d;
			}
		}
		c = c + 1;
		f = f + c;
		i = i + 1;
	}
	if (a > e)
	{
		g = g + a;
	}
	else
	{
		g = g + e;
	}
	return a + b + c + d + e + f + g + i;
}
