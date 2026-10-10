int test()
{
    int a0 = 1; int a1 = 2; int a2 = 3; int a3 = 4; int a4 = 5;
    int b0 = 6; int b1 = 7; int b2 = 8; int b3 = 9; int b4 = 10;
    int c0 = 11; int c1 = 12; int c2 = 13; int c3 = 14; int c4 = 15;
    int d0 = 16; int d1 = 17; int d2 = 18; int d3 = 19; int d4 = 20;
    int p = 4; int bias = 2;

    while ((p = p - 1) > 0)
    {
        a0 = (a0 + b4) / 2 + p; a1 = (a1 + c4) / 2 + bias;
        b0 = (b0 + d4) / 2 + p; b1 = (b1 + a4) / 2 + bias;
        int q = 4;
        while ((q = q - 1) > 0)
        {
            a2 = (a2 + d3) / 2 + q; a3 = (a3 + d2) / 2 + p;
            b2 = (b2 + c3) / 2 + q; b3 = (b3 + c2) / 2 + bias;
            int r = 4;
            while ((r = r - 1) > 0)
            {
                if (r > 2)
                {
                    c0 = (c0 + a3) / 2 + r;
                    c1 = (c1 + b2) / 2 + q;
                    d0 = (d0 + c0) / 2 + p;
                    d1 = (d1 + c1) / 2 + bias;
                    a4 = (a4 + d0) / 2 + r;
                    b4 = (b4 + d1) / 2 + q;
                }
                else
                {
                    c2 = (c2 + a2) / 2 + r;
                    c3 = (c3 + b3) / 2 + q;
                    d2 = (d2 + c2) / 2 + p;
                    d3 = (d3 + c3) / 2 + bias;
                    a0 = (a0 + d2) / 2 + r;
                    b0 = (b0 + d3) / 2 + q;
                }
                r = r - 1;
            }
            if (q > 1)
            {
                c4 = (c4 + a0) / 2 + q;
                d4 = (d4 + b0) / 2 + p;
            }
            else
            {
                c4 = (c4 + a1) / 2 + q;
                d4 = (d4 + b1) / 2 + p;
            }
        }
        bias = bias + 1;
        a1 = (a1 + d4) / 2 + bias;
        b1 = (b1 + c4) / 2 + p;
    }

    return a0 + a1 + a2 + a3 + a4
         + b0 + b1 + b2 + b3 + b4
         + c0 + c1 + c2 + c3 + c4
         + d0 + d1 + d2 + d3 + d4 + p + bias;
}
