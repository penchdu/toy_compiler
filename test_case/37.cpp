int test()
{
	int a0 = 10;
	int a1 = 11;
	int a2 = 12;
	int a3 = 13;
	int a4 = 14;
	int a5 = 15;
	int a6 = 16;
	int a7 = 17;

	int b0 = 20;
	int b1 = 21;
	int b2 = 22;
	int b3 = 23;
	int b4 = 24;
	int b5 = 25;
	int b6 = 26;
	int b7 = 27;

	int c0 = 30;
	int c1 = 31;
	int c2 = 32;
	int c3 = 33;
	int c4 = 34;
	int c5 = 35;
	int c6 = 36;
	int c7 = 37;

	int d0 = 40;
	int d1 = 41;
	int d2 = 42;
	int d3 = 43;
	int d4 = 44;
	int d5 = 45;
	int d6 = 46;
	int d7 = 47;

	int i0 = 2;
	int i1 = 2;
	int i2 = 2;
	int i3 = 2;
	int i4 = 2;

	while ((i0 = i0 - 1
	    + (a0 = a0 + 1)
	    - (b0 = b0 + 1)) > 0)
	{
		/*
		 * 外层 cond 用到 a0/b0。
		 * 这里先不改 a0/b0。
		 * 真正修改它们的是下面的深层 while。
		 *
		 * 另外先改一些无关变量，制造 register pressure。
		 */
		a7 = a7 + b7;
		b7 = b7 + c7;
		c7 = c7 + d7;
		d7 = d7 + a7;

		while ((i1 = i1 - 1
		    + (c0 = c0 + 1)
		    - (d0 = d0 + 1)) > 0)
		{
			/*
			 * 这里开始真正修改 outer-cond 的 a0/b0。
			 */
			a0 = a0 + c0;
			b0 = b0 + d0;

			a1 = a1 + b1;
			b1 = b1 + a1;

			while ((i2 = i2 - 1
			    + (a1 = a1 + 1)
			    - (b1 = b1 + 1)) > 0)
			{
				/*
				 * a0/b0 又被继续使用，
				 * 同时再制造一轮寄存器交换。
				 */
				c0 = c0 + a0;
				d0 = d0 + b0;

				c1 = c1 + d1;
				d1 = d1 + c1;

				while ((i3 = i3 - 1
				    + (c1 = c1 + 1)
				    - (d1 = d1 + 1)) > 0)
				{
					a2 = a2 + c2;
					b2 = b2 + d2;

					a3 = a3 + b3;
					b3 = b3 + a3;

					while ((i4 = i4 - 1
					    + (a2 = a2 + 1)
					    - (b2 = b2 + 1)) > 0)
					{
						/*
						 * 最深层一次性重新扰动多组
						 * outer / middle loop 都依赖的数据。
						 */
						a0 = a0 + c0 + 1;
						b0 = b0 + d0 + 2;

						c0 = c0 + a1 + 3;
						d0 = d0 + b1 + 4;

						a1 = a1 + c1 + 5;
						b1 = b1 + d1 + 6;

						c1 = c1 + a2 + 7;
						d1 = d1 + b2 + 8;

						a2 = a2 + c2 + 9;
						b2 = b2 + d2 + 10;

						c2 = c2 + a3 + 11;
						d2 = d2 + b3 + 12;

						a3 = a3 + c3 + 13;
						b3 = b3 + d3 + 14;

						c3 = c3 + a4 + 15;
						d3 = d3 + b4 + 16;
					}

					/*
					 * 故意在 child while 退出后继续修改
					 * 与祖先状态相关的数据。
					 */
					c4 = c4 + d4;
					d4 = d4 + c4;
				}

				a4 = a4 + b4;
				b4 = b4 + a4;
			}

			c5 = c5 + d5;
			d5 = d5 + c5;
		}

		/*
		 * 最外层 backedge 前再次修改变量。
		 */
		a5 = a5 + c5;
		b5 = b5 + d5;
	}

	return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7
	    + b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7
	    + c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7
	    + d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7
	    + i0 + i1 + i2 + i3 + i4;
}
