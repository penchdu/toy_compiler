int test(int dummy) {
    int t = 0; int n = 0;
    int a = 4; int b = 6; int c = 8; int d = 10; int result = 1;
    while ((t = t + 1) < 9) {
        if (t < 4) {
            t = t + 1;
            a = a + b;
            c = c + t;
        } else {
            b = b + t;
            d = d + a;
        }
        if (t < 6) {
            result = result + c;
        } else {
            result = result + d;
        }
        n = n + 1;
    }
    return t + n + a + b + c + d + result;
}
