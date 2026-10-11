#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Backwards compatibility: `./python_random_test.sh 500` means --cases 500.
EXTRA_ARGS=()
if [[ $# -gt 0 && "$1" =~ ^[0-9]+$ ]]; then
    EXTRA_ARGS+=(--cases "$1")
    shift
fi

# Defaults (including a time-derived seed) come from vsl_fuzz_config.json.
# Any extra CLI options override the corresponding config values.
python3 "$SCRIPT_DIR/vsl_diff_fuzz.py" \
    --config "$SCRIPT_DIR/vsl_fuzz_config.json" \
    "${EXTRA_ARGS[@]}" "$@"
