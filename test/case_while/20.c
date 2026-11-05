int test(int dummy)
{
    int total = 100;

    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;

    int outer = 3;

    while (outer > 0)
    {
        int inner = outer - 1;

        while (inner > 0)
        {
            a = a + b;
            b = b + c;
            c = c + d;
            d = d + e;
            e = e + a;

            total = total + a + c + e;

            inner = inner - 1;
        }

        a = a + total;
        c = c + b;
        e = e + d;

        outer = outer - 1;
    }

    return total + a + b + c + d + e + outer;
}
