int test()
{
    int a0 = 1; int a1 = 2; int a2 = 3; int a3 = 4;
    int b0 = 5; int b1 = 6; int b2 = 7; int b3 = 8;
    int c0 = 9; int c1 = 10; int c2 = 11; int c3 = 12;
    int d0 = 13; int d1 = 14; int d2 = 15; int d3 = 16;
    int i = 6; int shared = 2;

    while ((i = i - 1) > 0)
    {
        if (i > 3)
        {
            int j = 4;
            while ((j = j - 1) > 0)
            {
                if (j > 2)
                {
                    a0 = (a0 + b1) / 2 + i;
                    b0 = (b0 + c1) / 2 + j;
                    c0 = (c0 + d1) / 2 + shared;
                    shared = shared + 1;
                }
                else
                {
                    a1 = (a1 + b0) / 2 + j;
                    b1 = (b1 + c0) / 2 + i;
                    d0 = (d0 + a1) / 2 + shared;
                    shared = shared - 1;
                }
                j = j - 1;
            }
            a2 = (a2 + d0) / 2 + shared;
            b2 = (b2 + c0) / 2 + i;
        }
        else
        {
            int k = 5;
            while ((k = k - 1) > 0)
            {
                if (k > 2)
                {
                    c2 = (c2 + a3) / 2 + k;
                    d2 = (d2 + b3) / 2 + shared;
                    shared = shared + i;
                }
                else
                {
                    c3 = (c3 + a2) / 2 + k;
                    d3 = (d3 + b2) / 2 + shared;
                    shared = shared - 1;
                }
                k = k - 1;
            }
            a3 = (a3 + d3) / 2 + shared;
            b3 = (b3 + c3) / 2 + i;
        }

        if (shared > 5)
        {
            d1 = (d1 + a0) / 2 + i;
            c1 = (c1 + b0) / 2 + shared;
        }
        else
        {
            d1 = (d1 + a3) / 2 + i;
            c1 = (c1 + b3) / 2 + shared;
        }
    }

    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3 + i + shared;
}
