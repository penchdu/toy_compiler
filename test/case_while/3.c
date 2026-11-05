int test(int dummy)
{
    int a0 = 1;
    int a1 = 2;
    int a2 = 3;
    int a3 = 4;
    int a4 = 5;
    int a5 = 6;

    int i = 2;
    while (i > 0)
    {
        int t0 = a0 + a1;
        int t1 = a2 + a3;
        int t2 = a4 + a5;
        int t3 = t0 + t1;
        int t4 = t1 + t2;
        int t5 = t3 + t4;

        a0 = t3;
        a2 = t4;
        a4 = t5;

        a1 = a0 + t1;
        a3 = a2 + t2;
        a5 = a4 + t0;

        i = i - 1;
    }

    return a0 + a1 + a2 + a3 + a4 + a5;
}
