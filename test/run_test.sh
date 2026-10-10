#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# ===== test case dirs (relative to PROJECT_DIR) =====
TEST_DIRS=(
    case2
    case1
)

VSC="$PROJECT_DIR/build/vsc"
OUT="$SCRIPT_DIR/test_result"

[ -x "$VSC" ] || { echo "Error: compiler not found at $VSC"; exit 1; }

rm -rf "$OUT" && mkdir -p "$OUT"

total=0; pass=0; fail=0

for DIR_NAME in "${TEST_DIRS[@]}"; do
    # search order: SCRIPT_DIR (test/), then PROJECT_DIR (vsc/)
    TEST_DIR=""
    for base in "$SCRIPT_DIR" "$PROJECT_DIR"; do
        [ -d "$base/$DIR_NAME" ] && { TEST_DIR="$base/$DIR_NAME"; break; }
    done
    [ -z "$TEST_DIR" ] && { echo "skip: $DIR_NAME (not found)"; continue; }
    echo "===== $DIR_NAME ====="

    for CPP in "$TEST_DIR"/*.cpp; do
        [ -e "$CPP" ] || continue
        total=$((total + 1))

        NAME="$(basename "$CPP")"
        CD="$OUT/$DIR_NAME/${NAME%.cpp}"
        mkdir -p "$CD"

        # VSC compile
        (cd "$CD" && "$VSC" "$CPP") > /dev/null 2>&1
        if [ ! -f "$CD/a0.s" ] || [ ! -f "$CD/a1.s" ]; then
            echo "fail: $NAME (vsc compile)"; fail=$((fail + 1)); continue
        fi

        # GCC assemble
        (cd "$CD" && gcc a0.s -o a0 && gcc a1.s -o a1) > /dev/null 2>&1
        if [ $? -ne 0 ]; then
            echo "fail: $NAME (gcc assemble)"; fail=$((fail + 1)); continue
        fi

        # Run a0 / a1 (separate cwd via subshell)
        (cd "$CD" && timeout 3s ./a0) > "$CD/a0.out" 2>&1
        A0_ERR=$?
        (cd "$CD" && timeout 3s ./a1) > "$CD/a1.out" 2>&1
        A1_ERR=$?

        [ $A0_ERR -eq 124 ] && A0=TIMEOUT || A0=$(sed -n 's/.*Result:[[:space:]]*\([-0-9][0-9]*\).*/\1/p' "$CD/a0.out" | tail -n 1)
        [ $A1_ERR -eq 124 ] && A1=TIMEOUT || A1=$(sed -n 's/.*Result:[[:space:]]*\([-0-9][0-9]*\).*/\1/p' "$CD/a1.out" | tail -n 1)

        # GCC reference
        echo '#include <stdio.h>
#include "'"$CPP"'"
int main() { printf("Result: %d\n", test()); return 0; }' > "$CD/runner.cpp"
        (cd "$CD" && gcc runner.cpp -o gcc_ref && timeout 3s ./gcc_ref) > "$CD/gcc.out" 2>&1
        GCC_ERR=$?
        [ $GCC_ERR -eq 124 ] && GCC=TIMEOUT || GCC=$(sed -n 's/.*Result:[[:space:]]*\([-0-9][0-9]*\).*/\1/p' "$CD/gcc.out" | tail -n 1)

        # Compare
        if [ -n "$A0" ] && [ "$A0" != "TIMEOUT" ] && [ "$A0" = "$A1" ] && [ "$A1" = "$GCC" ]; then
            echo "pass: $NAME"; pass=$((pass + 1))
        else
            echo "fail: $NAME (a0:$A0 | a1:$A1 | gcc:$GCC)"; fail=$((fail + 1))
        fi
    done
done

#echo "----------------------------------------"
echo "result: all=$total | pass=$pass | fail=$fail"

