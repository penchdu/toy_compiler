int test()
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;

    int outer = 4;

    while (outer > 0)
    {
        int inner = outer - (outer / 3) * 3;

        while (inner > 0)
        {
            int t = a + b + c + d + e;

            a = t + outer;
            b = a + 1;
            c = b + 2;
            d = c + 3;
            e = d + 4;

            inner = inner - 1;
        }

        int q = a + e;

        a = q + 1;
        c = q + 2;

        outer = outer - 1;
    }

    return a + b + c + d + e;
}
