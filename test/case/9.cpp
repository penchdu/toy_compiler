int test()
{
    int a = 11;
    int b = 17;
    int c = 23;
    int d = 29;
    int e = 31;
    int i = 8;

    while (i > 0)
    {
        int j = i - (i / 2) * 2;

        while (j > 0)
        {
            int q = (a + b + c + d + e) / 3;

            a = q + i;
            b = (a + c) / 2;
            c = (b + d) / 2;
            d = (c + e) / 2;
            e = (d + q) / 2;

            j = j - 1;
        }

        int q = (a + e) / 2;

        a = q + b;
        c = (q + d) / 2;

        i = i - 1;
    }

    return a + b + c + d + e + i;
}
