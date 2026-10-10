int test()
{
    int a0 = 1; int a1 = 2; int a2 = 3; int a3 = 4;
    int b0 = 5; int b1 = 6; int b2 = 7; int b3 = 8;
    int c0 = 9; int c1 = 10; int c2 = 11; int c3 = 12;

    int p = 6;
    while ((p = p - 1) > 0)
    {
        p = p - 1;
        a0 = a0 + b0; a1 = a1 + b1;
        b0 = b0 + c0; b1 = b1 + c1;

        int q = 5;
        while ((q = q - 1) > 0)
        {
            q = q - 1;
            a2 = a2 + b2; a3 = a3 + b3;
            b2 = b2 + c2; b3 = b3 + c3;

            int r = 4;
            while ((r = r - 1) > 0)
            {
                r = r - 1;
                c0 = c0 + a3; c1 = c1 + a2;
                c2 = c2 + a1; c3 = c3 + a0;
                a0 = a0 + c3; a3 = a3 + c0;
                b0 = b0 + c2; b3 = b3 + c1;
            }
            a1 = a1 + b3; a2 = a2 + b0;
        }
        c0 = c0 + a1; c3 = c3 + a2;
        b1 = b1 + c3; b2 = b2 + c0;
    }
    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + p;
}
