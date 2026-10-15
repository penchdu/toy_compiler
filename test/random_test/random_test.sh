#!/usr/bin/env bash
set -euo pipefail

# Default to 1000 cases; an optional first argument overrides the count.
CASES="${1:-1000}"

# Use the current time in nanoseconds as the random seed.
SEED="$(date +%s%N)"

# Resolve paths relative to this script, so it works from any working directory.
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

printf 'Random seed: %s\n' "$SEED"
printf 'Cases: %s\n' "$CASES"

python3 vsl_diff_fuzz.py \
    --vsc ../../build/vsc \
    --cases "$CASES" \
    --seed "$SEED"  	


# --config vsl_fuzz_config_no_if_else.json
# --generate-only


#python3 vsl_diff_fuzz.py \
#    --vsc ../../build/vsc \
#    --cases 1000 \
#    --seed 2

