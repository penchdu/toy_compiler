
int test(int n)
{
	int s = 1;
	int a0 = s + 0;
	int a1 = s + 1;
	int a2 = s + 2;
	int a3 = s + 3;
	int b0 = s + 4;
	int b1 = s + 5;
	int b2 = s + 6;
	int b3 = s + 7;
	int c0 = s + 8;
	int c1 = s + 9;
	int c2 = s + 10;
	int c3 = s + 11;
	int d0 = s + 12;
	int d1 = s + 13;
	int d2 = s + 14;
	int d3 = s + 15;
	int e0 = s + 16;
	int e1 = s + 17;
	int e2 = s + 18;
	int e3 = s + 19;

	int k = n * 80;
	int v1 = s;
	int v2 = s + s;
	int v3 = s + s + s;
	int v4 = s + s + s + s;

	int i0 = v4;
	i0 = i0 * k;
	while ((i0 = i0 - v1) > 0)
	{
		a0 = (a0 + e3) + v2 + i0;
		a1 = (a1 + e2) + v2 + i0;
		b0 = (b0 + e1) + v2 + v1;
		b1 = (b1 + e0) + v2 + v2;

		int i1 = v4;
		i1 = i1 * k;
		while ((i1 = i1 - v1) > 0)
		{
			a2 = (a2 + d3) + v2 + i1;
			a3 = (a3 + d2) + v2 + i1;
			b2 = (b2 + c3) + v2 + i0;
			b3 = (b3 + c2) + v2 + i1;

			int i2 = v4;
			i2 = i2 * k;
			while ((i2 = i2 - v1) > 0)
			{
				c0 = (c0 + a3) + v2 + i2;
				c1 = (c1 + a2) + v2 + i1;
				d0 = (d0 + b3) + v2 + i2;
				d1 = (d1 + b2) + v2 + i0;

				int i3 = v3;
				i3 = i3 * k;
				while ((i3 = i3 - v1) > 0)
				{
					e0 = (e0 + c0) + v2 + i3;
					e1 = (e1 + c1) + v2 + i2;
					e2 = (e2 + d0) + v2 + i3;
					e3 = (e3 + d1) + v2 + i1;
					a0 = (a0 + e0) + v2 + i3;
					b0 = (b0 + e1) + v2 + i3;
					c2 = (c2 + a0) + v2 + i2;
					d2 = (d2 + b0) + v2 + i2;
					i3 = i3 - v1;
				}
				i2 = i2 - v1;
				c3 = (c3 + e3) + v2 + i2;
				d3 = (d3 + e2) + v2 + i2;
			}
			i1 = i1 - v1;
			a1 = (a1 + c3) + v2 + i1;
			b1 = (b1 + d3) + v2 + i1;
		}
		i0 = i0 - v1;
		a2 = (a2 + b1) + v2 + i0;
		e0 = (e0 + d1) + v2 + i0;
	}

	return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
	    + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3
	    + e0 + e1 + e2 + e3 + i0;
}
