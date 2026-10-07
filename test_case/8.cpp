int test()
{
    int a = 50;
    int b = 61;
    int c = 72;
    int d = 83;
    int e = 94;
    int i = 6;

    while (i > 0)
    {
        int j = i - (i / 3) * 3;

        while (j > 0)
        {
            int k = (i + j) - ((i + j) / 4) * 4;

            while (k > 0)
            {
                int t = (a + b + c + d + e) / 5;

                a = t + i + j + k;
                b = (a + c) / 2;
                c = (b + d) / 2;
                d = (c + e) / 2;
                e = (d + t) / 2;

                k = k - 1;
            }

            a = (a + d) / 2;
            e = (e + b) / 2;

            j = j - 1;
        }

        b = (b + c) / 2;
        d = (d + e) / 2;

        i = i - 1;
    }

    return a + b + c + d + e + i;
}
