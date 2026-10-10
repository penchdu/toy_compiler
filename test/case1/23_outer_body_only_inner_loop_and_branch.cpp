int test()
{
    int a0 = 1; int a1 = 2; int a2 = 3; int a3 = 4;
    int b0 = 5; int b1 = 6; int b2 = 7; int b3 = 8;
    int c0 = 9; int c1 = 10; int c2 = 11; int c3 = 12;
    int d0 = 13; int d1 = 14; int d2 = 15; int d3 = 16;
    int p = 5;

    while ((p = p - 1) > 0)
    {
        int q = 5;
        while ((q = q - 1) > 0)
        {
            if (q > 2)
            {
                a0 = (a0 + b1) / 2 + p;
                a1 = (a1 + c1) / 2 + q;
                b0 = (b0 + d0) / 2 + a0;
                b1 = (b1 + d1) / 2 + a1;
                c0 = (c0 + a0) / 2 + q;
                d0 = (d0 + b1) / 2 + p;
            }
            else
            {
                a2 = (a2 + b3) / 2 + p;
                a3 = (a3 + c3) / 2 + q;
                b2 = (b2 + d2) / 2 + a2;
                b3 = (b3 + d3) / 2 + a3;
                c2 = (c2 + a3) / 2 + q;
                d2 = (d2 + b2) / 2 + p;
            }
        }
    }

    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3 + p;
}
