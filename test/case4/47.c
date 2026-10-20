int test()
{
	int a = 1;
	int b = 0;

	// 不进入
	while (a < 0)
	{
		a = a + 100;
	}

	while (a < 10)
	{
		a = a + 1;

		if (a == 2)
		{
			while (a < 20)
			{
				b = b + 1;

				break;

				b = b + 100;
				a = a + 100;
			}

			a = a + 2;
		}

		if (a == 7)
		{
			break;
		}

		a = a + 2;
	}

	return a + b;
}
