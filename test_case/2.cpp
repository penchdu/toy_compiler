int test()
{
    // 6 个变量恰好填满 R10D~R15D
    int a0 = 1;    // 初始假设在 R10D
    int a1 = 2;    // R11D
    int a2 = 3;    // R12D
    int a3 = 4;    // R13D
    int a4 = 5;    // R14D
    int a5 = 6;    // R15D

    // get_swap_pr 会 spill 其中一个（比如 a1 是 VR_USAGE_READ）
    // 腾出 swap_pr，pre_cond 里就这个空闲

    int result = 0;
    int i0 = 3;

    // ====== 外层：cond 里写 + body 环状交换 ======
    while ((i0 = i0 - 1 + (a0 = a0 + 1) - (a5 = a5 - 1)) > 0)
    {
        // body 零临时变量！所有写都是对已有 a0-a5
        // wave allocator 必须重新分配位置（密集冲突）
        // 假设 after_body 里：
        //   a0 现在在原来 a1 的位置（R11D）
        //   a1 现在在原来 a2 的位置（R12D）
        //   a2 → R13D, a3 → R14D, a4 → R15D, a5 → R10D

        int tmp = a0;     // 等等这产生临时了...不行
        a0 = a1;          // 直接赋值让 wave allocator 换位置
        a1 = a2;
        a2 = a3;
        a3 = a4;
        a4 = a5;
        a5 = tmp;

        // 上面 tmp 是个临时变量！让我换成纯算术：
        // 其实可以用 a0 = a0 + a1 - (a0 = a1)... 不对 C++ 没顺序

        // 好吧那就用纯算术表达式但赋回已有变量
        // 关键是让 6 个变量的位置全换了，而且没有新变量

        // 真正要的是：after_body_vr2pr 里 a0-a5 全部 A≠B
        // 且 A 被别的 vr 占着

        // 内层继续加压
        int i1 = 2;
        while ((i1 = i1 - 1 + (a2 = a2 + 1) - (a3 = a3 - 1)) > 0)
        {
            // 内层 cond 也写变量
            // body 继续密集写，制造更多位置交换

            // 用三元表达式绕开临时变量
            a0 = a0 + (a1 = a1 + 1) - 1;   // a1++，然后 a0 不变（+1-1）
            a2 = a2 + (a4 = a4 + 1) - 1;
            a5 = a5 + (a3 = a3 + 1) - 1;

            // 上面这些赋值表达式都是"写已有变量"，不产生新 vr
            // 但 wave allocator 必须处理冲突！

            int i2 = 2;
            while (i2 > 0)
            {
                // 更深层！继续让变量换位置
                // 所有赋值都用已有变量，不让 wave allocator 分配新 vr
                a0 = a0 + a1 - a1;      // 净零操作但触发 wave 分配
                a1 = a1 + a2 - a2;
                a2 = a2 + a3 - a3;
                a3 = a3 + a4 - a4;
                a4 = a4 + a5 - a5;
                a5 = a5 + a0 - a0;

                // 上面虽然是零操作，但每个 = 都是写！
                // wave allocator 必须重新分配
                // 如果 after_body 里 a0-a5 位置全乱了
                // 那 recover 主循环里 6 个 vr 都要处理

                i2 = i2 - 1;
            }

            i1 = i1 - 1;
        }

        result = result + a0 + a1 + a2 + a3 + a4 + a5;
    }

    return result;
}
