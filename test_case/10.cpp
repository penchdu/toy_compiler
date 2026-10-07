int test()
{
    int a = 20;
    int b = 31;
    int c = 42;
    int d = 53;
    int e = 64;
    int i = 7;

    while (i > 0)
    {
        int q = (a + b + c + d + e) / i;
        int r = (b + c + d + e) / 2;

        a = q + 1;
        b = (r + a) / 2;
        c = (q + b) / 3;
        d = (r + c) / 2;
        e = (q + d) / 2;

        i = i - 1;
    }

    return a + b + c + d + e + i;
}
