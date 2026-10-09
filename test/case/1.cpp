int test()
{
    // 关键：声明前先让某个 pr 被上层代码占了
    // 这样 get_swap_pr 必须 spill 一个 vr 腾出空间

    // 先声明一些填满 pr 的变量
    int x0 = 100;
    int x1 = 200;
    int x2 = 300;
    int x3 = 400;
    int x4 = 500;
    int x5 = 600;
    // 6 个 pr 全满！get_swap_pr 必须 spill 一个

    // 再声明一个变量只在 while 里用
    int trigger = 0;

    int i = 2;
    while (i > 0)
    {
        // cond: 读 trigger 但写 x1
        // 这会让 wave allocator 把 x1（被 get_swap_pr spill 过的）reload 回来
        // 可能恰好放 swap_pr！

        // body: 密集操作让 x0-x5 位置全换
        trigger = trigger + x1;    // 读 x1（被 reload 到 swap_pr 了？）

        // 下面让变量环移：原来在 swap_pr 的 x1 要搬到别的地方
        x0 = x1;     // 等等这又要临时...
        // 算了，换个思路

        // 真正关键：让某个 vr 的 before.pr 恰好等于 swap_pr
        // 即 pre_cond_vr2pr[某vr].pr == swap_pr
        // 这发生在：wave allocator 在 cond alloc 时，把 pre_cond 里
        // 原本在 swap_pr（空）的某个 vr（被 get_swap_pr spill 的那个）reload 回 swap_pr

        x0 = x0 + x2;
        x1 = x1 + x3;
        x2 = x2 + x4;
        x3 = x3 + x5;
        x4 = x4 + x0;
        x5 = x5 + x1;

        // 上面每个操作都让 wave allocator 换位置
        // 假设最后 x1 从 swap_pr 被搬到了 R12D
        // recover 主循环处理 x1：
        //   A = pre_cond_vr2pr[x1].pr = swap_pr  ← 💀 目标就是 swap_pr！
        //   B = after_body_vr2pr[x1].pr = R12D
        //   curr_vr_have_A = now_pr2vr[swap_pr] → 可能是某个 cond 临时变量或空
        //
        //   情况 2a 里动态找 use_swap → 必须排除 A(swap_pr) 和 B(R12D) ✓
        //   但如果唯一的空闲就是 swap_pr 呢？！ use_swap 查找排除了 A=swap_pr
        //   → use_swap = 某个别的空闲 pr...如果没有别的空闲？💥

        i = i - 1;
    }

    return x0 + x1 + x2 + x3 + x4 + x5 + trigger;
}
