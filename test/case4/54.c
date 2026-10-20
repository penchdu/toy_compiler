int test(int dummy)
{
    int a=0;

    while(a<10)
    {
        a=a+1;

        if(a==5)
        {
            break;
        }

        a=a+1;
    }


    while(a<15)
    {
        a=a+1;
    }

    return a;
}