int test()
{
    int a;
    int b;

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