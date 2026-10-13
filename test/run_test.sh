#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

TEST_DIRS=(
	case4
	case2
	case3_vsl_alloc_cases
    case1
)

VSC="$PROJECT_DIR/build/vsc"

[ -x "$VSC" ] || { echo "Error: compiler not found at $VSC"; exit 1; }

total=0; pass=0; fail=0
START="$(date +%s.%N)"

extract_result() {
    local exit_code=$1
    local output=$2
    [ $exit_code -eq 124 ] && { echo TIMEOUT; return; }
    printf '%s\n' "$output" | sed -n 's/.*Result:[[:space:]]*\([-0-9][0-9]*\).*/\1/p' | tail -n 1
}

# ===== 阶段一：gcc 预编译全部 runner =====
for DIR_NAME in "${TEST_DIRS[@]}"; do
    TEST_DIR=""
    for base in "$SCRIPT_DIR" "$PROJECT_DIR"; do
        [ -d "$base/$DIR_NAME" ] && { TEST_DIR="$base/$DIR_NAME"; break; }
    done
    [ -z "$TEST_DIR" ] && continue

    for CPP in "$TEST_DIR"/*.cpp; do
        [ -e "$CPP" ] || continue
        NAME="$(basename "$CPP")"
        BASENAME="${NAME%.cpp}"
        GCC_BIN="$TEST_DIR/.gcc_ref/$BASENAME"
        mkdir -p "$TEST_DIR/.gcc_ref"
        if [ ! -x "$GCC_BIN" ] || [ "$CPP" -nt "$GCC_BIN" ]; then
            echo "gcc: $CPP"
            RUNNER_TMP="$(mktemp --suffix=.cpp)"
            cat > "$RUNNER_TMP" <<EOF
#include <stdio.h>
#include "$CPP"
int main() { printf("Result: %d\n", test()); return 0; }
EOF
            if ! gcc "$RUNNER_TMP" -o "$GCC_BIN" > /dev/null 2>&1; then
                rm -f "$RUNNER_TMP" "${GCC_BIN}.broken"
                touch "${GCC_BIN}.broken"
            else
                rm -f "$RUNNER_TMP" "${GCC_BIN}.broken"
            fi
        fi
    done
done

# ===== 阶段二：跑测试 =====
for DIR_NAME in "${TEST_DIRS[@]}"; do
    TEST_DIR=""
    for base in "$SCRIPT_DIR" "$PROJECT_DIR"; do
        [ -d "$base/$DIR_NAME" ] && { TEST_DIR="$base/$DIR_NAME"; break; }
    done
    [ -z "$TEST_DIR" ] && { echo "skip: $DIR_NAME (not found)"; continue; }
    echo "===== $DIR_NAME ====="

    for CPP in "$TEST_DIR"/*.cpp; do
        [ -e "$CPP" ] || continue
        let total=total+1

        NAME="$(basename "$CPP")"
        BASENAME="${NAME%.cpp}"
        CD="$(mktemp -d)"

        (cd "$CD" && "$VSC" "$CPP") > /dev/null 2>&1
        if [ ! -f "$CD/a0.s" ] || [ ! -f "$CD/a1.s" ]; then
            echo "fail: $NAME (vsc compile)"; let fail=fail+1; rm -rf "$CD"; continue
        fi

        if ! (cd "$CD" && gcc a0.s -o a0 && gcc a1.s -o a1) > /dev/null 2>&1; then
            echo "fail: $NAME (gcc assemble)"; let fail=fail+1; rm -rf "$CD"; continue
        fi

        A0_OUT="$(cd "$CD" && timeout 3s ./a0 2>&1)"
        A0_ERR=$?
        A1_OUT="$(cd "$CD" && timeout 3s ./a1 2>&1)"
        A1_ERR=$?
        A0="$(extract_result $A0_ERR "$A0_OUT")"
        A1="$(extract_result $A1_ERR "$A1_OUT")"

        GCC_OUT=""; GCC_ERR=0
        GCC_BIN="$TEST_DIR/.gcc_ref/$BASENAME"
        if [ -x "$GCC_BIN" ] && [ ! -f "${GCC_BIN}.broken" ]; then
            GCC_OUT="$(timeout 3s "$GCC_BIN" 2>&1)"
            GCC_ERR=$?
        fi
        GCC="$(extract_result $GCC_ERR "$GCC_OUT")"

        rm -rf "$CD"

        if [ -n "$A0" ] && [ "$A0" != "TIMEOUT" ] && [ "$A0" = "$A1" ] && [ "$A1" = "$GCC" ]; then
            echo "pass: $NAME"; let pass=pass+1
        else
            echo "fail: $NAME (a0:$A0 | a1:$A1 | gcc:$GCC)"; let fail=fail+1
        fi
    done
done

END="$(date +%s.%N)"
ELAPSED="$(awk "BEGIN {printf \"%.3f\", $END - $START}")"
echo "result: all=$total | pass=$pass | fail=$fail | time=${ELAPSED}s"