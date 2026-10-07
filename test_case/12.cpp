int test()
{
    int a = 1; int b = 2; int c = 3; int d = 4;
    int e = 5; int f = 6; int g = 7; int h = 8;
    int i = 9; int j = 10; int k = 11; int l = 12;
    int sum = 0;
    int loop = 0;

    while (loop < 10)
    {
        // 强制 12 个变量同时处于 Live 状态并进行复杂交叉计算
        a = a + b + c + d + e + f + g + h + i + j + k + l;
        b = b + a; c = c + b; d = d + c; e = e + d;
        f = f + e; g = g + f; h = h + g; i = i + h;
        j = j + i; k = k + j; l = l + k;

        loop = loop + 1;
    }

    sum = a + b + c + d + e + f + g + h + i + j + k + l;
    return sum;
}
