
int test()
{
	int a0 = 1;
	int a1 = 2;
	int a2 = 3;
	int a3 = 4;
	int b0 = 5;
	int b1 = 6;
	int b2 = 7;
	int b3 = 8;
	int c0 = 9;
	int c1 = 10;
	int c2 = 11;
	int c3 = 12;
	int d0 = 13;
	int d1 = 14;
	int d2 = 15;
	int d3 = 16;

	int i0 = 2;

	while (i0 > 0)
	{
		a0 = a0 + b0;
		a1 = a1 + c0;
		a2 = a2 + d0;
		a3 = a3 + b1;

		int i1 = 2;

		while (i1 > 0)
		{
			b0 = b0 + a1;
			b1 = b1 + c1;
			b2 = b2 + d1;
			b3 = b3 + a2;

			int i2 = 2;

			while (i2 > 0)
			{
				c0 = c0 + b2;
				c1 = c1 + d2;
				c2 = c2 + a3;
				c3 = c3 + b3;

				int i3 = 2;

				while (i3 > 0)
				{
					d0 = d0 + c0;
					d1 = d1 + a0;
					d2 = d2 + b1;
					d3 = d3 + c2;

					a0 = a0 + d3;
					b0 = b0 + a2;
					c0 = c0 + b2;

					i3 = i3 - 1;
				}

				c1 = c1 + d0;
				d1 = d1 + a1;
				b1 = b1 + c2;

				i2 = i2 - 1;
			}

			b2 = b2 + c3;
			c2 = c2 + d2;
			d2 = d2 + a2;

			i1 = i1 - 1;
		}

		a1 = a1 + b3;
		b3 = b3 + c1;
		c3 = c3 + d1;
		d3 = d3 + a3;

		i0 = i0 - 1;
	}

	return a0 + a1 + a2 + a3
	    + b0 + b1 + b2 + b3
	    + c0 + c1 + c2 + c3
	    + d0 + d1 + d2 + d3;
}
