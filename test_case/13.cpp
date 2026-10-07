int test()
{
    int a = 17;
    int b = 29;
    int c = 43;
    int d = 71;

    int i = 6;

    while (i > 0)
    {
        int x = (a + b + c + d) / 4;
        int y = (a + c) / 3;

        a = x + y;
        b = x + a;
        c = (y + b) / 2;
        d = (x + c) / 3;

        i = i - 1;
    }

    int j = 5;

    while (j > 0)
    {
        int q = (a + c) / 2;

        b = q + d;
        d = (q + a) / 3;
        a = (a + b) / 2;
        c = (c + d) / 2;

        j = j - 1;
    }

    return a + b + c + d;
}
