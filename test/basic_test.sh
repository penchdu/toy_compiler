#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

TEST_DIRS=()
TEST_DIRS+=(
	case_while
	#case_while_continue_break
	#case_if
	#case_while_if
	#case_while_continue_break_if
)

#if [ -d "$SCRIPT_DIR/../vsc_test/time_compare/case" ]; then
#	TEST_DIRS+=("../vsc_test/time_compare/case")
#fi

if [ -d "$SCRIPT_DIR/../vsc_test/time_compare/case2" ]; then
   TEST_DIRS+=("../vsc_test/time_compare/case2")
fi


VSC="$PROJECT_DIR/build/vsc"

[ -x "$VSC" ] || { echo "Error: compiler not found at $VSC"; exit 1; }

RUNNER_MAIN="$PROJECT_DIR/print_test.c"

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

    for SRC in $(ls -v "$TEST_DIR"/*.c 2>/dev/null); do
        [ -e "$SRC" ] || continue
        NAME="$(basename "$SRC")"
        BASENAME="${NAME%.c}"
        GCC_BIN="$TEST_DIR/.gcc_ref/$BASENAME"
        mkdir -p "$TEST_DIR/.gcc_ref"
        if [ ! -x "$GCC_BIN" ] || [ "$SRC" -nt "$GCC_BIN" ]; then
            echo "gcc compile: $SRC"
            if ! gcc "$RUNNER_MAIN" "$SRC" -o "$GCC_BIN" > /dev/null; then
                rm -f "${GCC_BIN}.broken"
                touch "${GCC_BIN}.broken"
            else
                rm -f "${GCC_BIN}.broken"
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

    for SRC in $(ls -v "$TEST_DIR"/*.c 2>/dev/null); do
        [ -e "$SRC" ] || continue
        let total=total+1

        NAME="$(basename "$SRC")"
        BASENAME="${NAME%.c}"
        CD="$(mktemp -d)"

        A0="compile fail"; A1="compile fail"

        if (cd "$CD" && "$VSC" "$SRC" > /dev/null); then
            if [ -f "$CD/a0.s" ]; then
                if (cd "$CD" && gcc "$RUNNER_MAIN" a0.s -o a0 > /dev/null); then
                    A0_TMP="$(extract_result 0 "$(cd "$CD" && timeout 3s ./a0 2>&1)")"
                    [ -n "$A0_TMP" ] && [ "$A0_TMP" != "TIMEOUT" ] && A0="$A0_TMP"
                fi
            fi
            if [ -f "$CD/a1.s" ]; then
                if (cd "$CD" && gcc "$RUNNER_MAIN" a1.s -o a1 > /dev/null); then
                    A1_TMP="$(extract_result 0 "$(cd "$CD" && timeout 3s ./a1 2>&1)")"
                    [ -n "$A1_TMP" ] && [ "$A1_TMP" != "TIMEOUT" ] && A1="$A1_TMP"
                fi
            fi
        fi

        GCC_OUT=""; GCC_ERR=0; GCC="compile fail"
        GCC_BIN="$TEST_DIR/.gcc_ref/$BASENAME"
        if [ -x "$GCC_BIN" ] && [ ! -f "${GCC_BIN}.broken" ]; then
            GCC_OUT="$(timeout 3s "$GCC_BIN" 2>&1)"
            GCC_TMP="$(extract_result 0 "$GCC_OUT")"
            if [ -n "$GCC_TMP" ] && [ "$GCC_TMP" != "TIMEOUT" ]; then
                GCC="$GCC_TMP"
            else
                echo "  gcc run fail: $GCC_OUT"
            fi
        fi

        rm -rf "$CD"

        if [ "$A0" = "$GCC" ] && [ "$A1" = "$GCC" ]; then
            echo "pass: $NAME"; let pass=pass+1
        else
            echo "fail: $NAME  a0:$A0  a1:$A1  gcc:$GCC"; let fail=fail+1
        fi
    done
done

END="$(date +%s.%N)"
ELAPSED="$(awk "BEGIN {printf \"%.3f\", $END - $START}")"
echo
echo "result: all=$total | pass=$pass | fail=$fail | time=${ELAPSED}s"