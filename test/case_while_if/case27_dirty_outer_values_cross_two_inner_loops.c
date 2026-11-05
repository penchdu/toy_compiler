int test(int dummy) {
    int i = 0; int j = 0; int k = 0;
    int a = 3; int b = 5; int c = 7; int d = 9; int e = 11; int out = 1;
    while ((i = i + 1) <= 3) {
        a = a + i;
        j = 0;
        while ((j = j + 1) <= 2) {
            b = b + a;
            if (j < 2) {
                c = c + b;
            } else {
                d = d + c;
            }
        }
        k = 0;
        while ((k = k + 1) < 3) {
            if (k < i) {
                e = e + d;
                a = a + e;
            } else {
                c = c + e;
                b = b + c;
            }
        }
        if (a > e) {
            out = out + b + d;
        } else {
            out = out + c + e;
        }
    }
    return i + j + k + a + b + c + d + e + out;
}
