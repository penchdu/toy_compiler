

int test()
{
	int a0 = 1;
	int a1 = 2;
	int a2 = 3;
	int a3 = 4;

	int b0 = 11;
	int b1 = 12;
	int b2 = 13;
	int b3 = 14;

	int c0 = 21;
	int c1 = 22;
	int c2 = 23;
	int c3 = 24;

	int d0 = 31;
	int d1 = 32;
	int d2 = 33;
	int d3 = 34;

	int i0 = 3;
	int i1 = 2;
	int i2 = 2;
	int i3 = 2;

	while ((i0 = i0 - 1) + a0 - a0 + b0 - b0 > 0)
	{
		a0 = a0 + b1;
		a1 = a1 + b0;

		b0 = b0 + c1;
		b1 = b1 + c0;

		c0 = c0 + d1;
		c1 = c1 + d0;

		d0 = d0 + a1;
		d1 = d1 + a0;

		while ((i1 = i1 - 1) + c0 - c0 + d0 - d0 > 0)
		{
			b0 = b0 + a2;
			b1 = b1 + a3;

			a2 = a2 + c0;
			a3 = a3 + c1;

			c0 = c0 + d2;
			c1 = c1 + d3;

			d2 = d2 + b1;
			d3 = d3 + b0;

			while ((i2 = i2 - 1) + a2 - a2 + b2 - b2 > 0)
			{
				c0 = c0 + b2;
				c1 = c1 + b3;

				b2 = b2 + d0;
				b3 = b3 + d1;

				d0 = d0 + a2;
				d1 = d1 + a3;

				a2 = a2 + c2;
				a3 = a3 + c3;

				while ((i3 = i3 - 1) + c2 - c2 + d2 - d2 > 0)
				{
					d0 = d0 + c0;
					d1 = d1 + c1;

					c2 = c2 + a0;
					c3 = c3 + a1;

					a0 = a0 + d2;
					a1 = a1 + d3;

					b0 = b0 + c2;
					b1 = b1 + c3;
				}

				a2 = a2 + b0 + d1;
				a3 = a3 + b1 + d0;
			}

			b2 = b2 + c0 + a2;
			b3 = b3 + c1 + a3;
		}

		c2 = c2 + d0 + b0;
		c3 = c3 + d1 + b1;
	}

	return a0 + a1 + a2 + a3
	    + b0 + b1 + b2 + b3
	    + c0 + c1 + c2 + c3
	    + d0 + d1 + d2 + d3
	    + i0 + i1 + i2 + i3;
}



