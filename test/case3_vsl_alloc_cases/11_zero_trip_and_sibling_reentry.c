int test()
{
    int a0 = 1; int a1 = 3; int a2 = 5; int a3 = 7;
    int b0 = 2; int b1 = 4; int b2 = 6; int b3 = 8;
    int c0 = 9; int c1 = 11; int c2 = 13; int c3 = 15;

    int skip = 1;
    while ((skip = skip - 1) > 0)
    {
        a0 = a0 + b0;
        b0 = b0 + c0;
    }
    a0 = a0 + skip;
    c0 = c0 + skip;

    int p = 6;
    while ((p = p - 1) > 0)
    {
        a1 = (a1 + b1) / 2 + p; c1 = (c1 + a1) / 2 + p;
        int q = 5;
        while ((q = q - 1) > 0)
        {
            b2 = (b2 + c2) / 2 + q; a2 = (a2 + b2) / 2 + q;
            q = q - 1;
            c2 = (c2 + a2) / 2 + q; b1 = (b1 + c1) / 2 + p;
        }
        p = p - 1;
        a3 = (a3 + c3) / 2 + p; b3 = (b3 + a3) / 2 + p;
    }

    int s = 5;
    while ((s = s - 1) > 0)
    {
        c0 = (c0 + b3) / 2 + s; a0 = (a0 + c3) / 2 + s;
        int t = 4;
        while ((t = t - 1) > 0)
        {
            b0 = (b0 + a3) / 2 + t; c3 = (c3 + b0) / 2 + t;
            t = t - 1;
            s = s - 1;
        }
        a2 = (a2 + c1) / 2 + s; b2 = (b2 + a1) / 2 + s;
    }

    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + skip + p + s;
}
