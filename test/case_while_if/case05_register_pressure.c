int test(int dummy) {
    int i = 0;
    int j = 0;
    int a0 = 1;
    int a1 = 2;
    int a2 = 3;
    int a3 = 4;
    int b0 = 5;
    int b1 = 6;
    int b2 = 7;
    int b3 = 8;
    int carry = 9;
    int total = 10;
    while (i < 3) {
        j = 0;
        while (j < 4) {
            if (i + j > 2) {
                a0 = a0 + b0;
                a2 = a2 + b2;
                total = total + a0;
            } else {
                a1 = a1 + b1;
                a3 = a3 + b3;
                total = total + a1;
            }
            carry = carry + i + j;
            j = j + 1;
        }
        i = i + 1;
    }
    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3 + carry + total + i + j;
}
