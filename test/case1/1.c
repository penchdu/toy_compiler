int test(int dummy)
{
    int x0 = 100;
    int x1 = 200;
    int x2 = 300;
    int x3 = 400;
    int x4 = 500;
    int x5 = 600;
    int trigger = 0;
    int i = 2;
	
    while (i > 0)
    {
        x0 = x1;    
        x0 = x0 + x2;
        x1 = x1 + x3;
        x2 = x2 + x4;
        x3 = x3 + x5;
        x4 = x4 + x0;
        x5 = x5 + x1;

        i = i - 1;
    }

    return x0 + x1 + x2 + x3 + x4 + x5 + trigger;
}
