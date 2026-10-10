int test()
{
    int a0 = 3; int a1 = 5; int a2 = 7; int a3 = 9;
    int b0 = 4; int b1 = 6; int b2 = 8; int b3 = 10;
    int c0 = 11; int c1 = 13; int c2 = 15; int c3 = 17;
    int d0 = 12; int d1 = 14; int d2 = 16; int d3 = 18;
    int p = 7;

    while ((p = p - 1) > 0)
    {
        if (p > 2)
        {
            a0 = (a0 + c0) / 2 + p;
            a1 = (a1 + c1) / 2 + p;
            b0 = (b0 + d0) / 2 + a0;
            b1 = (b1 + d1) / 2 + a1;
            if (a0 > b1)
            {
                c0 = (c0 + b0) / 2 + p;
                d0 = (d0 + a1) / 2 + b1;
                a2 = (a2 + c0) / 2 + d0;
            }
            else
            {
                c1 = (c1 + b1) / 2 + p;
                d1 = (d1 + a0) / 2 + b0;
                a3 = (a3 + c1) / 2 + d1;
            }
        }
        else
        {
            b2 = (b2 + a2) / 2 + p;
            b3 = (b3 + a3) / 2 + p;
            c2 = (c2 + d2) / 2 + b2;
            c3 = (c3 + d3) / 2 + b3;
            if (c2 > d3)
            {
                d2 = (d2 + c2) / 2 + p;
                a0 = (a0 + d2) / 2 + c3;
                b0 = (b0 + a0) / 2 + d2;
            }
            else
            {
                d3 = (d3 + c3) / 2 + p;
                a1 = (a1 + d3) / 2 + c2;
                b1 = (b1 + a1) / 2 + d3;
            }
        }
    }

    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3 + p;
}
