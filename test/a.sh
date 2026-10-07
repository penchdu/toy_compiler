#!/bin/bash

set -e

if [ "$#" -ne 2 ]; then
    echo "usage: $0 <input_file> <output_dir>"
    exit 1
fi

INPUT="$1"
OUTDIR="$2"

if [ ! -f "$INPUT" ]; then
    echo "error: input file not found: $INPUT"
    exit 1
fi

mkdir -p "$OUTDIR"

max=0

shopt -s nullglob

for file in "$OUTDIR"/*.cpp; do
    name=$(basename "$file")

    if [[ "$name" =~ ^([0-9]+)\.cpp$ ]]; then
        num="${BASH_REMATCH[1]}"

        if (( num > max )); then
            max=$num
        fi
    fi
done

file_num=$((max + 1))

echo "input : $INPUT"
echo "output: $OUTDIR"
echo "start : $file_num"

awk -v outdir="$OUTDIR" -v file_num="$file_num" '
function count_open(s, t) {
    t = s
    return gsub(/\{/, "", t)
}

function count_close(s, t) {
    t = s
    return gsub(/\}/, "", t)
}

function write_case(    filename) {

    filename = outdir "/" file_num ".cpp"

    # 绝不覆盖已有文件
    while (system("test -e \"" filename "\"") == 0) {
        file_num++
        filename = outdir "/" file_num ".cpp"
    }

    printf "%s", buf > filename
    close(filename)

    print "created: " filename

    file_num++
}

BEGIN {
    in_test = 0
    seen_open = 0
    depth = 0
    buf = ""
}

{
    line = $0

    # 尚未进入 test()
    if (!in_test) {

        if (line ~ /^[[:space:]]*int[[:space:]]+test[[:space:]]*\(/) {

            in_test = 1
            seen_open = 0
            depth = 0
            buf = line "\n"

            n_open = count_open(line)
            n_close = count_close(line)

            if (n_open > 0) {
                seen_open = 1
                depth += n_open
                depth -= n_close
            }

            # 极少见情况：int test() { ... }
            if (seen_open && depth == 0) {
                write_case()

                in_test = 0
                seen_open = 0
                depth = 0
                buf = ""
            }

            next
        }

        next
    }

    # 已经处于 test() 内
    buf = buf line "\n"

    n_open = count_open(line)
    n_close = count_close(line)

    if (n_open > 0)
        seen_open = 1

    depth += n_open
    depth -= n_close

    # {} 完整闭合
    if (seen_open && depth == 0) {

        write_case()

        in_test = 0
        seen_open = 0
        depth = 0
        buf = ""
    }
}

END {
    if (in_test) {
        print "warning: incomplete test() at end of input" > "/dev/stderr"
    }
}
' "$INPUT"