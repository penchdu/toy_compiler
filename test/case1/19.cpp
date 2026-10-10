int test()
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;
    int f = 6;

    int sink = a + b + c + d + e + f;

    int i = 4;

    while (i > 0)
    {
        int t = a + b + c + d + e + f;

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
