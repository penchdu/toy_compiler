int test()
{
    int keep0 = 111;
    int target = 37;

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
        a1 = a1 + b1;
        a2 = a2 + b2;
        a3 = a3 + b3;
        a4 = a4 + b4;
        a5 = a5 + b5;

        b0 = b0 + c0;
        b1 = b1 + c1;
        b2 = b2 + c2;
        b3 = b3 + c3;

        c0 = c0 + a1;
        c1 = c1 + a2;
        c2 = c2 + a3;
        c3 = c3 + a4;

        target = target + a0 + a5;

        int j = 2;

        while (j > 0)
        {
            a0 = a0 + c3;
            b0 = b0 + a2;
            c0 = c0 + b4;

            a3 = a3 + c1;
            b3 = b3 + a4;
            c3 = c3 + b1;

            j = j - 1;
        }

        i = i - 1;
    }

    return keep0 + target
         + a0 + a1 + a2 + a3 + a4 + a5
         + b0 + b1 + b2 + b3 + b4 + b5
         + c0 + c1 + c2 + c3;
}
