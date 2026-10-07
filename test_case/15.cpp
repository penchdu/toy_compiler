int test()
{
    int a = 11;
    int b = 22;
    int c = 33;
    int d = 44;
    int e = 55;

    int outer = 7;

    while (outer > 0)
    {
        int inner = outer - (outer / 3) * 3;

        while (inner > 0)
        {
            int t = (a + b + c + d + e) / 5;

            a = t + outer;
            b = t + a;
            c = (t + b) / 2;
            d = (t + c) / 3;
            e = (t + d) / 2;

            inner = inner - 1;
        }

        int q = (a + c + e) / 3;

        a = q + outer;
        c = q + d;

        outer = outer - 1;
    }

    return a + b + c + d + e;
}
