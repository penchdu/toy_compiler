int test()
{
	int a0 = 1;
	int a1 = 2;
	int a2 = 3;
	int a3 = 4;
	int a4 = 5;
	int a5 = 6;
	int a6 = 7;
	int a7 = 8;

	int b0 = 11;
	int b1 = 12;
	int b2 = 13;
	int b3 = 14;
	int b4 = 15;
	int b5 = 16;
	int b6 = 17;
	int b7 = 18;

	int c0 = 21;
	int c1 = 22;
	int c2 = 23;
	int c3 = 24;
	int c4 = 25;
	int c5 = 26;
	int c6 = 27;
	int c7 = 28;

	int d0 = 31;
	int d1 = 32;
	int d2 = 33;
	int d3 = 34;
	int d4 = 35;
	int d5 = 36;
	int d6 = 37;
	int d7 = 38;

	int e0 = 41;
	int e1 = 42;
	int e2 = 43;
	int e3 = 44;
	int e4 = 45;
	int e5 = 46;
	int e6 = 47;
	int e7 = 48;

	int f0 = 51;
	int f1 = 52;
	int f2 = 53;
	int f3 = 54;
	int f4 = 55;
	int f5 = 56;
	int f6 = 57;
	int f7 = 58;

	int g0 = 61;
	int g1 = 62;
	int g2 = 63;
	int g3 = 64;
	int g4 = 65;
	int g5 = 66;
	int g6 = 67;
	int g7 = 68;

	int h0 = 71;
	int h1 = 72;
	int h2 = 73;
	int h3 = 74;
	int h4 = 75;
	int h5 = 76;
	int h6 = 77;
	int h7 = 78;

	int i0 = 100;
	int i1 = 80;
	int i2 = 60;
	int i3 = 40;
	int i4 = 30;
	int i5 = 20;
	int i6 = 15;
	int i7 = 10;

	/*
	 * =========================================================
	 * LEVEL 1
	 * =========================================================
	 */
	while (i0 > 0)
	{
		a0 = a0 + b0 + c0 + d0;
		a1 = a1 + b1 + c1 + d1;
		a2 = a2 + b2 + c2 + d2;
		a3 = a3 + b3 + c3 + d3;

		int t10 = a0 + a4 + e0 + f0;
		int t11 = a1 + a5 + e1 + f1;
		int t12 = a2 + a6 + e2 + f2;
		int t13 = a3 + a7 + e3 + f3;

		b0 = t10 + g0;
		b1 = t11 + g1;
		b2 = t12 + g2;
		b3 = t13 + g3;

		/*
		 * =====================================================
		 * LEVEL 2
		 * =====================================================
		 */
		while (i1 > 0)
		{
			a0 = a0 + b1 + c2;
			a1 = a1 + b2 + c3;
			a2 = a2 + b3 + c4;

			c0 = c0 + d1 + e2;
			c1 = c1 + d2 + e3;
			c2 = c2 + d3 + e4;

			int t20 = a0 + c0 + f0 + g0;
			int t21 = a1 + c1 + f1 + g1;
			int t22 = a2 + c2 + f2 + g2;
			int t23 = a3 + c3 + f3 + g3;

			d0 = t20 + h0;
			d1 = t21 + h1;
			d2 = t22 + h2;
			d3 = t23 + h3;

			/*
			 * =================================================
			 * LEVEL 3
			 * =================================================
			 */
			while (i2 > 0)
			{
				b0 = b0 + d0 + e0;
				b1 = b1 + d1 + e1;
				b2 = b2 + d2 + e2;
				b3 = b3 + d3 + e3;

				e0 = e0 + f0 + g0;
				e1 = e1 + f1 + g1;
				e2 = e2 + f2 + g2;
				e3 = e3 + f3 + g3;

				int t30 = b0 + e0 + h0 + a0;
				int t31 = b1 + e1 + h1 + a1;
				int t32 = b2 + e2 + h2 + a2;
				int t33 = b3 + e3 + h3 + a3;

				f0 = t30 + c0;
				f1 = t31 + c1;
				f2 = t32 + c2;
				f3 = t33 + c3;

				/*
				 * =============================================
				 * LEVEL 4
				 * =============================================
				 */
				while (i3 > 0)
				{
					c0 = c0 + f0 + g1;
					c1 = c1 + f1 + g2;
					c2 = c2 + f2 + g3;
					c3 = c3 + f3 + g4;

					g0 = g0 + h0 + a1;
					g1 = g1 + h1 + a2;
					g2 = g2 + h2 + a3;
					g3 = g3 + h3 + a4;

					int t40 = c0 + g0 + d0 + e0;
					int t41 = c1 + g1 + d1 + e1;
					int t42 = c2 + g2 + d2 + e2;
					int t43 = c3 + g3 + d3 + e3;

					h0 = t40 + b0;
					h1 = t41 + b1;
					h2 = t42 + b2;
					h3 = t43 + b3;

					/*
					 * =========================================
					 * LEVEL 5
					 * =========================================
					 */
					while (i4 > 0)
					{
						d0 = d0 + h0 + a0;
						d1 = d1 + h1 + a1;
						d2 = d2 + h2 + a2;
						d3 = d3 + h3 + a3;

						f0 = f0 + a4 + b4;
						f1 = f1 + a5 + b5;
						f2 = f2 + a6 + b6;
						f3 = f3 + a7 + b7;

						int t50 = d0 + f0 + c0 + g0;
						int t51 = d1 + f1 + c1 + g1;
						int t52 = d2 + f2 + c2 + g2;
						int t53 = d3 + f3 + c3 + g3;

						a4 = t50 + h4;
						a5 = t51 + h5;
						a6 = t52 + h6;
						a7 = t53 + h7;

						/*
						 * =====================================
						 * LEVEL 6
						 * =====================================
						 */
						while (i5 > 0)
						{
							e0 = e0 + a4 + c4;
							e1 = e1 + a5 + c5;
							e2 = e2 + a6 + c6;
							e3 = e3 + a7 + c7;

							b4 = b4 + d4 + f4;
							b5 = b5 + d5 + f5;
							b6 = b6 + d6 + f6;
							b7 = b7 + d7 + f7;

							int t60 = e0 + b4 + g4 + h4;
							int t61 = e1 + b5 + g5 + h5;
							int t62 = e2 + b6 + g6 + h6;
							int t63 = e3 + b7 + g7 + h7;

							c4 = t60 + a4;
							c5 = t61 + a5;
							c6 = t62 + a6;
							c7 = t63 + a7;

							/*
							 * =================================
							 * LEVEL 7
							 * =================================
							 */
							while (i6 > 0)
							{
								f4 = f4 + c4 + e4;
								f5 = f5 + c5 + e5;
								f6 = f6 + c6 + e6;
								f7 = f7 + c7 + e7;

								g4 = g4 + d4 + b4;
								g5 = g5 + d5 + b5;
								g6 = g6 + d6 + b6;
								g7 = g7 + d7 + b7;

								int t70 = f4 + g4 + a4 + h4;
								int t71 = f5 + g5 + a5 + h5;
								int t72 = f6 + g6 + a6 + h6;
								int t73 = f7 + g7 + a7 + h7;

								d4 = t70 + c4;
								d5 = t71 + c5;
								d6 = t72 + c6;
								d7 = t73 + c7;

								/*
								 * =================================
								 * LEVEL 8
								 * =================================
								 */
								while (i7 > 0)
								{
									h4 = h4 + d4 + f4;
									h5 = h5 + d5 + f5;
									h6 = h6 + d6 + f6;
									h7 = h7 + d7 + f7;

									a4 = a4 + e4 + g4;
									a5 = a5 + e5 + g5;
									a6 = a6 + e6 + g6;
									a7 = a7 + e7 + g7;

									int t80 = h4 + a4 + b4 + c4;
									int t81 = h5 + a5 + b5 + c5;
									int t82 = h6 + a6 + b6 + c6;
									int t83 = h7 + a7 + b7 + c7;

									e4 = t80 + d4;
									e5 = t81 + d5;
									e6 = t82 + d6;
									e7 = t83 + d7;

									/*
									 * 最深层继续反向污染外层状态。
									 */
									a0 = a0 + e4;
									a1 = a1 + e5;
									b0 = b0 + e6;
									b1 = b1 + e7;

									c0 = c0 + f4;
									c1 = c1 + f5;
									d0 = d0 + g4;
									d1 = d1 + g5;

									/*
									 * deepest loop-carried state
									 */
									i7 = i7 - 1
									    + (a0 + 1)
									    - (b1 + 1)
									    + (c0 + 1);
								}

								/*
								 * LEVEL 7 back-edge state
								 */
								f0 = f0 + h4 + e4;
								f1 = f1 + h5 + e5;
								g0 = g0 + h6 + e6;
								g1 = g1 + h7 + e7;

								i6 = i6 - 1
								    + (f0 + 1)
								    - (g1 + 1);
							}

							/*
							 * LEVEL 6 back-edge state
							 */
							e0 = e0 + f0 + g0 + h0;
							e1 = e1 + f1 + g1 + h1;
							e2 = e2 + f2 + g2 + h2;
							e3 = e3 + f3 + g3 + h3;

							i5 = i5 - 1
							    + (e0 + 1)
							    - (e3 + 1);
						}

						/*
						 * LEVEL 5 back-edge state
						 */
						d0 = d0 + e0 + f0 + g0;
						d1 = d1 + e1 + f1 + g1;
						d2 = d2 + e2 + f2 + g2;
						d3 = d3 + e3 + f3 + g3;

						i4 = i4 - 1
						    + (d0 + 1)
						    - (d3 + 1);
					}

					/*
					 * LEVEL 4 back-edge state
					 */
					c0 = c0 + d0 + e0 + f0;
					c1 = c1 + d1 + e1 + f1;
					c2 = c2 + d2 + e2 + f2;
					c3 = c3 + d3 + e3 + f3;

					i3 = i3 - 1
					    + (c0 + 1)
					    - (c3 + 1);
				}

				/*
				 * LEVEL 3 back-edge state
				 */
				b0 = b0 + c0 + d0 + e0;
				b1 = b1 + c1 + d1 + e1;
				b2 = b2 + c2 + d2 + e2;
				b3 = b3 + c3 + d3 + e3;

				i2 = i2 - 1
				    + (b0 + 1)
				    - (b3 + 1);
			}

			/*
			 * LEVEL 2 back-edge state
			 */
			a0 = a0 + b0 + c0 + d0;
			a1 = a1 + b1 + c1 + d1;
			a2 = a2 + b2 + c2 + d2;
			a3 = a3 + b3 + c3 + d3;

			i1 = i1 - 1
			    + (a0 + 1)
			    - (a3 + 1);
		}

		/*
		 * LEVEL 1 back-edge state
		 */
		a4 = a4 + b4 + c4 + d4;
		a5 = a5 + b5 + c5 + d5;
		a6 = a6 + b6 + c6 + d6;
		a7 = a7 + b7 + c7 + d7;

		b4 = b4 + e4 + f4;
		b5 = b5 + e5 + f5;
		b6 = b6 + e6 + f6;
		b7 = b7 + e7 + f7;

		i0 = i0 - 1
		    + (a4 + 1)
		    - (b7 + 1);
	}

	return a0 + a1 + a2 + a3
	    + a4 + a5 + a6 + a7
	    + b0 + b1 + b2 + b3
	    + b4 + b5 + b6 + b7
	    + c0 + c1 + c2 + c3
	    + c4 + c5 + c6 + c7
	    + d0 + d1 + d2 + d3
	    + d4 + d5 + d6 + d7
	    + e0 + e1 + e2 + e3
	    + e4 + e5 + e6 + e7
	    + f0 + f1 + f2 + f3
	    + f4 + f5 + f6 + f7
	    + g0 + g1 + g2 + g3
	    + g4 + g5 + g6 + g7
	    + h0 + h1 + h2 + h3
	    + h4 + h5 + h6 + h7;
}
