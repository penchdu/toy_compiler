int test()
{
    int x0 = 0; int x1 = 1; int x2 = 2; int x3 = 3;
    int x4 = 4; int x5 = 5; int x6 = 6; int x7 = 7;
    int x8 = 8; int x9 = 9; int x10 = 10; int x11 = 11;
    int a = 2; int b = 3; int i = 6;

    while (((i = i - 1) + x0 - x0 + x1 - x1 + x2 - x2
            + x3 - x3 + x4 - x4 + x5 - x5 + x6 - x6
            + x7 - x7 + x8 - x8 + x9 - x9 + x10 - x10
            + x11 - x11) > 0)
    {
        if (i > 3)
        {
            x0 = (x0 + x3) / 2 + i;
            x1 = (x1 + x4) / 2 + a;
            x2 = (x2 + x5) / 2 + b;
            if (x0 > x1)
            {
                x6 = (x6 + x9) / 2 + x0;
                x7 = (x7 + x10) / 2 + x1;
                a = a + 1;
            }
            else
            {
                x8 = (x8 + x11) / 2 + x2;
                x9 = (x9 + x6) / 2 + x0;
                b = b + 1;
            }
        }
        else
        {
            x3 = (x3 + x0) / 2 + i;
            x4 = (x4 + x1) / 2 + a;
            x5 = (x5 + x2) / 2 + b;
            if (x4 > x5)
            {
                x10 = (x10 + x7) / 2 + x3;
                x11 = (x11 + x8) / 2 + x4;
            }
            else
            {
                x6 = (x6 + x9) / 2 + x5;
                x7 = (x7 + x10) / 2 + x4;
            }
        }
        int j = 4;
        while ((j = j - 1) > 0)
        {
            if (j > 1)
            {
                x0 = (x0 + x11) / 2 + j;
                x3 = (x3 + x8) / 2 + i;
                a = a + 1;
            }
            else
            {
                x1 = (x1 + x10) / 2 + j;
                x2 = (x2 + x9) / 2 + i;
                b = b + 1;
            }
        }
    }

    return x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7
         + x8 + x9 + x10 + x11 + a + b + i;
}
