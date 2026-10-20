int test() {
    int i = 0; int j = 0; int k = 0; int m = 0;
    int a = 2; int b = 3; int c = 5; int d = 7; int e = 11; int f = 13;
    while ((i = i + 1) <= 2) {
        a = a + i;
        j = 0;
        while ((j = j + 1) <= 2) {
            b = b + a;
            k = 0;
            while ((k = k + 1) <= 2) {
                c = c + b;
                m = 0;
                while ((m = m + 1) <= 2) {
                    if (m < 2) {
                        d = d + c;
                        e = e + i;
                    } else {
                        f = f + d;
                        a = a + f;
                    }
                    b = b + m;
                }
                c = c + e;
            }
            d = d + f;
        }
        e = e + d;
        f = f + c;
    }
    return i + j + k + m + a + b + c + d + e + f;
}
