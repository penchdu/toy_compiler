#!/bin/sh
set -eu
DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
for SRC in "$DIR"/*.cpp; do
    WRAP=$(mktemp --suffix=.c)
    BIN=$(mktemp)
    trap 'rm -f "$WRAP" "$BIN"' EXIT HUP INT TERM
    printf '#include <stdio.h>\n#include "%s"\nint main(void) { printf("%%d\\n", test()); return 0; }\n' "$SRC" > "$WRAP"
    gcc -std=c11 -O0 -Wall -Wextra "$WRAP" -o "$BIN"
    printf '%s: ' "$(basename "$SRC")"
    "$BIN"
    rm -f "$WRAP" "$BIN"
    trap - EXIT HUP INT TERM
done
