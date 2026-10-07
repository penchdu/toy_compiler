int test()
{
    int a = 100;
    int b = 200;
    int i = 0;

    while (i < 20)
    {
        // 循环开头：a 的使用密度远高于 b
        a = a + 1;
        a = a + 2;
        a = a + 3;

        // 循环结尾：反转！b 的使用密度爆表，强制波浪模型在 Body 结束时将 R10 抢给 b，a 扔给 R11
        b = b + a;
        b = b + 1;
        b = b + 2;
        b = b + 3;
        b = b + 4;
        b = b + 5;

        i = i + 1;
    }

    return a + b;
}
