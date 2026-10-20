int test(int dummy)
{
    int a = 2;
	int b = 3;

    if (a == 1)
    {
        if (b == 2)
        {
            a = 3;
        }
        else if (b == 3)
        {
            a = 4;
        }
        else
        {
            a = 5;
        }
    }
    else if (a == 2)
    {
        a = 6;
    }
    else
    {
        a = 7;
    }

    return a;
}