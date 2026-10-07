int test()
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

        /*
         * inner 对某些 outer 值是 0，
         * 但 allocator 仍然必须构造整个 inner body 的状态。
         */
        a = a + total;
        c = c + b;
        e = e + d;

        outer = outer - 1;
    }

    return total + a + b + c + d + e + outer;
}
