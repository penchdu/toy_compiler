#!/bin/bash


SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

TEST_DIR="$PROJECT_DIR/test_case2"
VSC="$PROJECT_DIR/build/vsc"
RESULT_DIR="$SCRIPT_DIR/test_result" 


if [ -z "$TEST_DIR" ] || [ ! -d "$TEST_DIR" ]; then
   echo "Error: Test directory does not exist!"
    exit 1
fi
if [ ! -x "$VSC" ]; then
    echo "Error: compiler does not exist!"
    exit 1
fi

rm -rf "$RESULT_DIR"
mkdir -p "$RESULT_DIR"

total=0
pass=0
fail=0

for CPP in "$TEST_DIR"/*.cpp; do
    [ -e "$CPP" ] || continue
    total=$((total + 1))
    
    NAME="$(basename "$CPP")"
    BASE="${NAME%.cpp}"
    CASE_DIR="$RESULT_DIR/$BASE"
    mkdir -p "$CASE_DIR"

    # VSC compile
    (cd "$CASE_DIR" && "$VSC" "$CPP") > /dev/null 2>&1
    if [ ! -f "$CASE_DIR/a0.s" ] || [ ! -f "$CASE_DIR/a1.s" ]; then
        echo "fail: $NAME (vsc compile fail)"
        fail=$((fail + 1))
        continue
    fi

    # GCC assembly
    (cd "$CASE_DIR" && gcc a0.s -o a0 && gcc a1.s -o a1) > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "fail: $NAME (gcc assembly failed)"
        fail=$((fail + 1))
        continue
    fi

    # a0 (limit 3s)
    (cd "$CASE_DIR" && timeout 3s ./a0) > "$CASE_DIR/a0.out" 2>&1
    A0_ERR=$?
    if [ $A0_ERR -eq 124 ]; then
        A0_RES="TIMEOUT"
    else
        A0_RES=$(sed -n 's/.*Result:[[:space:]]*\([-0-9][0-9]*\).*/\1/p' "$CASE_DIR/a0.out" | tail -n 1)
    fi

    # a1 (limit 3s)
    (cd "$CASE_DIR" && timeout 3s ./a1) > "$CASE_DIR/a1.out" 2>&1
    A1_ERR=$?
    if [ $A1_ERR -eq 124 ]; then
        A1_RES="TIMEOUT"
    else
        A1_RES=$(sed -n 's/.*Result:[[:space:]]*\([-0-9][0-9]*\).*/\1/p' "$CASE_DIR/a1.out" | tail -n 1)
    fi

    # GCC output
    cat > "$CASE_DIR/runner.cpp" <<EOF
#include <stdio.h>
#include "$CPP"
int main() { printf("Result: %d\n", test()); return 0; }
EOF
    (cd "$CASE_DIR" && gcc runner.cpp -o gcc_ref && timeout 3s ./gcc_ref) > "$CASE_DIR/gcc.out" 2>&1
    GCC_ERR=$?
    if [ $GCC_ERR -eq 124 ]; then
        GCC_RES="TIMEOUT"
    else
        GCC_RES=$(sed -n 's/.*Result:[[:space:]]*\([-0-9][0-9]*\).*/\1/p' "$CASE_DIR/gcc.out" | tail -n 1)
    fi

    # compare
    if [ -n "$A0_RES" ] && [ "$A0_RES" != "TIMEOUT" ] && [ "$A0_RES" = "$A1_RES" ] && [ "$A1_RES" = "$GCC_RES" ]; then
        pass=$((pass + 1))
        echo "pass: $NAME"
    else
        fail=$((fail + 1))
        echo "fail: $NAME (a0:${A0_RES:-no result} | a1:${A1_RES:-no result} | gcc:${GCC_RES:-no result})"
    fi
done

echo "----------------------------------------"
echo "tests finished: all=$total | pass=$pass | fail=$fail"