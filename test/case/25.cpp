int test()
{
    int keep0 = 101;
    int keep1 = 203;

    int a0 = 1;
    int a1 = 2;
    int a2 = 3;
    int a3 = 4;
    int a4 = 5;
    int a5 = 6;

    int b0 = 7;
    int b1 = 8;
    int b2 = 9;
    int b3 = 10;
    int b4 = 11;
    int b5 = 12;

    int c0 = 13;
    int c1 = 14;
    int c2 = 15;
    int c3 = 16;

    int i = 3;

    while (i > 0)
    {
        /*
         * 这些变量在 while cond 前已经长期存在，
         * 希望部分变量占据 before state 的 PR。
         */
        int hold = a0 + b0;

        a0 = a0 + b1;
        a1 = a1 + b2;
        a2 = a2 + b3;
        a3 = a3 + b4;

        b0 = b0 + c0;
        b1 = b1 + c1;
        b2 = b2 + c2;
        b3 = b3 + c3;

        c0 = c0 + a4;
        c1 = c1 + a5;
        c2 = c2 + b4;
        c3 = c3 + b5;

        /*
         * 制造大量 body-only VR，
         * 让它们在 now 出现、before 不存在。
         */
        int d0 = a0 + c1;
        int d1 = a1 + c2;
        int d2 = a2 + c3;
        int d3 = a3 + c0;
        int d4 = a4 + c1;
        int d5 = a5 + c2;

        d0 = d0 + d3;
        d1 = d1 + d4;
        d2 = d2 + d5;

        a4 = a4 + d0;
        a5 = a5 + d1;
        b4 = b4 + d2;
        b5 = b5 + d3;

        hold = hold + d4 + d5;

        /*
         * 再让原来的变量继续活到 while 下一轮，
         * 强迫 recovery 真正恢复 before state。
         */
        a0 = a0 + hold;
        a1 = a1 + d0;
        a2 = a2 + d1;
        a3 = a3 + d2;

        i = i - 1;
    }

    return keep0 + keep1
         + a0 + a1 + a2 + a3 + a4 + a5
         + b0 + b1 + b2 + b3 + b4 + b5
         + c0 + c1 + c2 + c3;
}
