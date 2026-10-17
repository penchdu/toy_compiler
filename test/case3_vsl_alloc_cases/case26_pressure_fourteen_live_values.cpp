int test() {
    int i = 0; int j = 0;
    int a = 1; int b = 2; int c = 3; int d = 4; int e = 5; int f = 6; int g = 7;
    int h = 8; int p = 9; int q = 10; int r = 11; int s = 12; int t = 13; int u = 14;
    while ((i = i + 1) <= 3) {
        if (i < 3) {
            a = a + h;
            b = b + p;
            c = c + q;
            d = d + r;
            e = e + s;
        } else {
            f = f + t;
            g = g + u;
            h = h + a;
            p = p + b;
            q = q + c;
        }
        j = 0;
        while ((j = j + 1) <= 2) {
            if (j < i) {
                r = r + d;
                s = s + e;
                t = t + f;
            } else {
                u = u + g;
                a = a + j;
                b = b + u;
            }
        }
        c = c + t;
        d = d + s;
        e = e + r;
    }
    return i + j + a + b + c + d + e + f + g + h + p + q + r + s + t + u;
}
