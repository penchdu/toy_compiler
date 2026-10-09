int test()
{
    int a0 = 4; int a1 = 6; int a2 = 8; int a3 = 10;
    int b0 = 5; int b1 = 7; int b2 = 9; int b3 = 11;
    int c0 = 12; int c1 = 14; int c2 = 16; int c3 = 18;
    int d0 = 13; int d1 = 15; int d2 = 17; int d3 = 19;

    int gate = 22;
    int step = 1;
    int floor = 0;
    while ((gate = gate - step) > 0)
    {
        gate = gate - 1;
        step = step + 1;
        a0 = (a0 + b0) / 2 + gate; a1 = (a1 + b1) / 2 + step;
        c0 = (c0 + d0) / 2 + gate; c1 = (c1 + d1) / 2 + step;

        int j = 20;
        while ((j = j - step) > floor)
        {
            j = j - 1;
            floor = floor + 1;
            step = step + 1;
            a2 = (a2 + c2) / 2 + j; a3 = (a3 + c3) / 2 + floor;
            b2 = (b2 + d2) / 2 + step; b3 = (b3 + d3) / 2 + j;
            gate = gate - 1;
        }

        b0 = (b0 + a3) / 2 + step; b1 = (b1 + a2) / 2 + floor;
        d2 = (d2 + c1) / 2 + gate; d3 = (d3 + c0) / 2 + step;
    }

    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3
         + gate + step + floor;
}
