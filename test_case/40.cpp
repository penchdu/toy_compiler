int test()
{
    int a0 = 1; int a1 = 2; int a2 = 3;
    int b0 = 4; int b1 = 5; int b2 = 6;
    int c0 = 7; int c1 = 8; int c2 = 9;
    int d0 = 10; int d1 = 11; int d2 = 12;
    int e0 = 13; int e1 = 14; int e2 = 15;
    int f0 = 16; int f1 = 17; int f2 = 18;

    int i0 = 2;

    while (i0 > 0)
    {
        // level 0: first permutation
        a0 = a0 + b1;
        b1 = b1 + c2;
        c2 = c2 + d0;
        d0 = d0 + e1;
        e1 = e1 + f2;
        f2 = f2 + a0;

        int i1 = 2;

        while (i1 > 0)
        {
            // level 1
            a1 = a1 + c0;
            c0 = c0 + e2;
            e2 = e2 + b0;
            b0 = b0 + d1;
            d1 = d1 + f0;
            f0 = f0 + a1;

            int i2 = 2;

            while (i2 > 0)
            {
                // level 2
                a2 = a2 + d2;
                d2 = d2 + f1;
                f1 = f1 + c1;
                c1 = c1 + e0;
                e0 = e0 + b2;
                b2 = b2 + a2;

                int i3 = 2;

                while (i3 > 0)
                {
                    // level 3
                    a0 = a0 + d1;
                    b1 = b1 + e2;
                    c2 = c2 + f0;

                    d0 = d0 + a1;
                    e1 = e1 + b0;
                    f2 = f2 + c0;

                    int i4 = 2;

                    while (i4 > 0)
                    {
                        // level 4
                        a1 = a1 + e0;
                        b0 = b0 + f1;
                        c0 = c0 + a2;

                        d1 = d1 + b2;
                        e2 = e2 + c1;
                        f0 = f0 + d2;

                        int i5 = 2;

                        while (i5 > 0)
                        {
                            // level 5: deliberately scramble the same
                            // outer live values again
                            a2 = a2 + f2;
                            b2 = b2 + e1;
                            c1 = c1 + d0;

                            d2 = d2 + c2;
                            e0 = e0 + a0;
                            f1 = f1 + b1;

                            // second permutation in the SAME iteration
                            a0 = a0 + f1;
                            b1 = b1 + a2;
                            c2 = c2 + b2;

                            d0 = d0 + c1;
                            e1 = e1 + d2;
                            f2 = f2 + e0;

                            i5 = i5 - 1;
                        }

                        // unwind permutation 1
                        a1 = a1 + d2;
                        b0 = b0 + e0;
                        c0 = c0 + f1;

                        d1 = d1 + a2;
                        e2 = e2 + b2;
                        f0 = f0 + c1;

                        i4 = i4 - 1;
                    }

                    // unwind permutation 2
                    a2 = a2 + e2;
                    b2 = b2 + f0;
                    c1 = c1 + a1;

                    d2 = d2 + b0;
                    e0 = e0 + c0;
                    f1 = f1 + d1;

                    i3 = i3 - 1;
                }

                // unwind permutation 3
                a0 = a0 + c1;
                b1 = b1 + d2;
                c2 = c2 + e0;

                d0 = d0 + f1;
                e1 = e1 + a2;
                f2 = f2 + b2;

                i2 = i2 - 1;
            }

            // unwind permutation 4
            a1 = a1 + f2;
            b0 = b0 + a0;
            c0 = c0 + b1;

            d1 = d1 + c2;
            e2 = e2 + d0;
            f0 = f0 + e1;

            i1 = i1 - 1;
        }

        // outer unwind
        a2 = a2 + b0;
        b2 = b2 + c0;
        c1 = c1 + d1;

        d2 = d2 + e2;
        e0 = e0 + f0;
        f1 = f1 + a1;

        i0 = i0 - 1;
    }

    return a0 + a1 + a2
         + b0 + b1 + b2
         + c0 + c1 + c2
         + d0 + d1 + d2
         + e0 + e1 + e2
         + f0 + f1 + f2;
}
