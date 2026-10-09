int test() 
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;
    int sum = 0;
    int i = 0;

    while (i + a < 100)
    {
        a = a + b;
        c = c + d;
        e = e + a + c;

        sum = sum + a + b + c + d + e;

        b = b + 1;
        d = d + 2;

        i = i + 1;
    }

    sum = sum + a + b + c + d + e;
    return sum;
}
