int test(int dummy) {
    int outer = 5;
    int accumulator = 0;
    int base = 10;

    while (outer > 0) {
        int inner = 4;
        while (inner > 0) {
            if (inner > 2) {
                accumulator = accumulator + base + inner;
            } else {
                accumulator = accumulator - base + inner;
            }
            inner = inner - 1;
        }
        base = base + 5;
        outer = outer - 1;
    }

    return accumulator + outer + base;
}