int test()
{
    int c0 = 0;
    int c1 = 0;
    int c2 = 0;
    int c3 = 0;
    int c4 = 0;
    int c5 = 0;

    int v0 = 6;
    int v1 = 9;
    int v2 = 8;
    int v3 = 11;
    int v4 = 8;
    int v5 = 6;
    int v6 = 4;
    int v7 = 3;
    int v8 = 5;
    int v9 = 5;

    c0 = 5;
    while ((c0 = c0 - 1) > 0)
    {
        c1 = 3;
        while ((c1 = c1 - 1) > 0)
        {
            v2 = (v1 + v8) / 3;
            v1 = (v3 - v2) / 4;
            v2 = (v9 - v5) / 3;
        }
        break;
        v0 = v4 + 2;
        v7 = v3 + 1;
    }

    v2 = (v4 + v5) / 2 - 2;
    c2 = 4;
    while ((c2 = c2 - 1) > 0)
    {
        c3 = 4;
        while ((c3 = c3 - 1) > 0)
        {
            v0 = (v3 - v3) / 2;
            v4 = v0 - 0;
        }
        continue;
        c4 = 5;
        while ((c4 = c4 - 1) > 0)
        {
            v4 = v4 + 0;
            continue;
            v5 = v7 + 2;
            c5 = 5;
            while ((c5 = c5 - 1) > 0)
            {
                v9 = (v4 - v7) / 4;
                v4 = (v5 - v4) / 4;
            }
            continue;
            v3 = v2 + 3;
            v6 = (v4 + v0) / 4;
            v2 = v0 - 2;
        }
        continue;
        v7 = (v0 + v0) / 2 - 1;
        v3 = (v9 + v1) / 2 + 1;
        break;
        v2 = v2 + 1;
    }
    v2 = (v9 + v6) / 2 - 1;

    return v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + c0 + c1 + c2 + c3 + c4 + c5;
}
