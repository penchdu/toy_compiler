int test()
{
    int a = 0;

    while(a < 20)
    {
        a = a + 1;

        if(a < 5)
            continue;

        if(a == 11)
            break;
    }

    return a;
}