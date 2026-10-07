

int test() {
    int a = 1; int b = 2; int c = 3; int d = 4;
    int i = 3;
    while ((i = i - 1 + (a = a + 1) - (b = b - 1)) > 0) {
        c = c + d;
        d = d + a;
        b = b + c;
    }
    return a + b + c + d;
}
