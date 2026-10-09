int test()
{
    int a = 31;
    int b = 47;
    int c = 59;
    int d = 71;
    int e = 83;
    int f = 97;

    int o = 6;

    while (o > 0)
    {
        int j = o - (o / 4) * 4;
        int k = 2;

        while (j > 0)
        {
            int x = (a + c + e) / 3;
            int y = (b + d + f) / 5;

            a = x + o;
            c = x + y + 1;
            e = (y + c) / 2;

            b = y + o;
            d = (x + e) / 3;
            f = (y + d) / 2;

            j = j - 1;
        }

        while (k > 0)
        {
            int q = (a + b + d + f) / 4;

            b = q + o;
            d = (q + c) / 2;
            f = (q + e) / 2;

            k = k - 1;
        }

        o = o - 1;
    }

    return a + b + c + d + e + f;
}
