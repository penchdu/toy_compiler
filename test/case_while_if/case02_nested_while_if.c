int test(int dummy) {
    int i = 0;
    int j = 0;
    int x = 1;
    int y = 2;
    int z = 3;
    int carry = 4;
    while (i < 4) {
        j = 0;
        while (j < 3) {
            if (i + j < 3) {
                x = x + y;
                carry = carry + 1;
            } else {
                z = z + x;
                y = y + 1;
            }
            j = j + 1;
        }
        if (carry > 5) {
            y = y + carry;
        } else {
            x = x + z;
        }
        i = i + 1;
    }
    return x + y + z + carry + i + j;
}
