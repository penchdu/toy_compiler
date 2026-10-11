int test() {
    int i = 0;
    int x = 1;
    int y = 2;
    int z = 3;
    int w = 4;
    int carry = 5;
    while (i < 4) {
        x = x + y;
        if (i > 1) {
            z = z + x;
        } else {
            w = w + y;
        }
        carry = carry + x - z;
        i = i + 1;
    }
    i = 0;
    while (i < 3) {
        if (carry > 0) {
            y = y + carry;
            z = z + 1;
        } else {
            w = w - carry;
            x = x + 1;
        }
        carry = carry + w - x;
        i = i + 1;
    }
    return x + y + z + w + carry + i;
}
