int test()
{
    int a0 = 1; int a1 = 2; int a2 = 3; int a3 = 4;
    int b0 = 5; int b1 = 6; int b2 = 7; int b3 = 8;
    int c0 = 9; int c1 = 10; int c2 = 11; int c3 = 12;
    int d0 = 13; int d1 = 14; int d2 = 15; int d3 = 16;
    int e0 = 17; int e1 = 18; int e2 = 19; int e3 = 20;

    int i0 = 4;
    while ((i0 = i0 - 1) > 0)
    {
        a0 = (a0 + e3) / 2 + i0; a1 = (a1 + e2) / 2 + i0;
        b0 = (b0 + e1) / 2 + 1; b1 = (b1 + e0) / 2 + 2;

        int i1 = 4;
        while ((i1 = i1 - 1) > 0)
        {
            a2 = (a2 + d3) / 2 + i1; a3 = (a3 + d2) / 2 + i1;
            b2 = (b2 + c3) / 2 + i0; b3 = (b3 + c2) / 2 + i1;

            int i2 = 4;
            while ((i2 = i2 - 1) > 0)
            {
                c0 = (c0 + a3) / 2 + i2; c1 = (c1 + a2) / 2 + i1;
                d0 = (d0 + b3) / 2 + i2; d1 = (d1 + b2) / 2 + i0;

                int i3 = 3;
                while ((i3 = i3 - 1) > 0)
                {
                    e0 = (e0 + c0) / 2 + i3; e1 = (e1 + c1) / 2 + i2;
                    e2 = (e2 + d0) / 2 + i3; e3 = (e3 + d1) / 2 + i1;
                    a0 = (a0 + e0) / 2 + i3; b0 = (b0 + e1) / 2 + i3;
                    c2 = (c2 + a0) / 2 + i2; d2 = (d2 + b0) / 2 + i2;
                    i3 = i3 - 1;
                }
                i2 = i2 - 1;
                c3 = (c3 + e3) / 2 + i2; d3 = (d3 + e2) / 2 + i2;
            }
            i1 = i1 - 1;
            a1 = (a1 + c3) / 2 + i1; b1 = (b1 + d3) / 2 + i1;
        }
        i0 = i0 - 1;
        a2 = (a2 + b1) / 2 + i0; e0 = (e0 + d1) / 2 + i0;
    }

    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3
         + e0 + e1 + e2 + e3 + i0;
}
