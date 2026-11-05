int test(int dummy) {
    int i = 0; int j = 0; int k = 0;
    int a = 2; int b = 4; int c = 6; int d = 8; int e = 10; int f = 12;
    int p = 14; int q = 16; int r = 18; int s = 20;
    while ((i = i + 1) <= 3) {
        if (i < 3) {
            j = 0;
            while ((j = j + 1) <= 2) {
                a = a + b;
                c = c + d;
                p = p + j;
            }
            if (a > c) {
                e = e + p;
                q = q + a;
            } else {
                f = f + q;
                r = r + c;
            }
        } else {
            k = 0;
            while ((k = k + 1) <= 3) {
                b = b + c;
                d = d + e;
                s = s + k;
            }
            if (b > d) {
                p = p + s;
                e = e + b;
            } else {
                q = q + s;
                f = f + d;
            }
        }
        a = a + r;
        c = c + s;
        e = e + q;
    }
    return i + j + k + a + b + c + d + e + f + p + q + r + s;
}
