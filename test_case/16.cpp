int test()
{
    int a = 100;
    int b = 203;
    int c = 305;
    int d = 407;
    int e = 509;
    int f = 611;
    int i = 7;

    while (i > 0)
    {
        int t = (a + b + c + d + e + f) / 7;

        a = t + 1;
        b = t + 2;
        c = t + 3;
        d = t + 4;
        e = t + 5;
        f = t + 6;

        i = i - 1;
    }

    return a + b + c + d + e + f;
}
