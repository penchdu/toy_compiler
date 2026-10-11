# Configurable VSL Random Differential Tester

This version reads generator/test settings from `vsl_fuzz_config.json`. You can change the syntax constructs and test intensity without editing Python.

## Quick start

Unzip this package, then copy these files **into your existing** `test/python_random_test/` directory, replacing the old Python and shell scripts:

- `vsl_diff_fuzz.py`
- `vsl_fuzz_config.json`
- `python_random_test.sh`
- Optionally, `vsl_fuzz_config_no_if_else.json` for the no-branch stress profile

Keeping them in the existing directory preserves the configured relative path `../../build/vsc`.

Then run:

```bash
./python_random_test.sh
```

The default config runs 1,000 programs, uses a time-derived seed, chooses 8–140 data variables, allows randomized nesting depth 30, and uses a per-program AST statement budget of 320. The script prints the seed so the run can be reproduced.

For backward compatibility, the first numeric shell argument overrides the number of tests:

```bash
./python_random_test.sh 500
```

Additional Python CLI options override the JSON settings:

```bash
./python_random_test.sh --seed 12345 --cases 200
python3 vsl_diff_fuzz.py --config vsl_fuzz_config.json --generate-only --cases 50
```

Relative paths in a config (such as `tools.vsc` and `test.failure_dir`) are resolved relative to the config file's directory.

## Configuration

`vsl_fuzz_config.json` looks like this:

```json
{
  "test": {
    "cases": 1000,
    "seed": "time",
    "timeout_seconds": 3.0,
    "failure_dir": "fuzz_failures",
    "generate_only": false
  },
  "tools": {
    "vsc": "../../build/vsc",
    "reference_compiler": "g++",
    "assembler": "gcc"
  },
  "generation": {
    "vars_min": 8,
    "vars_max": 140,
    "max_depth": 30,
    "statement_budget": 320,
    "tokens": {
      "if": true,
      "else": true,
      "while": true,
      "break": true,
      "continue": true
    },
    "force_statements_after_break_continue": true
  }
}
```

### Main options

| Setting | Meaning |
|---|---|
| `test.cases` | Number of generated programs |
| `test.seed` | Integer for a repeatable run, or `"time"` for a fresh time-based seed |
| `test.timeout_seconds` | Timeout per compile/run command |
| `test.failure_dir` | Directory where failing cases and logs are saved |
| `tools.vsc` | VSC executable; relative paths are relative to the config file |
| `generation.vars_min` / `vars_max` | Minimum and maximum number of data variables (`v0`, `v1`, …); loop counters are additional variables |
| `generation.max_depth` | Maximum randomized nested `if`/`while` depth; when `while` is enabled the generator also includes a guaranteed outer loop |
| `generation.statement_budget` | Random statement-generation budget for each program |
| `generation.tokens.if` | Allow generation of `if` statements |
| `generation.tokens.else` | Include `else` arms when an `if` is generated |
| `generation.tokens.while` | Allow nested loops and enable the guaranteed outer loop |
| `generation.tokens.break` | Allow `break` inside loops |
| `generation.tokens.continue` | Allow `continue` inside loops |
| `generation.force_statements_after_break_continue` | Force one or two sibling statements after a `break`/`continue` when budget permits, to exercise unreachable-code handling |

Tokens can be mixed to create targeted profiles. For example, to test loop-state handling without `if/else` branch `spill all`, edit `tokens` to:

```json
"tokens": {
  "if": false,
  "else": false,
  "while": true,
  "break": true,
  "continue": true
}
```

An equivalent preset is provided as `vsl_fuzz_config_no_if_else.json`; run it with:

```bash
python3 vsl_diff_fuzz.py --config vsl_fuzz_config_no_if_else.json
```

`break` and `continue` are generated only when a loop encloses them. If `while` is disabled, they cannot appear. The generator avoids the comma operator and unary `+` / `-`. Failed cases also include `run_settings.json` recording the seed, token flags, limits, and tool paths.

## Failure artifacts

A failing case is saved under the configured failure directory with source, reference source, generated assembly if available, seed, and compile/run logs. The top-level master seed and per-case seed are printed so you can reproduce the run.
