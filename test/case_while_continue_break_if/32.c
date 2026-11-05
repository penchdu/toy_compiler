int test(int dummy)
{
    int a = 0;
    int b = 0;

    while(a < 10)
    {
        a=a+1;

        if(a < 5)
        {
            continue;
            b=b+100;
        }

        b=b+1;
    }

    return b;
}