//int c = 0;
//int c2 = 1;

int test(int dummy)
{
	int a = 1;
	
	if(a < 0)		//  2
		return a;
	
	//  4
	
	if(a = 1)			//  2 1
	{					//  3 1
	
		if(a == 2)		//  4 3
		{					// 5 3 
			a = 3;
			
			if(a == 4)		// 6 5
			{					//  7 5
				a = 4;
			}
			a=5;			// 8 5
		}
							//  9 3
		return a + 1;
		
		{
			a=6;
		}
	}
							//  10 1
	return a + a;
		
	{	
		return 0;
	}
}