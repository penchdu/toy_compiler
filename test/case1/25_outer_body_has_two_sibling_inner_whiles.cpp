// No if/else. The outer body contains two sibling child loops, each with its
// own BasicBlocks. x is kept clean while both child loops are allocated; it is
// dirtied only afterward. The outer repair must find all relevant victims in
// the descendants, not only scan the outer body's own BasicBlock.
int test()
{
    int x = 6;
    int p = 10;
    int a = 4;
    int b = 8;
    int q = 0;

    while ((p = p - 1) + x > 0)
    {
        q = 4;
        while ((q = q - 1) > 0)
        {
            a = (a + b) / 2 + (a + 1) / 2;
            b = (b + a) / 2 + (b + 1) / 2;
        }

        q = 5;
        while ((q = q - 1) > 0)
        {
            b = (b + a) / 2 + (b + 2) / 2;
            a = (a + b) / 2 + (a + 2) / 2;
        }

        // Both sibling loops were allocated before x becomes dirty.
        x = x - 2;
        p = p - 1;
    }

    return p * 100 + a + b;
}
