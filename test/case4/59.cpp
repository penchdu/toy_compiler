int test()
{
    int a = 10;
    int b = 20;
    int c = 0;

    if (a < b)
        c = 1;
    else
        c = 2;

    if (c == 1)
    {
        if (b > a)
            c = c + 10;
        else
            c = c + 20;
    }
    else
        c = c + 30;

    if (a > b)
        c = 100;
    else
        c = c + 5;

    return c;
}