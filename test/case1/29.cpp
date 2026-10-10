int test()
{
    int hold0 = 91;
    int hold1 = 137;

    int a0 = 1;
    int a1 = 2;
    int a2 = 3;
    int a3 = 4;
    int a4 = 5;
    int a5 = 6;

    int b0 = 7;
    int b1 = 8;
    int b2 = 9;
    int b3 = 10;
    int b4 = 11;
    int b5 = 12;

    int c0 = 13;
    int c1 = 14;
    int c2 = 15;
    int c3 = 16;

    int i = 3;

    while (i > 0)
    {
        a0 = a0 + b0;
        a1 = a1 + c0;
        a2 = a2 + b1;
        a3 = a3 + c1;

        b0 = b0 + a2;
        b1 = b1 + c2;
        b2 = b2 + a3;
        b3 = b3 + c3;

        int j = 2;

        while (j > 0)
        {
            c0 = c0 + a4;
            c1 = c1 + b4;
            c2 = c2 + a5;
            c3 = c3 + b5;

            a4 = a4 + b2;
            a5 = a5 + c2;
            b4 = b4 + a0;
            b5 = b5 + c0;

            j = j - 1;
        }

        i = i - 1;
    }

    return hold0 + hold1
         + a0 + a1 + a2 + a3 + a4 + a5
         + b0 + b1 + b2 + b3 + b4 + b5
         + c0 + c1 + c2 + c3;
}
