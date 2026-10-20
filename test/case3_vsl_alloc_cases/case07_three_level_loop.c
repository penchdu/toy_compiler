int test(int dummy) {
    int i = 0;
    int j = 0;
    int k = 0;
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;
    int f = 6;
    int carry = 7;
    while (i < 3) {
        j = 0;
        while (j < 2) {
            k = 0;
            while (k < 3) {
                if (i + j + k > 2) {
                    a = a + d;
                    e = e + 1;
                    carry = carry + a;
                } else {
                    b = b + c;
                    f = f + b;
                    carry = carry - f;
                }
                k = k + 1;
            }
            if (j > 0) {
                d = d + e;
            } else {
                c = c + f;
            }
            j = j + 1;
        }
        if (i > 1) {
            a = a + carry;
        } else {
            b = b + carry;
        }
        i = i + 1;
    }
    return a + b + c + d + e + f + carry + i + j + k;
}
