int test()
{
    int gate = 4;
    int x = 17;
    int y = 7;

    int a0 = 1;
    int a1 = 2;
    int a2 = 3;
    int a3 = 4;
    int a4 = 5;
    int a5 = 6;

    int b0 = 7;
    int b1 = 8;
    int b2 = 9;
    int b3 = 10;
    int b4 = 11;
    int b5 = 12;

    int c0 = 13;
    int c1 = 14;
    int c2 = 15;
    int c3 = 16;

    while (gate > 0)
    {
        int t = gate;

        a0 = a0 + b0 + t;
        a1 = a1 + c0;
        a2 = a2 + b1;
        a3 = a3 + c1;

        b0 = b0 + a2;
        b1 = b1 + c2;
        b2 = b2 + a3;
        b3 = b3 + c3;

        c0 = c0 + a4;
        c1 = c1 + b4;
        c2 = c2 + a5;
        c3 = c3 + b5;

        x = x - a0 + b0;
        y = y + a1;

        a4 = a4 + b2;
        a5 = a5 + c2;
        b4 = b4 + a0;
        b5 = b5 + c0;

        gate = gate - 1;
    }

    return gate + x + y
         + a0 + a1 + a2 + a3 + a4 + a5
         + b0 + b1 + b2 + b3 + b4 + b5
         + c0 + c1 + c2 + c3;
}
