

int test(int i0)
{
	if(i0 < 0)
		return 0;

	int a = 1;
	int b = 2;
	int c = 3;
	int d = 4;
	int e = 5;
	int f = 6;

//	int i0 = 20000;
	int v1 = 1;
	while ((i0 = i0 - v1) > 0)
	{
		a = b + c;
		b = c + d;
		c = d + e;
		d = e + f;
		e = f + a;
		f = a + b;

		int i1 = 20000;
		while ((i1 = i1 - v1) > 0)
		{
			a = c + f;
			b = d + a;
			c = e + b;
			d = f + c;
			e = a + d;
			f = b + e;
		}
	}

	return a + b + c + d + e + f;
}
