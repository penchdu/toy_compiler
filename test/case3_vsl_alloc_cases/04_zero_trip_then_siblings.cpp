int test()
{
    int a0 = 2; int a1 = 3; int a2 = 4; int a3 = 5;
    int b0 = 6; int b1 = 7; int b2 = 8; int b3 = 9;
    int c0 = 10; int c1 = 11; int c2 = 12; int c3 = 13;

    int empty = 1;
    while ((empty = empty - 1) > 0)
    {
        a0 = a0 + b0; b0 = b0 + c0;
        c0 = c0 + a0;
    }

    int i = 4;
    while ((i = i - 1) > 0)
    {
        a0 = a0 + b1; a1 = a1 + b2;
        b0 = b0 + c1; b1 = b1 + c2;

        int j = 3;
        while ((j = j - 1) > 0)
        {
            a2 = a2 + b3; a3 = a3 + b0;
            c0 = c0 + a3; c1 = c1 + a2;
        }
        b2 = b2 + c3; b3 = b3 + c0;
    }

    int k = 5;
    while ((k = k - 1) > 0)
    {
        c2 = c2 + a0; c3 = c3 + a1;
        a0 = a0 + c2; a1 = a1 + c3;
        b0 = b0 + a2; b1 = b1 + a3;
    }

    int m = 3;
    while ((m = m - 1) > 0)
    {
        int n = 3;
        while ((n = n - 1) > 0)
        {
            a2 = a2 + c3; b2 = b2 + a1;
            c2 = c2 + b3; a3 = a3 + c0;
        }
        b3 = b3 + a2; c3 = c3 + b2;
    }

    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + empty + i + k + m;
}
