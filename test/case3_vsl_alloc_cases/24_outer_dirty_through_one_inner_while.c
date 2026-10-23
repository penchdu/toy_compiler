int test(int dummy)
{
    int x = 5;
    int p = 9;
    int a = 3;
    int b = 5;
    int q = 0;

    while ((p = p - 1) + x > 0)
    {
        q = 4;
        while ((q = q - 1) > 0)
        {
            a = (a + b) / 2 + (a + 1) / 2;
            b = (b + a) / 2 + (b + 2) / 2;
        }

        x = x - 2;
        p = p - 1;
    }

    return p * 100 + a + b;
}
