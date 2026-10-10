int test()
{
    int a0 = 4; int a1 = 6; int a2 = 8; int a3 = 10;
    int b0 = 5; int b1 = 7; int b2 = 9; int b3 = 11;
    int c0 = 12; int c1 = 14; int c2 = 16; int c3 = 18;
    int d0 = 13; int d1 = 15; int d2 = 17; int d3 = 19;
    int p = 8; int token = 2;

    while ((p = p - 1) > 0)
    {
        p = p - 1;
        if (p > 3)
        {
            int q = 6;
            while ((q = q - 1) > 0)
            {
                q = q - 1;
                if (q > 2)
                {
                    a0 = (a0 + d3) / 2 + p;
                    b0 = (b0 + c3) / 2 + q;
                    token = token + 1;
                    p = p - 1;
                }
                else
                {
                    a1 = (a1 + d2) / 2 + p;
                    b1 = (b1 + c2) / 2 + q;
                    token = token - 1;
                }

                int r = 4;
                while ((r = r - 1) > 0)
                {
                    if (r > 1)
                    {
                        c0 = (c0 + a0) / 2 + token;
                        d0 = (d0 + b0) / 2 + p;
                    }
                    else
                    {
                        c1 = (c1 + a1) / 2 + token;
                        d1 = (d1 + b1) / 2 + p;
                        q = q - 1;
                    }
                }
            }
        }
        else
        {
            c2 = (c2 + a2) / 2 + p;
            d2 = (d2 + b2) / 2 + token;
            if (c2 > d2)
            {
                a2 = (a2 + c2) / 2 + p;
                b2 = (b2 + d2) / 2 + token;
            }
            else
            {
                a3 = (a3 + c3) / 2 + p;
                b3 = (b3 + d3) / 2 + token;
            }
        }

        a0 = (a0 + b3) / 2 + token;
        a1 = (a1 + b2) / 2 + p;
        c3 = (c3 + d1) / 2 + token;
        d3 = (d3 + c1) / 2 + p;
    }

    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3 + p + token;
}
