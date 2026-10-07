#!/bin/bash
# while 寄存器分配器毒辣测试套件
# 原理：main.cpp 固定读 ./test/test.cpp
#      x64_reg_alloc() 会生成 a0.s(O0基准) 和 a1.s(wave)
#      编译运行两者的 Result 应该一致

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

PASS=0
FAIL=0
TOTAL=0
SRC=./test/test.cpp

run_test() {
    local name="$1"
    local src="$2"

    TOTAL=$((TOTAL + 1))

    # 注入测试代码
    cat > "$SRC" << TESTEOF
#include <stdio.h>
$src
TESTEOF

    # 编译
    make -s clean && make -s 2>/dev/null
    if [ $? -ne 0 ]; then
        echo -e "${RED}[FAIL]${NC} #$TOTAL $name — 编译失败"
        FAIL=$((FAIL + 1))
        return
    fi

    # 生成 a0.s / a1.s 并运行 main
    ./build/a > /dev/null 2>&1

    # 编译并分别运行
    local r1 r2
    if [ -f a0.s ] && gcc a0.s -o /tmp/a0_test 2>/dev/null; then
        r1=$(/tmp/a0_test 2>/dev/null | grep -oP 'Result:\s*\K\d+' | head -1)
    fi
    if [ -f a1.s ] && gcc a1.s -o /tmp/a1_test 2>/dev/null; then
        r2=$(/tmp/a1_test 2>/dev/null | grep -oP 'Result:\s*\K\d+' | head -1)
    fi

    # 验证
    if [ "$r1" = "$r2" ] && [ -n "$r1" ]; then
        echo -e "${GREEN}[PASS]${NC} #$TOTAL $name => $r1"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}[FAIL]${NC} #$TOTAL $name => O0=$r1 Wave=$r2"
        FAIL=$((FAIL + 1))
    fi
}

# ===== Case 1: 最初那个 bug 触发条件 =====
run_test "BUG1_line108条件" '
int test()
{
    int a0 = 1; int a1 = 2; int a2 = 3; int a3 = 4;
    int b0 = 5; int b1 = 6; int b2 = 7; int b3 = 8;
    int c0 = 9; int c1 = 10; int c2 = 11; int c3 = 12;
    int d0 = 13; int d1 = 14; int d2 = 15; int d3 = 16;
    int i0 = 2;
    while (i0 > 0) {
        a0 = a0 + b0; a1 = a1 + c0; a2 = a2 + d0; a3 = a3 + b1;
        int i1 = 2;
        while (i1 > 0) {
            b0 = b0 + a1; b1 = b1 + c1; b2 = b2 + d1; b3 = b3 + a2;
            int i2 = 2;
            while (i2 > 0) {
                c0 = c0 + b2; c1 = c1 + d2; c2 = c2 + a3; c3 = c3 + b3;
                int i3 = 2;
                while (i3 > 0) {
                    d0 = d0 + c0; d1 = d1 + a0; d2 = d2 + b1; d3 = d3 + c2;
                    a0 = a0 + d3; b0 = b0 + a2; c0 = c0 + b2;
                    i3 = i3 - 1;
                }
                c1 = c1 + d0; d1 = d1 + a1; b1 = b1 + c2;
                i2 = i2 - 1;
            }
            b2 = b2 + c3; c2 = c2 + d2; d2 = d2 + a2;
            i1 = i1 - 1;
        }
        a1 = a1 + b3; b3 = b3 + c1; c3 = c3 + d1; d3 = d3 + a3;
        i0 = i0 - 1;
    }
    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3 + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3;
}'

# ===== Case 2: 你的原始 4 层嵌套大 case =====
run_test "原始4层嵌套" '
int test() {
    int a0 = 1, a1 = 2, a2 = 3, a3 = 4;
    int b0 = 5, b1 = 6, b2 = 7, b3 = 8;
    int c0 = 9, c1 = 10, c2 = 11, c3 = 12;
    int d0 = 13, d1 = 14, d2 = 15, d3 = 16;
    int i0 = 2;
    while (i0 > 0) {
        a0 = a0 + b0; a1 = a1 + c0; a2 = a2 + d0; a3 = a3 + b1;
        int i1 = 2;
        while (i1 > 0) {
            b0 = b0 + a1; b1 = b1 + c1; b2 = b2 + d1; b3 = b3 + a2;
            int i2 = 2;
            while (i2 > 0) {
                c0 = c0 + b2; c1 = c1 + d2; c2 = c2 + a3; c3 = c3 + b3;
                int i3 = 2;
                while (i3 > 0) {
                    d0 = d0 + c0; d1 = d1 + a0; d2 = d2 + b1; d3 = d3 + c2;
                    a0 = a0 + d3; b0 = b0 + a2; c0 = c0 + b2;
                    i3 = i3 - 1;
                }
                c1 = c1 + d0; d1 = d1 + a1; b1 = b1 + c2;
                i2 = i2 - 1;
            }
            b2 = b2 + c3; c2 = c2 + d2; d2 = d2 + a2;
            i1 = i1 - 1;
        }
        a1 = a1 + b3; b3 = b3 + c1; c3 = c3 + d1; d3 = d3 + a3;
        i0 = i0 - 1;
    }
    return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3 + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3;
}'

