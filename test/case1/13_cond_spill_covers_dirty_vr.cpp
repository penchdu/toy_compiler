// Case A: the condition is intentionally register-pressure heavy.
// The assigned value of i must survive while the right-hand expression is
// evaluated. The allocator should have an opportunity to emit MC_ST for i
// inside the condition block. That condition-side store should make a second
// body-tail store for the same VR unnecessary.
int test()
{
    int x0 = 0; int x1 = 0; int x2 = 0; int x3 = 0;
    int x4 = 0; int x5 = 0; int x6 = 0; int x7 = 0;
    int x8 = 0; int x9 = 0; int x10 = 0; int x11 = 0;
    int guard = 73;
    int i = 5;

    while (((i = i - 1) +
            (x0 + (x1 + (x2 + (x3 + (x4 + (x5 + (x6 + (x7 + (x8 + (x9 + (x10 + x11)))))))))))) > 0)
    {
        // Make the condition operands dirty in the body without changing the
        // zero-valued sum. They remain live because the condition and return
        // both read them; pressure in cond can then spill a body-dirty VR.
        i = i - 1;
        x0 = x0 + 0; x1 = x1 + 0; x2 = x2 + 0; x3 = x3 + 0;
        x4 = x4 + 0; x5 = x5 + 0; x6 = x6 + 0; x7 = x7 + 0;
        x8 = x8 + 0; x9 = x9 + 0; x10 = x10 + 0; x11 = x11 + 0;
    }

    return guard + i + x0 + x1 + x2 + x3 + x4 + x5
                 + x6 + x7 + x8 + x9 + x10 + x11;
}
