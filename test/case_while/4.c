int test(int dummy)
{
    int a0 = 100;
    int a1 = 200; 
    int a2 = 300;
    int a3 = 400;
    int a4 = 500;
    int a5 = 600;

    int i0 = 2;
    int result = 0;

    while (i0 > 0)
    {
        a0 = a0 + a1;
        a2 = a2 + a3;

        int i1 = 2;
        while (i1 > 0)
        {
            a1 = a1 + a2;  
            a3 = a3 + a4;

            int i2 = 2;
            while (i2 > 0)
            {
                int t0 = a0 + a1;
                int t1 = a2 + a3;
                int t2 = a4 + a5;

                a0 = t0 + a2;
                a2 = t1 + a4;
                a4 = t2 + a0;

                t0 = a1 + a3;
                t1 = a5 + a0;
                t2 = a2 + a4;

                a1 = t0 + a5;
                a3 = t1 + a1;
                a5 = t2 + a3;

                i2 = i2 - 1;
            }

            a1 = a1 + a3;
            a3 = a3 + a5;
            a5 = a5 + a1;

            i1 = i1 - 1;
        }

        result = result + a0 + a1 + a2 + a3 + a4 + a5;

        i0 = i0 - 1;
    }

    return result;
}
