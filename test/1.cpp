int test()
{
	int a0 = 1;
	int a1 = 3;
	int a2 = 5;
	int a3 = 7;
	int b0 = 2;
	int b1 = 4;
	int b2 = 6;
	int b3 = 8;
	int c0 = 9;
	int c1 = 11;
	int c2 = 13;
	int c3 = 15;
	int d0 = 10;
	int d1 = 12;
	int d2 = 14;
	int d3 = 16;
	int skip = 1;
	int carry = 3;

	while ((skip = skip - 1) > 0)
	{
		if (carry > 0)
		{
			a0 = a0 + b0;
			b0 = b0 + c0;
		}
		else
		{
			c0 = c0 + d0;
			d0 = d0 + a0;
		}
	}

	a0 = (a0 + skip) / 2 + carry;
	c0 = (c0 + skip) / 2 + a0;

	int p = 6;

	while ((p = p - 1) > 0)
	{
		if (p > 2)
		{
			a1 = (a1 + b1) / 2 + p;
			c1 = (c1 + d1) / 2 + a1;
			int q = 5;
			while ((q = q - 1) > 0)
			{
				if (q > 2)
				{
					b2 = (b2 + c2) / 2 + q;
					d2 = (d2 + a2) / 2 + p;
				}
				else
				{
					c2 = (c2 + a3) / 2 + q;
					b1 = (b1 + d3) / 2 + p;
				}
			}
		}
		else
		{
			a3 = (a3 + c3) / 2 + p;
			b3 = (b3 + d3) / 2 + p;
			if (a3 > b3)
			{
				c3 = (c3 + a3) / 2 + carry;
				d3 = (d3 + b3) / 2 + p;
			}
			else
			{
				c2 = (c2 + b3) / 2 + carry;
				d2 = (d2 + a3) / 2 + p;
			}
		}

		carry = carry + 1;
	}

	int s = 5;
	while ((s = s - 1) > 0)
	{
		if (s > 2)
		{
			d0 = (d0 + c3) / 2 + s;
			a0 = (a0 + b3) / 2 + s;
		}
		else
		{
			d1 = (d1 + c2) / 2 + s;
			a1 = (a1 + b2) / 2 + s;
		}
	}

	return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
	    + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3 + skip + p + s + carry;
}
