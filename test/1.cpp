int test()
{
	int x = 50;
	int y = 0;
	int step = 15;

	while (x > 0)
	{
		x = x - step;

		if (x > 30)
		{
			y = y + 5;
			continue;
		}
		else
		{
			if (x > 10)
			{
				y = y + 10;
			}
			else
			{
				y = y + 20;
				if (y > 40)
				{
					break;
				}
			}
		}

		y = y + x;
	}

	return x + y + step;
}
