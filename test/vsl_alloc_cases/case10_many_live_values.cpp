int test() {
    int i = 0;
    int a = 2;
    int b = 3;
    int c = 5;
    int d = 7;
    int e = 11;
    int f = 13;
    int g = 17;
    int h = 19;
    int carry = 23;
    while (i < 5) {
        if (i > 2) {
            a = a + d;
            c = c + f;
            e = e + a;
        } else {
            b = b + e;
            d = d + g;
            f = f + b;
        }
        carry = carry + a - b + c - d;
        g = g + carry;
        h = h + g;
        i = i + 1;
    }
    return a + b + c + d + e + f + g + h + carry + i;
}
