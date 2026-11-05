int test(int dummy) {
    int i = 0;
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;
    while (i < 5) {
        if (a < b) {
            a = a + c;
            b = b + 1;
            if (i > 1) {
                d = d + a;
            } else {
                e = e + b;
            }
        } else {
            c = c + d;
            d = d + e;
        }
        i = i + 1;
    }
    return a + b + c + d + e + i;
}
