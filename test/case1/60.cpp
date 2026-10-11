int test() {
    int state = 0;
    int p1 = 5;
    int p2 = 10;
    int p3 = 15;
    int count = 30;

    while (count > 0) {
        if (state == 0) {
            p1 = p1 + p2;
            state = 1;
        } else {
            if (state == 1) {
                p2 = p2 + p3;
                state = 2;
            } else {
                p3 = p3 + p1;
                state = 0;
            }
        }
        count = count - 1;
    }

    return p1 + p2 + p3 + state + count;
}