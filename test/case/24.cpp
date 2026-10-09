int test()
{
    int a0 = 1;
    int a1 = 2;
    int a2 = 3;
    int a3 = 4;
    int a4 = 5;
    int a5 = 6;
    int a6 = 7;
    int a7 = 8;

    /*
     * 故意让 a0~a7 在进入 while 前最后一次使用是 READ，
     * 这样它们的 u 都尽量保持 READ。
     */
    int sink = a0 + a1 + a2 + a3
             + a4 + a5 + a6 + a7;

    int i = 3;

    while (i > 0)
    {
        /*
         * 先全部读，再全部写。
         *
         * 如果 get_swap_pr fallback 错误地 spill 掉某个 aX
         * 而没有把最新值存栈，那么这里的 t 会读到旧值。
         */
        int t = a0 + a1 + a2 + a3
              + a4 + a5 + a6 + a7;

        a0 = t + 1;
        a1 = t + 2;
        a2 = t + 3;
        a3 = t + 4;
        a4 = t + 5;
        a5 = t + 6;
        a6 = t + 7;
        a7 = t + 8;

        i = i - 1;
    }

    return a0 + a1 + a2 + a3
         + a4 + a5 + a6 + a7
         + sink * 0;
}
