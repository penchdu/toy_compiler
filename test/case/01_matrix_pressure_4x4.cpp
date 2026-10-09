int test()
{
    int a0 = 1;  int a1 = 2;  int a2 = 3;  int a3 = 4;
    int b0 = 5;  int b1 = 6;  int b2 = 7;  int b3 = 8;
    int c0 = 9;  int c1 = 10; int c2 = 11; int c3 = 12;
    int d0 = 13; int d1 = 14; int d2 = 15; int d3 = 16;

    int i0 = 3;
    while ((i0 = i0 - 1) > 0)
    {
        a0 = a0 + b0; a1 = a1 + b1; a2 = a2 + b2; a3 = a3 + b3;
        b0 = b0 + c0; b1 = b1 + c1; b2 = b2 + c2; b3 = b3 + c3;
        c0 = c0 + d0; c1 = c1 + d1; c2 = c2 + d2; c3 = c3 + d3;
        d0 = d0 + a0; d1 = d1 + a1; d2 = d2 + a2; d3 = d3 + a3;

        int i1 = 3;
        while ((i1 = i1 - 1) > 0)
        {
            a0 = a0 + d3; a3 = a3 + d0;
            b0 = b0 + d2; b3 = b3 + d1;
            c0 = c0 + a3; c3 = c3 + a0;
            d0 = d0 + b3; d3 = d3 + b0;

            int i2 = 3;
            while ((i2 = i2 - 1) > 0)
            {
                a1 = a1 + c3; a2 = a2 + c0;
                b1 = b1 + d3; b2 = b2 + d0;
                c1 = c1 + a2; c2 = c2 + a1;
                d1 = d1 + b2; d2 = d2 + b1;

                a0 = a0 + c1; b2 = b2 + d3;
                c0 = c0 + a1; d2 = d2 + b1;
            }
            a1 = a1 + b2; b1 = b1 + c2;
            c1 = c1 + d2; d1 = d1 + a2;
        }
        a2 = a2 + d0; b2 = b2 + a3;
        c2 = c2 + b0; d2 = d2 + c3;
    }
    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3 + i0;
}
