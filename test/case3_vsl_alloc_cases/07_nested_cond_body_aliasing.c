int test()
{
    int a0 = 3; int a1 = 5; int a2 = 7; int a3 = 9;
    int b0 = 4; int b1 = 6; int b2 = 8; int b3 = 10;
    int c0 = 11; int c1 = 13; int c2 = 15; int c3 = 17;
    int d0 = 12; int d1 = 14; int d2 = 16; int d3 = 18;

    int p = 7;
    while ((p = p - 1) > 0)
    {
        a0 = (a0 + b0) / 2 + p; a1 = (a1 + b1) / 2 + p;
        b0 = (b0 + c0) / 2 + 1; b1 = (b1 + c1) / 2 + 2;

        int q = 6;
        while ((q = q - 1) > 0)
        {
            a2 = (a2 + d2) / 2 + q; a3 = (a3 + d3) / 2 + q;
            c0 = (c0 + a2) / 2 + p; c1 = (c1 + a3) / 2 + q;

            int r = 5;
            while ((r = r - 1) > 0)
            {
                c2 = (c2 + b3) / 2 + r; c3 = (c3 + b2) / 2 + r;
                d0 = (d0 + c2) / 2 + p; d1 = (d1 + c3) / 2 + q;
                a0 = (a0 + d1) / 2 + r; b0 = (b0 + d0) / 2 + r;
            }
            q = q - 1;
            p = p - 1;
            d2 = (d2 + a0) / 2 + q; d3 = (d3 + b0) / 2 + p;
        }
        a1 = (a1 + c3) / 2 + p; b1 = (b1 + d2) / 2 + p;
    }

    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3 + p;
}