# ===== Case 3: swap_pr 正好是 before 要用的 =====
run_test "swap_pr==A 攻击" '
int test() {
    int a0 = 1; int a1 = 2; int a2 = 3;
    int a3 = 4; int a4 = 5; int a5 = 6;
    int i = 3;
    while (i > 0) {
        a0 = a0 + a1;
        a1 = a1 + a2;
        a2 = a2 + a3;
        a3 = a3 + a4;
        a4 = a4 + a5;
        a5 = a5 + a0;
        i = i - 1;
    }
    return a0 + a1 + a2 + a3 + a4 + a5;
}'

# ===== Case 4: 连环 3 寄存器交换 =====
run_test "连环3寄存器交换" '
int test() {
    int a0 = 1; int a1 = 2; int a2 = 3;
    int a3 = 4; int a4 = 5; int a5 = 6;
    int i = 2;
    while (i > 0) {
        int t0 = a0 + a1;
        int t1 = a2 + a3;
        int t2 = a4 + a5;
        a0 = a2 + t0;
        a2 = a4 + t1;
        a4 = a0 + t2;
        a1 = a1 + a3;
        a3 = a3 + a5;
        a5 = a5 + a1;
        i = i - 1;
    }
    return a0 + a1 + a2 + a3 + a4 + a5;
}'

# ===== Case 5: 简单 while =====
run_test "简单while" '
int test() {
    int a = 0;
    while (a < 10) {
        a = a + 1;
    }
    return a;
}'

# ===== Case 6: 双层 while 嵌套 =====
run_test "双层嵌套" '
int test() {
    int a = 0; int b = 0;
    while (a < 5) {
        a = a + 1;
        while (b < 10) {
            b = b + 1;
            if (b == 3) break;
        }
        b = b + 10;
    }
    return b;
}'

# ===== Case 7: cond 里密集写变量 =====
run_test "cond密集写" '
int test() {
    int a = 1; int b = 2; int c = 3; int d = 4;
    int i = 3;
    while ((i = i - 1 + (a = a + 1) - (b = b - 1)) > 0) {
        c = c + d;
        d = d + a;
        b = b + c;
    }
    return a + b + c + d;
}'

# ===== Case 8: continue + break =====
run_test "continue+break" '
int test() {
    int a = 0; int b = 0;
    while (a < 20) {
        a = a + 1;
        if (a < 5) continue;
        if (a == 11) break;
        b = b + a;
    }
    return a + b;
}'

# ===== Case 9: 7变量（触发 eviction）=====
run_test "7变量+while" '
int test() {
    int a = 1; int b = 2; int c = 3; int d = 4;
    int e = 5; int f = 6; int g = 7;
    int i = 3;
    while (i > 0) {
        a = a + b; b = b + c; c = c + d;
        d = d + e; e = e + f; f = f + g;
        g = g + a;
        i = i - 1;
    }
    return a + b + c + d + e + f + g;
}'

# ===== Case 10: while 里大量临时变量 =====
run_test "大量临时变量" '
int test() {
    int a0 = 1; int a1 = 2; int a2 = 3;
    int a3 = 4; int a4 = 5; int a5 = 6;
    int i = 2;
    while (i > 0) {
        int t0 = a0 + a1;
        int t1 = a2 + a3;
        int t2 = a4 + a5;
        int t3 = t0 + t1;
        int t4 = t1 + t2;
        int t5 = t3 + t4;
        a0 = t3; a2 = t4; a4 = t5;
        a1 = a0 + t1; a3 = a2 + t2; a5 = a4 + t0;
        i = i - 1;
    }
    return a0 + a1 + a2 + a3 + a4 + a5;
}'

# ===== Case 11: 3 层 while + cond 里写 =====
run_test "3层while_cond写" '
int test() {
    int a = 100; int b = 200; int c = 300; int d = 400; int e = 500; int f = 600;
    int i0 = 2; int result = 0;
    while (i0 > 0) {
        a = a + b;
        c = c + d;
        int i1 = 2;
        while (i1 > 0) {
            b = b + c;
            e = e + f;
            int i2 = 2;
            while (i2 > 0) {
                int t0 = a + b; int t1 = c + d; int t2 = e + f;
                a = t0 + c; c = t1 + e; e = t2 + a;
                i2 = i2 - 1;
            }
            b = b + e;
            f = f + a;
            i1 = i1 - 1;
        }
        result = result + a + b + c + d + e + f;
        i0 = i0 - 1;
    }
    return result;
}'

echo ""
echo "=========================================="
echo -e "  ${GREEN}Pass${NC}: $PASS / $TOTAL    ${RED}Fail${NC}: $FAIL"
echo "=========================================="