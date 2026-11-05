int test(int dummy)
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
	int c1 = 0;
	int c2 = 1;
	int c3 = 2;

	int d0 = 3;
	int d1 = 4;
	int d2 = 5;
	int d3 = 6;

	int i0 = 2;
	int i1 = 2;
	int i2 = 2;
	int i3 = 2;

	{
		a3 = a3 + c3;

		while ((i3 = i3 - 1) > 0)
		{
			c3 = c3 + 1;
		}
	}

	return a0 + a1 + a2 + a3
	    + c0 + c1 + c2 + c3
	    + d0 + d1 + d2 + d3
	    + i0 + i1 + i2 + i3;
}
