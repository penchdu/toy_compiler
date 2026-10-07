int test()
{
    int a = 3;
    int b = 5;
    int c = 7;
    int d = 11;
    int i = 5;

    while (i > 0)
    {
        a = a + b;
        b = b + c;
        c = c + d;
        d = d + a;
        i=i-1;
    }

    int x = a + c;

    int j = 4;

    while (j > 0)
    {
        c = c + d;
        d = d + a;
        a = a + b;
        b = b + x;
        j=j-1;
    }

    return a + b + c + d + x;
}
