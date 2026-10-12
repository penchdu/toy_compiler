int test()
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;

    // a：高密度连续使用
    a = a + b;
    a = a + c;
    a = a + b;
    a = a + c;
    a = a + b;
    a = a + c;
    a = a + b;
    a = a + c;

    // b：暂时不怎么用，后面再连续使用
    b = b + 10;
    b = b + 20;

    // c：这里暂时不使用

    // a 继续使用
    a = a + d;
    a = a + e;
    a = a + d;
    a = a + e;

    // c：形成一个新的使用段
    c = c + d;
    c = c + e;
    c = c + d;

    // b：第二个使用段
    b = b + c;
    b = b + a;
    b = b + c;

    // d：相对分散
    a = a + d;
    b = b + d;
    c = c + d;
    e = e + d;

    return a + b + c + d + e;
}