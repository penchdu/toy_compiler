//10
int test(int dummy)
{
    int a=0;
    int b=0;


    while(a<5)
    {
        a=a+1;

        while(b<5)
        {
            b=b+1;

            if(b==3)
                continue;

            b=b+1;
        }
    }


    return b+a;
}