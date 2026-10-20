int test() {
    int k = 2;
    int x = 7;
    int y = 9;
    int z = 11;
    while (k < 0) {
        x = x + y;
        y = y + z;
    }
    if (k > 1) {
        z = z + x;
    } else {
        z = z - y;
    }
    return x + y + z + k;
}
