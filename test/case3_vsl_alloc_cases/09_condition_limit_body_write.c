int test()
{
    int a0 = 2; int a1 = 4; int a2 = 6; int a3 = 8;
    int b0 = 3; int b1 = 5; int b2 = 7; int b3 = 9;
    int c0 = 10; int c1 = 12; int c2 = 14; int c3 = 16;
    int d0 = 11; int d1 = 13; int d2 = 15; int d3 = 17;

    int i = 9;
    int limit = 0;
    int floor = 0;
    while ((i = i - 1) > limit)
    {
        a0 = (a0 + b0) / 2 + i; a1 = (a1 + b1) / 2 + limit;
        c0 = (c0 + d0) / 2 + i; c1 = (c1 + d1) / 2 + limit;
        limit = limit + 1;

        int j = 8;
        while ((j = j - 1) > floor)
        {
            a2 = (a2 + c2) / 2 + j; a3 = (a3 + c3) / 2 + floor;
            b2 = (b2 + d2) / 2 + j; b3 = (b3 + d3) / 2 + floor;
            floor = floor + 1;
            j = j - 1;
            b0 = (b0 + a3) / 2 + j; d0 = (d0 + b3) / 2 + floor;
        }

        i = i - 1;
        c2 = (c2 + a1) / 2 + limit; d2 = (d2 + b1) / 2 + i;
        b1 = (b1 + c2) / 2 + limit; d1 = (d1 + a2) / 2 + floor;
    }

    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3
         + i + limit + floor;
}
