#!/bin/bash

# ============ 在这里配置要扫描的头文件 ============
FILES=(
    "enums.h"
    # 可以继续加: "parser.h" "semantic.h" ...
)

# ============ 下面不用改 ============
for file in "${FILES[@]}"; do
    [ ! -f "$file" ] && echo "skip $file (not found)" && continue
    
    base="${file%.*}"
    OUTPUT="${base}_string.cpp"

    if [ -f "$OUTPUT" ] && [ "$OUTPUT" -nt "$file" ]; then
        echo "skip $OUTPUT (up to date)"
        continue
    fi

    TMPFILE=$(mktemp "${base}_string.XXXXXX.cpp")
    echo "#include \"$file\"" > "$TMPFILE"
    echo "// from $file" >> "$TMPFILE"
    echo "" >> "$TMPFILE"

    awk '
    { gsub(/\/\*[^*]*\*+([^/*][^*]*\*+)*\//, " ") }
    /^[[:space:]]*enum[[:space:]]+[A-Za-z_][A-Za-z0-9_]*/ {
        match($0, /enum[[:space:]]+([A-Za-z_][A-Za-z0-9_]*)/, arr)
        enum_name = arr[1]
        if ($0 ~ /\{/) {
            delete items
            count = 0
            in_enum = 1
            has_assign = 0
        } else {
            waiting_brace = 1
        }
        next
    }
    waiting_brace {
        if ($0 ~ /\{/) {
            delete items
            count = 0
            in_enum = 1
            waiting_brace = 0
            has_assign = 0
        }
        next
    }
    in_enum {
        if ($0 ~ /^[[:space:]]*\}/) {
            if (!has_assign && count > 0) {
                table_name = enum_name "_string"
                last = items[count-1]
                printf "const char *%s[%s + 1] = {\n", table_name, last
                for (i = 0; i < count; i++) {
                    if (items[i] == "---BLANK---") {
                        printf "\n"
                    } else {
                        printf "\t[%s] = \"%s\",\n", items[i], items[i]
                    }
                }
                printf "};\n\n"
            }
            in_enum = 0
            next
        }
        # 整行里有 = ，跳过这个 enum
        if ($0 ~ /=/) {
            has_assign = 1
        }
        if ($0 ~ /^[[:space:]]*$/) {
            if (count > 0 && items[count-1] != "---BLANK---")
                items[count++] = "---BLANK---"
            next
        }
        line = $0
        sub(/\/\/.*/, "", line)
        num = split(line, parts, ",")
        for (i = 1; i <= num; i++) {
            p = parts[i]
            # 有 = 说明是显式赋值别名，跳过这个名字
            if (index(p, "=") > 0) continue
            gsub(/[[:space:]]/, "", p)
            if (p != "") items[count++] = p
        }
    }
    ' "$file" >> "$TMPFILE"

    if [ -f "$OUTPUT" ] && cmp -s "$TMPFILE" "$OUTPUT"; then
        rm "$TMPFILE"
        echo "skip $OUTPUT (content unchanged)"
    else
        mv "$TMPFILE" "$OUTPUT"
        echo "generated $OUTPUT"
    fi
done