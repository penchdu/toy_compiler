int test()
{
    int a = 1;
    int b = 10;
    int c = 100;
    int d = 1000;
    int e = 10000;
    int f = 100000;
    int g = 1000000;

    int i = 4;

    while (i > 0)
    {
        int t0 = a + b;
        int t1 = b + c;
        int t2 = c + d;
        int t3 = d + e;
        int t4 = e + f;
        int t5 = f + g;
        int t6 = g + a;

        a = t6 + t0;
        b = t0 + t1;
        c = t1 + t2;
        d = t2 + t3;
        e = t3 + t4;
        f = t4 + t5;
        g = t5 + t6;

        i = i - 1;
    }

    return a + b + c + d + e + f + g;
}
