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

        a0 = a2 + t0;
        a2 = a4 + t1;
        a4 = a0 + t2;

        a1 = a1 + a3;
        a3 = a3 + a5;
        a5 = a5 + a1;

        i = i - 1;
    }

    return a0 + a1 + a2 + a3 + a4 + a5;
}
