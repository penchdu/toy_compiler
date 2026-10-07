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

	int i0 = 4;
	int i1 = 3;
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
		 * body 故意极轻，只改一个 outer-cond 使用的 VR。
		 */
		a0 = a0 + 1;

		i1 = 3;

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
			 * 同样只改一个 cond 使用的 VR。
			 */
			b0 = b0 + 1;

			i2 = 2;

			while ((i2 = i2 - 1)
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
				 * 最深层 body 几乎空，只改一个 VR。
				 */
				c0 = c0 + 1;
			}
		}
	}

	return a0 + a1 + a2 + a3
	    + b0 + b1 + b2 + b3
	    + c0 + c1 + c2 + c3
	    + d0 + d1 + d2 + d3
	    + i0 + i1 + i2;
}
