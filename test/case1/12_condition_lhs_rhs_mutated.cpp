int test()
{
    int a0 = 3; int a1 = 5; int a2 = 7; int a3 = 9;
    int b0 = 4; int b1 = 6; int b2 = 8; int b3 = 10;
    int c0 = 11; int c1 = 13; int c2 = 15; int c3 = 17;
    int d0 = 12; int d1 = 14; int d2 = 16; int d3 = 18;

    int x = 50;
    int y = 2;
    int bound = 1;
    while ((x = x - y) > bound)
    {
        x = x - 1;
        y = y + 1;
        bound = bound + 1;
        a0 = (a0 + b0) / 2 + x; a1 = (a1 + b1) / 2 + y;
        c0 = (c0 + d0) / 2 + x; c1 = (c1 + d1) / 2 + bound;

        int u = 24;
        int v = 2;
        while ((u = u - v) > bound)
        {
            u = u - 1;
            v = v + 1;
            bound = bound + 1;
            a2 = (a2 + c2) / 2 + u; a3 = (a3 + c3) / 2 + v;
            b2 = (b2 + d2) / 2 + u; b3 = (b3 + d3) / 2 + bound;
        }

        b0 = (b0 + a3) / 2 + x; d0 = (d0 + b3) / 2 + y;
        c2 = (c2 + a1) / 2 + bound; d2 = (d2 + b1) / 2 + x;
    }

    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3
         + x + y + bound;
}
