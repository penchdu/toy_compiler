int test(int dummy)
{
    int a=0;


    while(a<10)
    {
        a=a+1;


        if(a<3)
        {
            continue;
        }
        else
        {
            if(a==7)
                break;
        }
    }


    return a;
}