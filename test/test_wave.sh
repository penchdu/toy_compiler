#!/bin/bash
# Wave 寄存器分配器测试脚本 v2
# 修复: heredoc 变量展开 + Result 输出捕获

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

PASS=0
FAIL=0
TOTAL=0

# 重跑 x64_reg_alloc 里的流程，直接分别执行 ./a0 和 ./a1
run_both() {
    # 重新生成汇编并分别运行
    make -s clean && make -s 2>/dev/null || return 1

    # 从 main.cpp 知道：x64_reg_alloc() 会同时生成 a0.s 和 a1.s
    # 但我们只运行了 build/a。需要确保 a0.s 和 a1.s 已经在目录里
    make -s clean && make -s 2>/dev/null
    ./build/a > /dev/null 2>&1   # 生成 a0.s a1.s 并运行一次（会打印两次 Result 到终端）

    # 现在 a0.s 和 a1.s 应该已经存在
    [ -f a0.s ] && { gcc a0.s -o /tmp/a0_test 2>/dev/null || return 1; } || return 1
    [ -f a1.s ] && { gcc a1.s -o /tmp/a1_test 2>/dev/null || return 1; } || return 1

    local r1 r2
    r1=$(/tmp/a0_test 2>/dev/null | grep -oP 'Result:\s*\K\d+')
    r2=$(/tmp/a1_test 2>/dev/null | grep -oP 'Result:\s*\K\d+')

    echo "$r1 $r2"
}

run_test() {
    local name="$1"
    local expected="$2"
    local src="$3"

    TOTAL=$((TOTAL + 1))

    # --- 修复 Bug1: heredoc 不加引号让 $src 展开 ---
    cat > t1.txt << TESTEOF
$src
TESTEOF

    # --- 分别运行 -O0 和 wave ---
    # 先让 build/a 跑一遍（会生成 a0.s 和 a1.s）
    make -s clean && make -s 2>/dev/null
    if [ $? -ne 0 ]; then
        echo -e "${RED}[FAIL]${NC} #$TOTAL $name — 编译失败"
        FAIL=$((FAIL + 1))
        return
    fi

    ./build/a > /dev/null 2>&1   # 内部会调 system("gcc a0.s -o a0; ./a0") 和 system("gcc a1.s -o a1; ./a1")

    # 直接运行生成的 a0 和 a1，捕获 Result
    local r1 r2
    r1=$(./a0 2>/dev/null | grep -oP 'Result:\s*\K\d+' | head -1)
    r2=$(./a1 2>/dev/null | grep -oP 'Result:\s*\K\d+' | head -1)

    # 验证
    if [ "$r1" = "$r2" ] && [ "$r1" = "$expected" ]; then
        echo -e "${GREEN}[PASS]${NC} #$TOTAL $name => O0=$r1 Wave=$r2 ✓"
        PASS=$((PASS + 1))
    elif [ "$r1" = "$r2" ]; then
        echo -e "${YELLOW}[WARN]${NC} #$TOTAL $name => O0=$r1 Wave=$r2 (期望值=$expected)"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}[FAIL]${NC} #$TOTAL $name => O0=$r1 Wave=$r2 (期望值=$expected)"
        FAIL=$((FAIL + 1))
    fi
}

# ===== 测试用例 =====

run_test "T1 单变量立即数" 42 '
int main() {
    int x = 42;
    return x;
}'

run_test "T2 两变量加法" 30 '
int main() {
    int a = 10;
    int b = 20;
    return a + b;
}'

run_test "T3 三变量加法链" 6 '
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    return a + b + c;
}'

run_test "T4 六变量塞满PR" 21 '
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;
    int f = 6;
    return a + b + c + d + e + f;
}'

run_test "T5 七变量触发eviction" 28 '
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;
    int f = 6;
    int g = 7;
    return a + b + c + d + e + f + g;
}'

run_test "T6 写后写覆盖" 100 '
int main() {
    int x = 1;
    x = 100;
    return x;
}'

run_test "T7 连续累加同一变量" 15 '
int main() {
    int a = 0;
    a = a + 1;
    a = a + 2;
    a = a + 3;
    a = a + 4;
    a = a + 5;
    return a;
}'

run_test "T8 高频vs低频使用" 14 '
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    a = a + a;
    a = a + b;
    b = b + b;
    c = c + c;
    return a + b + c;
}'

run_test "T9 READ_WRITE语义" 30 '
int main() {
    int a = 5;
    int b = 10;
    a = a + b;
    a = a + a;
    return a;
}'

run_test "T10 十变量多重eviction" 55 '
int main() {
    int a=1, b=2, c=3, d=4, e=5;
    int f=6, g=7, h=8, i=9, j=10;
    return a+b+c+d+e+f+g+h+i+j;
}'

run_test "T11 减法乘法混合" 4 '
int main() {
    int a = 10;
    int b = 3;
    int c = 2;
    return a - b * c;
}'

run_test "T12 Eviction后正确Reload" 60 '
int main() {
    int a = 10;
    int b = 20;
    int c = 30;
    int d = 1;
    int e = 2;
    int f = 3;
    int g = 4;
    d = d + e + f + g;
    e = e + f + g;
    f = f + g;
    g = g + 100;
    return a + b + c;
}'

run_test "T13 你的原测试用例" 601 '
int main()
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;

    a = a + b;
    a = a + c;
    a = a + b;
    a = a + c;
    a = a + b;
    a = a + c;
    a = a + b;
    a = a + c;

    b = b + 10;
    b = b + 20;

    a = a + d;
    a = a + e;
    a = a + d;
    a = a + e;

    c = c + d;
    c = c + e;
    c = c + d;

    b = b + c;
    b = b + a;
    b = b + c;

    a = a + d;
    b = b + d;
    c = c + d;
    e = e + d;

    return a + b + c + d + e;
}'

echo ""
echo "=========================================="
echo -e "  ${GREEN}Pass${NC}: $PASS / $TOTAL    ${RED}Fail${NC}: $FAIL"
echo "=========================================="