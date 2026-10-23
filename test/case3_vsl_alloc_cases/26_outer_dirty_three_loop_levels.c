int test(int dummy)
{
    int x = 5;
    int p = 10;
    int q = 0;
    int r = 0;
    int a = 3;
    int b = 6;

    while ((p = p - 1) + x > 0)
    {
        q = 3;
        while ((q = q - 1) > 0)
        {
            r = 3;
            while ((r = r - 1) > 0)
            {
                a = (a + b + a + b + a) / 5 + 1;
                b = (b + a + b + a + b) / 5 + 2;
            }
        }

        x = x - 1;
        p = p - 1;
    }

    return p * 100 + a + b;
}
