int test(int dummy)
{
    // 前置变量填满所�?pr
    int a0 = 1;
    int a1 = 2;
    int a2 = 3;
    int a3 = 4;
    int a4 = 5;
    int a5 = 6;

    int i = 2;
    while (i > 0)
    {
        // cond: 读很多变�?
        // body: 密集写，制造大�?before有但now位置不同"的vr
        //       让很�?vr �?A≠B �?A 被别人占

        int t0 = a0 + a1;     // 临时变量 �?spill 循环释放
        int t1 = a2 + a3;     // 又一个临�?
        int t2 = a4 + a5;

        // 连环写：每个都换位置，互相占目标
        a0 = a2 + t0;   // a0 原来�?R10D，现在要搬到 R12D，但 R12D �?a2 占着�?
        a2 = a4 + t1;   // a2 要搬�?a4 的位置，�?a4 也被连环�?
        a4 = a0 + t2;   // a4 要搬�?a0 的位�?

        // 这三组形成环：a0→R12D(被a2�?, a2→R14D(被a4�?, a4→R10D(被a0�?
        // recover 主循环处理这三个时，全部进入情况 2a�?

        a1 = a1 + a3;
        a3 = a3 + a5;
        a5 = a5 + a1;

        i = i - 1;
    }

    return a0 + a1 + a2 + a3 + a4 + a5;
}
