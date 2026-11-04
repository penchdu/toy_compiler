int test(int dummy)
{
    // 填满 6 �?pr
    int a0 = 1;
    int a1 = 2;
    int a2 = 3;
    int a3 = 4;
    int a4 = 5;
    int a5 = 6;

    int i = 2;
    while (i > 0)
    {
        // cond 里读 a0-a5（全�?6 个），wave allocator 重新分配
        // body 里密集写 + 密集产生临时变量（spill 循环释放�?
        // 关键：让多个"pre_cond 有但 after_body �?spill"的变量存�?
        //       且它们的 A 都被现在别的 vr 占着

        int t0 = a0 + a1;
        int t1 = a2 + a3;
        int t2 = a4 + a5;
        int t3 = t0 + t1;
        int t4 = t1 + t2;
        int t5 = t3 + t4;

        // �?6 个临时变量会�?spill 循环释放（因�?before.pr==MAX�?
        // 释放 6 �?pr！然后主循环恢复 a0-a5 时，
        // 有些 a �?A 可能被这些临时变量残留占用，触发情况 3

        a0 = t3;     // �?a0 换位�?
        a2 = t4;
        a4 = t5;

        // 这些也让位置变化
        a1 = a0 + t1;
        a3 = a2 + t2;
        a5 = a4 + t0;

        i = i - 1;
    }

    return a0 + a1 + a2 + a3 + a4 + a5;
}
