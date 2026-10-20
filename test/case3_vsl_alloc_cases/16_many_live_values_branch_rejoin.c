int test()
{
    int a0 = 2; int a1 = 4; int a2 = 6; int a3 = 8;
    int b0 = 3; int b1 = 5; int b2 = 7; int b3 = 9;
    int c0 = 10; int c1 = 12; int c2 = 14; int c3 = 16;
    int d0 = 11; int d1 = 13; int d2 = 15; int d3 = 17;
    int i = 8; int flip = 3; int bias = 1;

    while ((i = i - 1) > 0)
    {
        if (flip > 0)
        {
            flip = flip - 1;
            a0 = (a0 + b0) / 2 + i;
            a1 = (a1 + b1) / 2 + bias;
            b0 = (b0 + c0) / 2 + i;
            b1 = (b1 + c1) / 2 + bias;
            if (a0 > a1)
            {
                c0 = (c0 + d0) / 2 + a0;
                c1 = (c1 + d1) / 2 + b0;
                d0 = (d0 + a1) / 2 + i;
            }
            else
            {
                c2 = (c2 + d2) / 2 + a1;
                c3 = (c3 + d3) / 2 + b1;
                d2 = (d2 + a0) / 2 + i;
            }
        }
        else
        {
            flip = flip + 2;
            a2 = (a2 + b2) / 2 + i;
            a3 = (a3 + b3) / 2 + bias;
            b2 = (b2 + c2) / 2 + i;
            b3 = (b3 + c3) / 2 + bias;
            if (c2 > c3)
            {
                d1 = (d1 + a2) / 2 + b2;
                c0 = (c0 + d1) / 2 + i;
            }
            else
            {
                d3 = (d3 + a3) / 2 + b3;
                c1 = (c1 + d3) / 2 + i;
            }
        }

        if (i > 3)
        {
            if (b0 > b2)
            {
                a0 = (a0 + d3) / 2 + i;
                b1 = (b1 + c2) / 2 + flip;
            }
            else
            {
                a1 = (a1 + d2) / 2 + i;
                b0 = (b0 + c3) / 2 + flip;
            }
        }
        else
        {
            if (d0 > d3)
            {
                a2 = (a2 + c0) / 2 + i;
                b2 = (b2 + d1) / 2 + flip;
            }
            else
            {
                a3 = (a3 + c1) / 2 + i;
                b3 = (b3 + d0) / 2 + flip;
            }
        }

        bias = bias + 1;
        d0 = (d0 + a0) / 2 + bias;
        d1 = (d1 + a1) / 2 + flip;
        d2 = (d2 + a2) / 2 + bias;
        d3 = (d3 + a3) / 2 + flip;
    }

    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3 + i + flip + bias;
}
