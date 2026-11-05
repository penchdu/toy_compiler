int test(int dummy)
{
    int a0 = 1; 
    int a1 = 2; 
    int a2 = 3; 
    int a3 = 4; 
    int a4 = 5; 
    int a5 = 6; 

    int result = 0;
    int i0 = 3;

    while ((i0 = i0 - 1 + (a0 = a0 + 1) - (a5 = a5 - 1)) > 0)
    {
        int tmp = a0; 
        a0 = a1;      
        a1 = a2;
        a2 = a3;
        a3 = a4;
        a4 = a5;
        a5 = tmp;
        
        int i1 = 2;
        while ((i1 = i1 - 1 + (a2 = a2 + 1) - (a3 = a3 - 1)) > 0)
        {
            a0 = a0 + (a1 = a1 + 1) - 1;  
            a2 = a2 + (a4 = a4 + 1) - 1;
            a5 = a5 + (a3 = a3 + 1) - 1;

            int i2 = 2;
            while (i2 > 0)
            {
                a0 = a0 + a1 - a1;      // 净零操作但触发 wave 分配
                a1 = a1 + a2 - a2;
                a2 = a2 + a3 - a3;
                a3 = a3 + a4 - a4;
                a4 = a4 + a5 - a5;
                a5 = a5 + a0 - a0;

                i2 = i2 - 1;
            }

            i1 = i1 - 1;
        }

        result = result + a0 + a1 + a2 + a3 + a4 + a5;
    }

    return result;
}
