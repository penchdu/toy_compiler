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
	int i1 = 1;
	int i2 = 3;
	int i3 = 1;

	while ((i0 = i0 - 1)
		+ a0 - a0
		+ b0 - b0 > 0)
	{
		a0 = a0 + b0;
		a1 = a1 + b1;

		b0 = b0 + c0;
		b1 = b1 + c1;

		c0 = c0 + d0;
		c1 = c1 + d1;

		/*
		 * 故意让这个 while 一次都不进。
		 * 每次 outer iteration 都重新设成 1，
		 * condition 执行后直接变成 0。
		 */
		i1 = 1;

		while ((i1 = i1 - 1)
			+ a1 - a1
			+ c0 - c0 > 0)
		{
			a2 = a2 + d2;
			b2 = b2 + c2;
		}

		/*
		 * zero-iteration while 结束后，
		 * 继续修改长生命周期变量。
		 */
		d0 = d0 + a0;
		d1 = d1 + b1;

		/*
		 * 这个 while 每次 outer iteration 执行两次。
		 */
		i2 = 3;

		while ((i2 = i2 - 1)
			+ d0 - d0
			+ d1 - d1 > 0)
		{
			b2 = b2 + a2;
			b3 = b3 + a3;

			c2 = c2 + b2;
			c3 = c3 + b3;

			d2 = d2 + c2;
			d3 = d3 + c3;

			/*
			 * 再嵌一个必定 0 次执行的 while。
			 */
			i3 = 1;

			while ((i3 = i3 - 1)
				+ b2 - b2 > 0)
			{
				a2 = a2 + d2;
			}
		}
	}

	return a0 + a1 + a2 + a3
		+ b0 + b1 + b2 + b3
		+ c0 + c1 + c2 + c3
		+ d0 + d1 + d2 + d3
		+ i0 + i1 + i2 + i3;
}
