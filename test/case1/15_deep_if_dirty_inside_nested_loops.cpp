int test()
{
    int a0 = 3; int a1 = 5; int a2 = 7; int a3 = 9;
    int b0 = 4; int b1 = 6; int b2 = 8; int b3 = 10;
    int c0 = 11; int c1 = 13; int c2 = 15; int c3 = 17;
    int d0 = 12; int d1 = 14; int d2 = 16; int d3 = 18;
    int p = 7; int gate = 2;

    while ((p = p - 1) > 0)
    {
        p = p - 1;
        if (p > 2)
        {
            a0 = (a0 + b1) / 2 + p;
            a1 = (a1 + c1) / 2 + 1;
            b0 = (b0 + d0) / 2 + p;
            b1 = (b1 + a1) / 2 + 2;
            if (a0 > b0)
            {
                c0 = (c0 + a0) / 2 + gate;
                d0 = (d0 + b1) / 2 + p;
                gate = gate - 1;
            }
            else
            {
                c1 = (c1 + b0) / 2 + gate;
                d1 = (d1 + a1) / 2 + p;
                gate = gate + 1;
            }
        }
        else
        {
            a2 = (a2 + d2) / 2 + p;
            a3 = (a3 + d3) / 2 + 1;
            b2 = (b2 + c2) / 2 + p;
            b3 = (b3 + c3) / 2 + 2;
            if (c2 > c3)
            {
                d2 = (d2 + a2) / 2 + gate;
                a0 = (a0 + d2) / 2 + p;
            }
            else
            {
                d3 = (d3 + b3) / 2 + gate;
                b0 = (b0 + d3) / 2 + p;
            }
        }

        int q = 5;
        while ((q = q - 1) > 0)
        {
            if (q > 2)
            {
                c2 = (c2 + a0) / 2 + q;
                d2 = (d2 + b0) / 2 + p;
                gate = gate + 1;
            }
            else
            {
                c3 = (c3 + a1) / 2 + q;
                d3 = (d3 + b1) / 2 + p;
                gate = gate - 1;
            }

            int r = 4;
            while ((r = r - 1) > 0)
            {
                if (r > 1)
                {
                    a2 = (a2 + c2) / 2 + r;
                    b2 = (b2 + d2) / 2 + q;
                }
                else
                {
                    a3 = (a3 + c3) / 2 + r;
                    b3 = (b3 + d3) / 2 + p;
                    p = p - 1;
                }
            }
        }
        c0 = (c0 + d3) / 2 + p;
        c1 = (c1 + d2) / 2 + gate;
    }

    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3 + p + gate;
}
