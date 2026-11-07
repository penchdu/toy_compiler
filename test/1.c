int test(int n)
{
	int a = 0;
	int b = 1;

	int i0 = n + 1;
	while ((i0 = i0 - 1) > 0)
	{
		a = a + b;
		b = b + a;

		int i1 = n + 2;
		while ((i1 = i1 - 1) > 0)
		{
			a = a + b;
			b = b + a;

			int i2 = n + 3;
			while ((i2 = i2 - 1) > 0)
			{
				a = b + a;
				b = a + b;

				int i3 = n + 4;
				while ((i3 = i3 - 1) > 0)
				{
					a = a + b;
					b = b + a;
				}
			}
		}
	}

	return a + b;
}
