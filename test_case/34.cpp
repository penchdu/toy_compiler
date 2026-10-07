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

	int e0 = 41;
	int e1 = 42;
	int e2 = 43;
	int e3 = 44;

	int i0 = 2;
	int i1 = 2;
	int i2 = 2;
	int i3 = 2;
	int i4 = 2;

	while ((i0 = i0 - 1)
	    + a0 - a0
	    + a1 - a1
	    + b0 - b0
	    + b1 - b1
	    + c0 - c0
	    + c1 - c1 > 0)
	{
		i1 = 2;

		b0 = b0 + a2 + c2;
		b1 = b1 + a3 + c3;

		c0 = c0 + d2 + e2;
		c1 = c1 + d3 + e3;

		d0 = d0 + b2 + c2;
		d1 = d1 + b3 + c3;

		e0 = e0 + d0 + a0;
		e1 = e1 + d1 + a1;

		while ((i1 = i1 - 1)
		    + b0 - b0
		    + b1 - b1
		    + c0 - c0
		    + c1 - c1
		    + d0 - d0
		    + d1 - d1 > 0)
		{
			i2 = 2;

			a0 = a0 + e2 + d2;
			a1 = a1 + e3 + d3;

			c2 = c2 + a2 + b2;
			c3 = c3 + a3 + b3;

			d2 = d2 + c0 + e0;
			d3 = d3 + c1 + e1;

			e2 = e2 + d2 + a2;
			e3 = e3 + d3 + a3;

			while ((i2 = i2 - 1)
			    + a2 - a2
			    + a3 - a3
			    + c2 - c2
			    + c3 - c3
			    + d2 - d2
			    + d3 - d3 > 0)
			{
				i3 = 2;

				a2 = a2 + e0 + c0;
				a3 = a3 + e1 + c1;

				b2 = b2 + a0 + d0;
				b3 = b3 + a1 + d1;

				d0 = d0 + b0 + e2;
				d1 = d1 + b1 + e3;

				e0 = e0 + a2 + d2;
				e1 = e1 + a3 + d3;

				while ((i3 = i3 - 1)
				    + d0 - d0
				    + d1 - d1
				    + e0 - e0
				    + e1 - e1
				    + a0 - a0
				    + a1 - a1 > 0)
				{
					i4 = 2;

					a0 = a0 + b2 + d3;
					a1 = a1 + b3 + d2;

					b0 = b0 + c2 + e3;
					b1 = b1 + c3 + e2;

					c0 = c0 + a3 + d0;
					c1 = c1 + a2 + d1;

					while ((i4 = i4 - 1)
					    + e2 - e2
					    + e3 - e3
					    + a2 - a2
					    + a3 - a3
					    + b0 - b0
					    + b1 - b1 > 0)
					{
						d2 = d2 + a0 + c3;
						d3 = d3 + a1 + c2;

						e2 = e2 + b0 + d0;
						e3 = e3 + b1 + d1;

						a2 = a2 + e2 + c0;
						a3 = a3 + e3 + c1;
					}

					/*
					 * 回到 i3 body 后，再次修改刚才
					 * 在最深层被修改过的变量。
					 */
					d0 = d0 + e2 + a2;
					d1 = d1 + e3 + a3;
				}

				/*
				 * 这里继续碰祖先层的 VR。
				 */
				b2 = b2 + c0 + d0;
				b3 = b3 + c1 + d1;
			}

			/*
			 * 重新碰 outer cond 相关 VR。
			 */
			a0 = a0 + e2 + d2;
			a1 = a1 + e3 + d3;
		}

		/*
		 * outer backedge 前再做一次交叉。
		 */
		c2 = c2 + a0 + b0;
		c3 = c3 + a1 + b1;

		d2 = d2 + c2 + e0;
		d3 = d3 + c3 + e1;
	}

	return a0 + a1 + a2 + a3
	    + b0 + b1 + b2 + b3
	    + c0 + c1 + c2 + c3
	    + d0 + d1 + d2 + d3
	    + e0 + e1 + e2 + e3
	    + i0 + i1 + i2 + i3 + i4;
}
