int test()
{
    int i = 5;

    int a = 3;
    int b = 7;
    int c = 11;
    int d = 13;
    int e = 17;
    int f = 19;

    while (a > 0)
    {
        int t0 = a + b;
        int t1 = c + d;
        int t2 = e + f;

        b = b + t0;
        c = c + t1;
        d = d + t2;

        a = a - 1;

        int q0 = a + c;
        int q1 = b + d;
        int q2 = e + f;

        e = e + q0;
        f = f + q1;
        b = b + q2;

        i = i + a + b + c + d + e + f;
    }

    return i + a + b + c + d + e + f;
}
