int test() 
{
    int a = 1;
    int b = 2;
    int i = 0;
	int sum = 0;
	
    while (i < 100)
    {
        a = a + b;
        b = b + 1;
        sum = sum + i;
		i = i + 1;
		
		
		while (i < 50)
		{
			a = a + 2;
			sum = sum + a;
			i = i + 1;
		}
    }

    sum = sum + a;
    return sum;
}
