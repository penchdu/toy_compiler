int test(int dummy) {
    int n = 5;
    int x = 1;
    int y = 2;
    int z = 3;
    int carry = 4;
    while ((n = n - 1) > 0) {
        if (n > 2) {
            x = x + y;
            carry = carry + x;
        } else {
            y = y + z;
            carry = carry - y;
        }
        z = z + carry;
    }
    return n + x + y + z + carry;
}
