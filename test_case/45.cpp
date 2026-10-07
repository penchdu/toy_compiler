int test() {
    int a = 1;
    int b = 200;
    int sum = 0;
    int i = 0;

    if (a < b) {
        int a = 10;        // 裸块 shadow
        int e = 5;
        while (i < 120) {
            if (a < 50) {    // a 找裸块的 100
                break;
            }
            if (b > 0) {
                i = i + 1;
                continue;
            }
            sum = sum + a + e;   // 正常路径
            i = i + 1;
        }
    }
    // 块外恢复：a 又回到 1
    sum = sum + a;

    while (i < 100) {
        int e = 12;
        while (i < 10) {
            int a = 1;
            a = a + e;
            if (i > 5) break;
            i = i + a;
        }
        i = i + 1;
		sum = sum + i;
    }
    return sum;
}
