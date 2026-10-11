int test() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;
    int f = 6;
    int g = 7;
    int h = 8;
    int i = 100;

    while (i > 0) {
        a = (a + b) / 2;
        b = (b + c) / 2;
        c = (c + d) / 2;
        d = (d + e) / 2;
        e = (e + f) / 2;
        f = (f + g) / 2;
        g = (g + h) / 2;
        h = (h + a) / 2;
        i = i - 1;
    }

    return a + b + c + d + e + f + g + h + i;
}