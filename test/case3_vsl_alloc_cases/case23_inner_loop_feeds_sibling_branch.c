int test(int dummy) {
    int i = 0; int j = 0;
    int a = 2; int b = 3; int c = 5; int d = 7; int e = 11; int f = 13;
    while ((i = i + 1) <= 4) {
        j = 0;
        while ((j = j + 1) <= 3) {
            if (j < i) {
                a = a + b;
                b = b + j;
            } else {
                c = c + a;
                d = d + i;
            }
        }
        if (a > d) {
            e = e + c;
            f = f + a;
        } else {
            c = c + e;
            d = d + f;
        }
        b = b + e;
    }
    return i + j + a + b + c + d + e + f;
}
