int test() {
    int w1 = 1;
    int w2 = 2;
    int w3 = 3;
    int w4 = 4;
    int cnt = 10;

    while (cnt > 0) {
        int temp1 = w1 + w2;
        int temp2 = w2 + w3;
        int temp3 = w3 + w4;
        
        w1 = temp2;
        w2 = temp3;
        w3 = temp1 + cnt;
        w4 = temp1 - temp2;
        
        cnt = cnt - 1;
    }

    return w1 + w2 + w3 + w4 + cnt;
}