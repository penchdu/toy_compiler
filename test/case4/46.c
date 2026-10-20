int test(int dummy)
{
    int a = 0;

    while (a < 5)
    {
        {
            a = a + 1;

            break;

            // 不应该生�?TAC
            a = a + 100;
        }

        // 也不应该生成 TAC
        a = a + 2;
    }

    // while 结束后，这句必须继续生成
    a = a + 10;

    return a;
}