int test(int dummy) {
    int i = 0;
    int a = 3; int b = 4; int c = 8; int d = 2; int post = 5;
    while ((i = i + 1) <= 5) {
        if (i < 3) {
            a = a + b;
            c = c + i;
        } else {
            b = b + c;
            d = d + a;
        }
        if (i < 5) {
            post = post + b;
        } else {
            post = post + d;
        }
    }
    if (i > 5) {
        a = a + post;
        c = c + a;
    } else {
        b = b + post;
        d = d + b;
    }
    return i + a + b + c + d + post;
}
