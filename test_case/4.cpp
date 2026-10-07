int test()
{
    // 6 个变量填满所有 pr
    int a0 = 100;
    int a1 = 200;   // ← 外层 get_swap_pr 会 spill 这个
    int a2 = 300;
    int a3 = 400;
    int a4 = 500;
    int a5 = 600;

    int i0 = 2;
    int result = 0;

    while (i0 > 0)
    {
        // 外层 cond 读 a1（被 spill 到栈），wave allocator reload 后可能放 swap_pr
        // 外层 body...
        a0 = a0 + a1;
        a2 = a2 + a3;

        int i1 = 2;
        while (i1 > 0)
        {
            // 内层 get_swap_pr 又要 spill 某个...
            // 内层 cond/body 里的临时变量可能又占了外层的 swap_pr
            a1 = a1 + a2;    // 内层写 a1
            a3 = a3 + a4;

            int i2 = 2;
            while (i2 > 0)
            {
                // 最深层密集操作，制造最大寄存器压力
                // 多个临时变量同时存活，spill 循环释放后，
                // recover 主循环需要连续做 3 寄存器交换
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
