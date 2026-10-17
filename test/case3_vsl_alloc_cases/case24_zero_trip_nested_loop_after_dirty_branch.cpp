int test() {
    int i = 0; int j = 0;
    int a = 4; int b = 6; int c = 9; int d = 12; int flag = 0;
    while ((i = i + 1) <= 4) {
        if (i < 3) {
            a = a + b;
            j = 5;
            while ((j = j + 1) < 3) {
                c = c + j;
            }
            flag = flag + a;
        } else {
            j = 0;
            while ((j = j + 1) <= 2) {
                b = b + c;
                d = d + j;
            }
            flag = flag + d;
        }
        c = c + flag;
    }
    return i + j + a + b + c + d + flag;
}
