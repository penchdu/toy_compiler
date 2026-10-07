int test()
{
    int a0 = 1;   // 填满 R10D
    int a1 = 2;   // R11D  ← get_swap_pr 会 spill 这个（VR_USAGE_READ）
    int a2 = 3;   // R12D
    int a3 = 4;   // R13D
    int a4 = 5;   // R14D
    int a5 = 6;   // R15D

    // 这里把 a1 标记为只读后（get_swap_pr 要 spill 它腾出 pr）
    // 但 spill_vr 不更新 pr2vr → pre_cond 里 a1.pr == swap_pr！！

    int i = 3;
    while (i > 0)
    {
        // cond: 读 a1（触发 reload，让 wave allocator 把 a1 挪走）
        // body: 把 a1 从 swap_pr 挪到别的 pr（比如 R12D）
        // recover 时：
        //   A = before_vr2pr[a1].pr = swap_pr  ← 💀 目标就是 swap_pr
        //   B = now_vr2pr[a1].pr = R12D       ← a1 在这
        //   curr_vr_have_A = now_pr2vr[swap_pr]  ← swap_pr 被 body 里临时变量占了
        //   → 进入情况 2a → assert(now_pr2vr[swap_pr].vr==INVALID) 崩！！
        a0 = a0 + a1;
        a1 = a1 + a2;    // 让 a1 被写
        a2 = a2 + a3;
        a3 = a3 + a4;
        a4 = a4 + a5;
        a5 = a5 + a0;

        i = i - 1;
    }

    return a0 + a1 + a2 + a3 + a4 + a5;
}
