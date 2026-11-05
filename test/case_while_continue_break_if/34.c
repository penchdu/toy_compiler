int test(int dummy)
{
    int a=1;

    while(a<7)
    {
        if(a==3)
        {
			a=10;
           continue;
        }

        if(a==5)
        {
			a=11;
            break;
        }

        a=a+1;
    }

    return a;
}