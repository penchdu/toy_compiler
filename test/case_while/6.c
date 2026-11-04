int test(int dummy)
{
    int a0 = 1;   // å¡«æ»¡ R10D
    int a1 = 2;   // R11D  â†?get_swap_pr ä¼?spill è¿™ä¸ªï¼ˆVR_USAGE_READï¼?
    int a2 = 3;   // R12D
    int a3 = 4;   // R13D
    int a4 = 5;   // R14D
    int a5 = 6;   // R15D

    // è¿™é‡ŒæŠ?a1 æ ‡è®°ä¸ºåªè¯»åï¼ˆget_swap_pr è¦?spill å®ƒè…¾å‡?prï¼?
    // ä½?spill_vr ä¸æ›´æ–?pr2vr â†?pre_cond é‡?a1.pr == swap_prï¼ï¼

    int i = 3;
    while (i > 0)
    {
        // cond: è¯?a1ï¼ˆè§¦å?reloadï¼Œè®© wave allocator æŠ?a1 æŒªèµ°ï¼?
        // body: æŠ?a1 ä»?swap_pr æŒªåˆ°åˆ«çš„ prï¼ˆæ¯”å¦?R12Dï¼?
        // recover æ—¶ï¼š
        //   A = before_vr2pr[a1].pr = swap_pr  â†?ğŸ’€ ç›®æ ‡å°±æ˜¯ swap_pr
        //   B = now_vr2pr[a1].pr = R12D       â†?a1 åœ¨è¿™
        //   curr_vr_have_A = now_pr2vr[swap_pr]  â†?swap_pr è¢?body é‡Œä¸´æ—¶å˜é‡å äº?
        //   â†?è¿›å…¥æƒ…å†µ 2a â†?assert(now_pr2vr[swap_pr].vr==INVALID) å´©ï¼ï¼?
        a0 = a0 + a1;
        a1 = a1 + a2;    // è®?a1 è¢«å†™
        a2 = a2 + a3;
        a3 = a3 + a4;
        a4 = a4 + a5;
        a5 = a5 + a0;

        i = i - 1;
    }

    return a0 + a1 + a2 + a3 + a4 + a5;
}
