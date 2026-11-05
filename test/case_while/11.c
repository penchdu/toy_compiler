int test(int dummy)
{
    int a = 100;
    int b = 200;
    int i = 0;

    while (i < 20)
    {
        a = a + 1;
        a = a + 2;
        a = a + 3;

        b = b + a;
        b = b + 1;
        b = b + 2;
        b = b + 3;
        b = b + 4;
        b = b + 5;

        i = i + 1;
    }

    return a + b;
}
