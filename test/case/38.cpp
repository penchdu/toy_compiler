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
		/*
		 * 这一段让部分 VR 在 cond/body 前后保持稳定，
		 * 部分 VR 后面马上参与高压力竞争。
		 */
		a0 = a0 + b0;
		a1 = a1 + c0;
		a2 = a2 + d0;

		int i1 = 2;

		while (i1 > 0)
		{
			/*
			 * 第一组高压力
			 */
			b0 = b0 + a1;
			b1 = b1 + c1;
			b2 = b2 + d1;
			b3 = b3 + a2;

			/*
			 * 第二组高压力
			 */
			c0 = c0 + b2;
			c1 = c1 + d2;
			c2 = c2 + a3;
			c3 = c3 + b3;

			int i2 = 2;

			while (i2 > 0)
			{
				/*
				 * 这里保留 a0/a1/i0/i1，
				 * 同时让另外一批 VR 大量换手。
				 */
				d0 = d0 + c0;
				d1 = d1 + a0;
				d2 = d2 + b1;
				d3 = d3 + c2;

				a3 = a3 + d3;
				b0 = b0 + a2;
				c0 = c0 + b2;

				int i3 = 2;

				while (i3 > 0)
				{
					/*
					 * 第一轮：把 16 个长期变量尽量卷进来
					 */
					a0 = a0 + d1;
					a1 = a1 + c2;
					a2 = a2 + b3;
					a3 = a3 + d0;

					b0 = b0 + c1;
					b1 = b1 + d2;
					b2 = b2 + a0;
					b3 = b3 + c0;

					c0 = c0 + d3;
					c1 = c1 + a2;
					c2 = c2 + b1;
					c3 = c3 + d1;

					d0 = d0 + a1;
					d1 = d1 + b2;
					d2 = d2 + c3;
					d3 = d3 + a3;

					/*
					 * 第二轮：马上重新排列依赖
					 */
					a1 = a1 + b0;
					b2 = b2 + c1;
					c3 = c3 + d2;
					d0 = d0 + a0;

					/*
					 * 第三轮：再次拉回前面刚被使用过的 VR
					 */
					a0 = a0 + c3;
					b1 = b1 + d0;
					c2 = c2 + a1;
					d3 = d3 + b2;

					i3 = i3 - 1;
				}

				/*
				 * 从 deepest recover 返回以后，
				 * 立即只使用其中一部分值。
				 *
				 * 这会让一部分寄存器保持稳定，
				 * 另一部分重新进入竞争。
				 */
				a0 = a0 + b1;
				c2 = c2 + d3;

				b0 = b0 + a2;
				d1 = d1 + c0;

				/*
				 * 再一次扩大 live set
				 */
				a3 = a3 + b3;
				b2 = b2 + c3;
				c1 = c1 + d2;
				d0 = d0 + a1;

				i2 = i2 - 1;
			}

			/*
			 * inner loop 完成后重新洗牌
			 */
			a0 = a0 + c1;
			a2 = a2 + d1;
			b1 = b1 + d3;
			b3 = b3 + a0;

			c0 = c0 + b2;
			c2 = c2 + d0;
			d2 = d2 + a1;
			d3 = d3 + c3;

			i1 = i1 - 1;
		}

		/*
		 * 外层 unwind：
		 * 保留一部分值，另外一部分重新制造寄存器压力。
		 */
		a1 = a1 + b3;
		b0 = b0 + c2;
		c3 = c3 + d1;
		d0 = d0 + a3;

		a2 = a2 + c0;
		b2 = b2 + d3;
		c1 = c1 + a0;
		d2 = d2 + b1;

		i0 = i0 - 1;
	}

	return a0 + a1 + a2 + a3
	    + b0 + b1 + b2 + b3
	    + c0 + c1 + c2 + c3
	    + d0 + d1 + d2 + d3;
}
