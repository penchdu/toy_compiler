int test()
{
    int a = 17;
    int b = 23;
    int c = 31;
    int d = 43;
    int e = 59;
    int f = 71;
    int g = 83;
    int h = 97;

    int i = 6;

    while (i > 0)
    {
        int j = i - (i / 2) * 2;

        while (j > 0)
        {
            int t0 = a;
            int t1 = b;
            int t2 = c;
            int t3 = d;
            int t4 = e;
            int t5 = f;
            int t6 = g;
            int t7 = h;

            int q = (t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7) / 8;

            a = (t7 + q) / 2 + i;
            b = (t0 + q) / 2 + 1;
            c = (t1 + q) / 2 + 2;
            d = (t2 + q) / 2 + 3;
            e = (t3 + q) / 2 + 4;
            f = (t4 + q) / 2 + 5;
            g = (t5 + q) / 2 + 6;
            h = (t6 + q) / 2 + 7;

            j = j - 1;
        }

        int k = 3;

        while (k > 0)
        {
            int q = (a + h) / 2;
            int r = (b + g) / 2;
            int s = (c + f) / 2;
            int t = (d + e) / 2;

            a = q + i + k;
            b = r + a;
            c = s + b;
            d = t + c;
            e = (q + d) / 2;
            f = (r + e) / 2;
            g = (s + f) / 2;
            h = (t + g) / 2;

            k = k - 1;
        }

        int q = (a + c + e + g) / 4;
        int r = (b + d + f + h) / 4;

        a = q + i;
        c = (q + r) / 2;
        e = (r + c) / 2;
        g = (q + e) / 2;

        i = i - 1;
    }

    return a + b + c + d + e + f + g + h + i;
}
