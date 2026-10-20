int test()
{
    int a0 = 3; int a1 = 5; int a2 = 7; int a3 = 9;
    int b0 = 4; int b1 = 6; int b2 = 8; int b3 = 10;
    int c0 = 11; int c1 = 13; int c2 = 15; int c3 = 17;
    int d0 = 12; int d1 = 14; int d2 = 16; int d3 = 18;
    int e0 = 19; int e1 = 20; int e2 = 21; int e3 = 22;

    int outer = 5;
    while ((outer = outer - 1) > 0)
    {
        a0 = (a0 + e3) / 2 + 1; a1 = (a1 + e2) / 2 + 2; b0 = (b0 + e1) / 2 + 2; b1 = (b1 + e0) / 2 + 3;
        int mid = 4;
        while ((mid = mid - 1) > 0)
        {
            c0 = (c0 + a1) / 2 + 3; c1 = (c1 + a0) / 2 + 4;
            d0 = (d0 + b1) / 2 + 4; d1 = (d1 + b0) / 2 + 5;
            int inner = 4;
            while ((inner = inner - 1) > 0)
            {
                e0 = (e0 + c0) / 2 + 5; e1 = (e1 + c1) / 2 + 6;
                e2 = (e2 + d0) / 2 + 7; e3 = (e3 + d1) / 2 + 8;
                a2 = (a2 + e0) / 2 + 3; a3 = (a3 + e1) / 2 + 4;
                b2 = (b2 + e2) / 2 + 4; b3 = (b3 + e3) / 2 + 5;

                int leaf = 3;
                while ((leaf = leaf - 1) > 0)
                {
                    c2 = (c2 + a3) / 2 + 5; c3 = (c3 + a2) / 2 + 6;
                    d2 = (d2 + b3) / 2 + 6; d3 = (d3 + b2) / 2 + 7;
                    a0 = (a0 + d2) / 2 + 1; b0 = (b0 + c3) / 2 + 2;
                    a1 = (a1 + d3) / 2 + 2; b1 = (b1 + c2) / 2 + 3;
                }
                e0 = (e0 + b2) / 2 + 5; e1 = (e1 + b3) / 2 + 6;
            }
            c0 = (c0 + d3) / 2 + 3; d0 = (d0 + c3) / 2 + 4;
        }
        a2 = (a2 + b1) / 2 + 3; b2 = (b2 + c1) / 2 + 4;
        c2 = (c2 + d1) / 2 + 5; d2 = (d2 + a1) / 2 + 6;
    }
    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
         + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3
         + e0 + e1 + e2 + e3 + outer;
}
