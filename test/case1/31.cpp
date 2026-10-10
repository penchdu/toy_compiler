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

	while ((i0 = i0 - 1)
		+ a0 - a0
		+ a1 - a1
		+ a2 - a2
		+ a3 - a3
		+ b0 - b0
		+ b1 - b1
		+ b2 - b2
		+ b3 - b3
		+ c0 - c0
		+ c1 - c1
		+ c2 - c2
		+ c3 - c3
		+ d0 - d0
		+ d1 - d1
		+ d2 - d2
		+ d3 - d3 > 0)
	{
		/*
		 * a0 / b0 都同时是 outer cond 的读取对象，
		 * 又在 body 中被修改。
		 */
		a0 = a0 + b0;
		b0 = b0 + c0;

		/*
		 * 再制造一批活跃值，增加 cond spill/reload 压力。
		 */
		a1 = a1 + d1;
		a2 = a2 + d2;
		a3 = a3 + d3;

		b1 = b1 + c1;
		b2 = b2 + c2;
		b3 = b3 + c3;

		i1 = 2;

		while ((i1 = i1 - 1)
			+ a0 - a0
			+ a1 - a1
			+ a2 - a2
			+ a3 - a3
			+ b0 - b0
			+ b1 - b1
			+ b2 - b2
			+ b3 - b3
			+ c0 - c0
			+ c1 - c1
			+ c2 - c2
			+ c3 - c3
			+ d0 - d0
			+ d1 - d1
			+ d2 - d2
			+ d3 - d3 > 0)
		{
			/*
			 * 再次修改 outer cond 里的 VR。
			 */
			a0 = a0 + d0;
			b0 = b0 + d0;

			c0 = c0 + a0;
			d0 = d0 + b0;

			i2 = 2;

			while ((i2 = i2 - 1)
				+ a0 - a0
				+ b0 - b0
				+ c0 - c0
				+ d0 - d0
				+ a1 - a1
				+ b1 - b1
				+ c1 - c1
				+ d1 - d1 > 0)
			{
				/*
				 * 最深层继续把 cond 相关 VR 改脏。
				 */
				a0 = a0 + c0;
				b0 = b0 + d0;

				c0 = c0 + a0;
				d0 = d0 + b0;

				a1 = a1 + c0;
				b1 = b1 + d0;
			}
		}
	}

	return a0 + a1 + a2 + a3
		+ b0 + b1 + b2 + b3
		+ c0 + c1 + c2 + c3
		+ d0 + d1 + d2 + d3
		+ i0 + i1 + i2;
}
