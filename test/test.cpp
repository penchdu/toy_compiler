/*
 * test.cpp
 *
 *  Created on: 2026年9月19日
 *      Author: x
 */

#include <stdio.h>

int f2()
{
    int a = 8;
    int b = 3;
    int c = 0;

    if (a > b)
    {
        int b = 10;
        c = a + b;

        if (c > 15)
        {
            int a = 2;
            c = c + a;
        }
        else
        {
            c = c - 1;
        }

        b = c - 5;
    }
    else
    {
        c = a - b;
    }

    {
        int a = 4;
        int c = 7;

        if (a < c)
        {
            int b = 2;
            c = c + b;

            if (c >= a)
                a = a + 3;
        }

        b = b + c;
    }

    if (c > a)
    {
        int c = 2;
        c = c + b;

        if (c < a)
            b = b + c;
        else
            b = b - c;

        if (b == 8)
            a = a + 10;
    }
    else
    {
        c = c + 100;
    }


    return a + b + c;
}

int f()
{
	int a = 1;
	int b = 1;

	if (a == 1)
		if (2)
			if (3)
			{
				if (b == 2)
				{
					a = 3;
				}
				else if (b == 3)
					if (2)
						if (3)
						{
							a = 4;
						}
				if (2)
					if (3)
						2;
					else
					{
						a = 5;
					}
			}
			else if (a == 2)
			{
				a = 6;
			}
			else
			{
				a = 7;
			}
	if (1)
		if (2)
			if (3)
				if (4)
					2;
				else if (5)
					if (6)
						0;
					else
						(1);

	return 0;
}

int f3()
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
void test()
{
	int r = f3();

	printf("test: %d\n", r);
}

