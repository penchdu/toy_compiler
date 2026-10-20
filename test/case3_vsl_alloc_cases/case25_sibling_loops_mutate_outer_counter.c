int test() {
    int i = 0; int k = 0;
    int a = 1; int b = 2; int c = 4; int d = 8; int sum = 3;
    while ((i = i + 1) <= 3) {
        k = 0;
        while ((k = k + 1) <= 2) {
            a = a + i;
            b = b + k;
        }
        k = 0;
        while ((k = k + 1) <= 3) {
            c = c + a;
            if (k < 2) {
                i = i + 1;
            } else {
                b = b + c;
            }
            d = d + k;
        }
        sum = sum + a + b;
    }
    return i + k + a + b + c + d + sum;
}
