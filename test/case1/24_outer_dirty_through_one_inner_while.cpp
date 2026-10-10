// No if/else. The inner loop is allocated while x is still clean: x is
// modified only AFTER the child loop. At runtime, later outer iterations enter
// that already-allocated child loop with the new x value. If x had a victim in
// a descendant BB with no MC_ST, outer new_dirty repair must discover that
// child-loop victim; scanning only the outer body's own BB cannot do so.
int test()
{
    int x = 5;
    int p = 9;
    int a = 3;
    int b = 5;
    int q = 0;

    while ((p = p - 1) + x > 0)
    {
        q = 4;
        while ((q = q - 1) > 0)
        {
            // Two live subexpressions create temporary register pressure.
            a = (a + b) / 2 + (a + 1) / 2;
            b = (b + a) / 2 + (b + 2) / 2;
        }

        // Critical placement: dirty x only after the descendant loop.
        x = x - 2;
        p = p - 1;
    }

    // x influences the outer-loop trip count but is not read after the loop.
    return p * 100 + a + b;
}
