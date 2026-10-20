// No if/else. Three nested while levels are allocated while x is still clean.
// The outer body dirties x only after all descendant loops have been allocated.
// That makes this a direct test of whether outer st_new_dirty can locate victim
// sites in deep descendant BasicBlocks. The deepest expressions add temporary
// pressure while the live set of source variables stays relatively small.
int test()
{
    int x = 5;
    int p = 10;
    int q = 0;
    int r = 0;
    int a = 3;
    int b = 6;

    while ((p = p - 1) + x > 0)
    {
        q = 3;
        while ((q = q - 1) > 0)
        {
            r = 3;
            while ((r = r - 1) > 0)
            {
                a = (a + b + a + b + a) / 5 + 1;
                b = (b + a + b + a + b) / 5 + 2;
            }
        }

        // Critical placement: dirty x after the whole nested-loop subtree.
        x = x - 1;
        p = p - 1;
    }

    return p * 100 + a + b;
}
