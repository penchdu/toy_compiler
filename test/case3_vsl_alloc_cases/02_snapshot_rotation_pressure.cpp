int test()
{
    int a0 = 1; int a1 = 2; int a2 = 3; int a3 = 4;
    int a4 = 5; int a5 = 6; int a6 = 7; int a7 = 8;
    int b0 = 9; int b1 = 10; int b2 = 11; int b3 = 12;
    int b4 = 13; int b5 = 14; int b6 = 15; int b7 = 16;

    int i = 4;
    while ((i = i - 1) > 0)
    {
        int t0 = a0; int t1 = a1; int t2 = a2; int t3 = a3;
        int t4 = a4; int t5 = a5; int t6 = a6; int t7 = a7;
        int u0 = b0; int u1 = b1; int u2 = b2; int u3 = b3;
        int u4 = b4; int u5 = b5; int u6 = b6; int u7 = b7;

        a0 = t1 + u0; a1 = t2 + u1; a2 = t3 + u2; a3 = t4 + u3;
        a4 = t5 + u4; a5 = t6 + u5; a6 = t7 + u6; a7 = t0 + u7;
        b0 = u1 + t2; b1 = u2 + t3; b2 = u3 + t4; b3 = u4 + t5;
        b4 = u5 + t6; b5 = u6 + t7; b6 = u7 + t0; b7 = u0 + t1;

        int j = 3;
        while ((j = j - 1) > 0)
        {
            a0 = a0 + b7; a1 = a1 + b6;
            a2 = a2 + b5; a3 = a3 + b4;
            b0 = b0 + a7; b1 = b1 + a6;
            b2 = b2 + a5; b3 = b3 + a4;

            int k = 3;
            while ((k = k - 1) > 0)
            {
                a4 = a4 + b3; a5 = a5 + b2;
                a6 = a6 + b1; a7 = a7 + b0;
                b4 = b4 + a3; b5 = b5 + a2;
                b6 = b6 + a1; b7 = b7 + a0;
            }
        }
    }
    return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7
         + b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + i;
}
