int test()
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;
    int f = 6;
    int g = 7;

    int x = 3;

    while (x > 0)
    {
        int p = 2;

        while (p > 0)
        {
            int t0 = a + d + g;
            int t1 = b + e + a;
            int t2 = c + f + b;

            d = d + t0;
            e = e + t1;
            f = f + t2;

            a = a + f;
            b = b + d;
            c = c + e;

            g = g + a + b + c;

            p = p - 1;
        }

        int q0 = a + b + c;
        int q1 = d + e + f;
        int q2 = g + a + f;

        a = a + q1;
        b = b + q2;
        c = c + q0;

        d = d + a;
        e = e + b;
        f = f + c;

        g = g + d + e + f;

        x = x - 1;
    }

    return a + b + c + d + e + f + g;
}
