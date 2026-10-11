int test() {
    int n = 0; int total = 0; int x = 3; int y = 7; int z = 11;
    while ((n = n + 1) <= 7) {
        if (n < 4) {
            total = total + n;
            x = x + y;
            n = n + 1;
        } else {
            total = total + x;
            y = y + z;
        }
        z = z + total;
    }
    return n + total + x + y + z;
}
