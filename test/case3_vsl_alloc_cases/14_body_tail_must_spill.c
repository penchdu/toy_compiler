// Case B: the condition is deliberately simple and only touches i.
// The loop body dirties a, b and c, which remain live after the loop.
// The condition should not emit MC_ST for these VRs; the body tail must
// store any such dirty VR that remains resident after recovery.
int test()
{
    int a = 2;
    int b = 3;
    int c = 4;
    int i = 5;

    while ((i = i - 1) > 0)
    {
        i = i - 1;
        a = a + b;
        b = b + c;
        c = c + a;
    }

    return a + b + c + i;
}
