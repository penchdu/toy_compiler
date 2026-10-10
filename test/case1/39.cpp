int test()
{
	int a0 = 1;
	int a1 = 2;
	int a2 = 3;
	int a3 = 4;
	int a4 = 5;
	int b0 = 6;
	int b1 = 7;
	int b2 = 8;
	int b3 = 9;
	int b4 = 10;
	int c0 = 11;
	int c1 = 12;
	int c2 = 13;
	int c3 = 14;
	int c4 = 15;
	int d0 = 16;
	int d1 = 17;
	int d2 = 18;
	int d3 = 19;
	int d4 = 20;

	int i0 = 2;

	while (i0 > 0)
	{
		a0 = a0 + b1;
		b1 = b1 + c2;
		c2 = c2 + d3;
		d3 = d3 + a4;

		a1 = a1 + b2;
		b2 = b2 + c3;
		c3 = c3 + d4;
		d4 = d4 + a0;

		int i1 = 2;

		while (i1 > 0)
		{
			a2 = a2 + c0;
			c0 = c0 + d1;
			d1 = d1 + b4;
			b4 = b4 + a3;

			a3 = a3 + c1;
			c1 = c1 + d2;
			d2 = d2 + b0;
			b0 = b0 + a2;

			int i2 = 2;

			while (i2 > 0)
			{
				a4 = a4 + d0;
				d0 = d0 + b1;
				b1 = b1 + c4;
				c4 = c4 + a1;

				b3 = b3 + c2;
				c2 = c2 + d3;
				d3 = d3 + a4;
				a1 = a1 + b3;

				int i3 = 2;

				while (i3 > 0)
				{
					/* 第一轮：制造一组寄存器占用 */
					a0 = a0 + c3;
					c3 = c3 + b2;
					b2 = b2 + d4;
					d4 = d4 + a0;

					/* 第二轮：马上换手 */
					a2 = a2 + d2;
					d2 = d2 + b4;
					b4 = b4 + c1;
					c1 = c1 + a2;

					/* 第三轮：重新抢之前的值 */
					a4 = a4 + b0;
					b0 = b0 + c0;
					c0 = c0 + d1;
					d1 = d1 + a4;

					int i4 = 2;

					while (i4 > 0)
					{
						/*
						 * 这里故意按照“旧 VR 被新 VR 顶掉，
						 * 新 VR 又立即回来”的顺序访问。
						 */
						a0 = a0 + d1;
						b1 = b1 + a2;
						c2 = c2 + a3;
						d3 = d3 + b4;

						a1 = a1 + c4;
						b2 = b2 + d0;
						c3 = c3 + a4;
						d4 = d4 + b0;

						a2 = a2 + c0;
						b3 = b3 + d2;
						c1 = c1 + a0;
						d1 = d1 + b1;

						/*
						 * 再把前面已经被频繁换手的 VR 拉回来。
						 */
						a3 = a3 + d3;
						b4 = b4 + c2;
						c0 = c0 + a1;
						d2 = d2 + b2;

						i4 = i4 - 1;
					}

					/*
					 * inner recover 后再立即使用这些变量，
					 * 强迫 loop-carried state 继续参与分配。
					 */
					a4 = a4 + d2;
					b0 = b0 + c0;
					c4 = c4 + b4;
					d0 = d0 + a3;

					a0 = a0 + d4;
					b1 = b1 + c3;
					c2 = c2 + d1;
					d3 = d3 + a2;

					i3 = i3 - 1;
				}

				/*
				 * unwind level 3
				 */
				a1 = a1 + b0;
				b2 = b2 + c1;
				c3 = c3 + d2;
				d4 = d4 + a4;

				a3 = a3 + b3;
				b4 = b4 + c4;
				c0 = c0 + d0;
				d1 = d1 + a3;

				i2 = i2 - 1;
			}

			/*
			 * unwind level 2
			 */
			a0 = a0 + b4;
			b1 = b1 + c0;
			c2 = c2 + d1;
			d3 = d3 + a2;

			a4 = a4 + b2;
			b0 = b0 + c3;
			c4 = c4 + d4;
			d0 = d0 + a1;

			i1 = i1 - 1;
		}

		/*
		 * unwind level 1
		 */
		a2 = a2 + c4;
		b3 = b3 + d0;
		c1 = c1 + a0;
		d2 = d2 + b1;

		a3 = a3 + c2;
		b4 = b4 + d3;
		c0 = c0 + a4;
		d1 = d1 + b0;

		i0 = i0 - 1;
	}

	return a0 + a1 + a2 + a3 + a4
	    + b0 + b1 + b2 + b3 + b4
	    + c0 + c1 + c2 + c3 + c4
	    + d0 + d1 + d2 + d3 + d4;
}
