int test()
{
    int a=0;
    int b=0;

    while(a<5)
    {
        a=a+1;

        while(b<10)
        {
            b=b+1;

            if(b==3)
                break;
        }

        b=b+10;
    }

    return b;
}