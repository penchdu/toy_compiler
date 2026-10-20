int test()
{
    int a=0;

    while(a<10)
    {
        if(a==2)
        {
            while(a<20)
            {
                if(a==5)
                    break;

                a=a+1;
            }
        }

        a=a+1;
    }

    return a;
}