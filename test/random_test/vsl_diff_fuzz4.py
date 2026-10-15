#!/usr/bin/env python3
"""Differential tester for the VSL compiler's generated x86-64 assembly.

It creates small programs using a conservative VSL subset, runs them through:
  1) g++ as the reference implementation, and
  2) ./build/vsc followed by gcc a1.s -o <exe>.

The generated programs avoid unsupported syntax such as the comma operator
and unary plus/minus. `break` and `continue` are generated only inside loops,
both directly in loop bodies and under if/else. Random statements are also
intentionally emitted after break/continue, creating unreachable source code
that still exercises compiler control-flow and register-state handling.
Failing cases are preserved with generated source and logs.
"""
from __future__ import annotations

import argparse
import json
import os
import random
import re
import shutil
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Optional


@dataclass
class GeneratedCase:
    source: str
    seed: int
    data_vars: list[str]
    counter_vars: list[str]
    break_count: int
    continue_count: int
    direct_break_count: int
    direct_continue_count: int
    post_control_statement_count: int


class ProgramGenerator:
    def __init__(self, seed: int, data_count: int, max_depth: int, budget: int,
                 tokens: dict[str, bool], force_dead_tail: bool):
        self.rng = random.Random(seed)
        self.seed = seed
        self.data_vars = [f"v{i}" for i in range(data_count)]
        self.counter_vars: list[str] = []
        self.max_depth = max_depth
        self.budget = budget
        self.tokens = dict(tokens)
        self.force_dead_tail = force_dead_tail
        self.next_counter = 0
        self.break_count = 0
        self.continue_count = 0
        # "Direct" means no if/else branch intervenes between the current
        # loop body and this break/continue. Entering a nested while starts a
        # new loop body, so its direct controls are counted independently.
        self.direct_break_count = 0
        self.direct_continue_count = 0
        self.post_control_statement_count = 0

    def choose_data(self) -> str:
        return self.rng.choice(self.data_vars)

    def expr(self) -> str:
        mode = self.rng.randrange(5)
        a = self.choose_data()
        b = self.choose_data()
        if mode == 0:
            return f"({a} + {b}) / {self.rng.choice([2, 3, 4])}"
        if mode == 1:
            return f"({a} - {b}) / {self.rng.choice([2, 3, 4])}"
        if mode == 2:
            offset = self.rng.randint(-3, 3)
            return f"{a} + {offset}" if offset >= 0 else f"{a} - {abs(offset)}"
        if mode == 3:
            offset = self.rng.randint(-3, 3)
            return f"{a} - {offset}" if offset >= 0 else f"{a} + {abs(offset)}"
        offset = self.rng.randint(-2, 2)
        tail = f" + {offset}" if offset >= 0 else f" - {abs(offset)}"
        return f"({a} + {b}) / 2{tail}"

    def condition(self) -> str:
        a = self.choose_data()
        b = self.choose_data()
        return f"{a} > {b}"

    def new_counter(self) -> str:
        name = f"c{self.next_counter}"
        self.next_counter += 1
        self.counter_vars.append(name)
        return name

    def indent(self, lines: list[str], level: int) -> list[str]:
        prefix = "    " * level
        return [prefix + line if line else "" for line in lines]

    def block(
        self,
        depth: int,
        level: int,
        loop_depth: int = 0,
        branch_depth: int = 0,
    ) -> list[str]:
        """Generate sibling statements in one lexical block.

        `loop_depth` controls whether break/continue are legal. `branch_depth`
        counts if/else blocks since entering the current loop body, allowing the
        summary to show that controls are also generated directly in loop bodies.
        """
        if self.budget <= 0:
            return []

        count = self.rng.randint(2, 4)
        out: list[str] = []
        for _ in range(count):
            if self.budget <= 0:
                break
            out.extend(self.statement(depth, level, loop_depth, branch_depth))
        return out

    def statement(
        self,
        depth: int,
        level: int,
        loop_depth: int,
        branch_depth: int,
        force_dead_tail: bool = True,
    ) -> list[str]:
        """Generate one statement, optionally forcing code after loop control.

        A break/continue is followed by one or two more randomly generated
        statements whenever the statement budget permits. Those statements are
        intentionally unreachable along the path that executes the control
        statement, but are valid source and are still important for testing a
        compiler's handling of dead code/basic-block state.
        """
        if self.budget <= 0:
            return []

        self.budget -= 1
        pad = "    " * level
        can_recurse = depth < self.max_depth and self.budget > 0
        kinds = ["assign"]
        weights = [6]
        if can_recurse and self.tokens["if"]:
            kinds.append("if")
            weights.append(3)
        if can_recurse and self.tokens["while"]:
            kinds.append("while")
            weights.append(2)

        if loop_depth > 0:
            # Control statements are independently configurable and do not
            # require an if/else wrapper.
            if self.tokens["break"]:
                kinds.append("break")
                weights.append(2)
            if self.tokens["continue"]:
                kinds.append("continue")
                weights.append(2)

        kind = self.rng.choices(kinds, weights=weights, k=1)[0]
        out: list[str] = []

        if kind in ("break", "continue"):
            out.append(f"{pad}{kind};")
            if kind == "break":
                self.break_count += 1
                if branch_depth == 0:
                    self.direct_break_count += 1
            else:
                self.continue_count += 1
                if branch_depth == 0:
                    self.direct_continue_count += 1

            # Deliberately emit code immediately after break/continue. The
            # forced tail is generated at the same lexical level; it may itself
            # be an assignment, if/else, or nested while. Prevent recursively
            # forcing another tail for the first tail statement, but continue
            # normal random generation within any nested blocks it creates.
            if force_dead_tail and self.force_dead_tail and self.budget > 0:
                tail_count = self.rng.randint(1, 2)
                for _ in range(tail_count):
                    if self.budget <= 0:
                        break
                    tail = self.statement(
                        depth, level, loop_depth, branch_depth,
                        force_dead_tail=False,
                    )
                    if not tail:
                        break
                    out.extend(tail)
                    self.post_control_statement_count += 1
            return out

        if kind == "assign":
            target = self.choose_data()
            out.append(f"{pad}{target} = {self.expr()};")
            return out

        if kind == "if":
            then_lines = self.block(depth + 1, level + 1, loop_depth, branch_depth + 1)
            out.append(f"{pad}if ({self.condition()})")
            out.append(f"{pad}{{")
            out.extend(then_lines)
            out.append(f"{pad}}}")
            if self.tokens["else"]:
                else_lines = self.block(depth + 1, level + 1, loop_depth, branch_depth + 1)
                out.append(f"{pad}else")
                out.append(f"{pad}{{")
                out.extend(else_lines)
                out.append(f"{pad}}}")
            return out

        # Every generated loop uses a dedicated counter and decreases it in
        # the condition. Random data assignments never modify counters, which
        # guarantees termination even if a continue is executed repeatedly.
        counter = self.new_counter()
        trips = self.rng.randint(0, 4)
        # Reset branch_depth for the new loop body: controls can be generated
        # directly in that body, even if this loop itself sits inside an if.
        body = self.block(depth + 1, level + 1, loop_depth + 1, branch_depth=0)
        out.append(f"{pad}{counter} = {trips + 1};")
        out.append(f"{pad}while (({counter} = {counter} - 1) > 0)")
        out.append(f"{pad}{{")
        out.extend(body)
        out.append(f"{pad}}}")
        return out

    def generate(self) -> GeneratedCase:
        prelude = [f"int {name} = {self.rng.randint(0, 12)};" for name in self.data_vars]
        lines = ["int test()", "{"]

        # Preserve a guaranteed outer loop only when while is enabled. This
        # makes it possible to test syntax subsets with loops disabled.
        if self.tokens["while"]:
            outer = self.new_counter()
            outer_trips = self.rng.randint(1, 4)
            body = self.block(depth=1, level=2, loop_depth=1, branch_depth=0)
            lines.extend(self.indent(prelude, 1))
            lines.append("")
            lines.append(f"    {outer} = {outer_trips + 1};")
            lines.append(f"    while (({outer} = {outer} - 1) > 0)")
            lines.append("    {")
            lines.extend(body)
            lines.append("    }")
        else:
            lines.extend(self.indent(prelude, 1))

        # Optional guaranteed branch keeps branch-merge pressure in the default
        # profile, but can be disabled from the config along with all if syntax.
        if self.tokens["if"]:
            branch = [
                f"    if ({self.condition()})",
                "    {",
                f"        {self.choose_data()} = {self.expr()};",
                "    }",
            ]
            if self.tokens["else"]:
                branch.extend([
                    "    else",
                    "    {",
                    f"        {self.choose_data()} = {self.expr()};",
                    "    }",
                ])
            lines.append("")
            lines.extend(branch)

        # Generate additional statements outside the guaranteed outer loop too.
        after_loop = self.block(depth=0, level=1, loop_depth=0, branch_depth=0)
        lines.append("")
        lines.extend(after_loop)
        lines.append("")

        # Counter declarations are emitted after generation because nested
        # random blocks may allocate more counters.
        counters = [f"int {name} = 0;" for name in self.counter_vars]
        # Insert declarations immediately after the function opening brace.
        lines[2:2] = self.indent(counters, 1)
        if counters:
            lines.insert(2 + len(counters), "")
        result_vars = self.data_vars + self.counter_vars
        lines.append("    return " + " + ".join(result_vars) + ";")
        lines.append("}")
        return GeneratedCase(
            "\n".join(lines) + "\n",
            self.seed,
            self.data_vars[:],
            self.counter_vars[:],
            self.break_count,
            self.continue_count,
            self.direct_break_count,
            self.direct_continue_count,
            self.post_control_statement_count,
        )


def run_cmd(cmd: list[str], cwd: Path, timeout: float) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        cmd,
        cwd=str(cwd),
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=timeout,
        check=False,
    )


def result_value(stdout: str) -> Optional[int]:
    match = re.search(r"Result:\s*(-?\d+)", stdout)
    return int(match.group(1)) if match else None


def write_logs(folder: Path, logs: dict[str, str]) -> None:
    for name, content in logs.items():
        (folder / name).write_text(content, encoding="utf-8", errors="replace")


def save_failure(tmp_dir: Path, output_dir: Path, label: str, category: str, detail: str) -> Path:
    dest = output_dir / label
    dest.mkdir(parents=True, exist_ok=True)
    for item in tmp_dir.iterdir():
        target = dest / item.name
        if item.is_dir():
            shutil.copytree(item, target, dirs_exist_ok=True)
        else:
            shutil.copy2(item, target)
    (dest / "FAILURE.txt").write_text(f"category: {category}\n{detail}\n", encoding="utf-8")
    return dest


DEFAULT_CONFIG = {
    "test": {
        "cases": 1000,
        "seed": "time",
        "timeout_seconds": 3.0,
        "failure_dir": "fuzz_failures",
        "generate_only": False,
    },
    "tools": {
        "vsc": "../../build/vsc",
        "reference_compiler": "g++",
        "assembler": "gcc",
    },
    "generation": {
        "vars_min": 8,
        "vars_max": 140,
        "max_depth": 30,
        "statement_budget": 320,
        "tokens": {
            "if": True,
            "else": True,
            "while": True,
            "break": True,
            "continue": True,
        },
        "force_statements_after_break_continue": True,
    },
}


def load_config(path: Path) -> dict:
    try:
        raw = json.loads(path.read_text(encoding="utf-8"))
    except OSError as exc:
        raise ValueError(f"cannot read config file {path}: {exc}") from exc
    except json.JSONDecodeError as exc:
        raise ValueError(f"invalid JSON in config file {path}: {exc}") from exc
    if not isinstance(raw, dict):
        raise ValueError("config root must be a JSON object")

    # Merge known sections with defaults, including token flags individually.
    merged = {
        "test": {**DEFAULT_CONFIG["test"], **(raw.get("test") or {})},
        "tools": {**DEFAULT_CONFIG["tools"], **(raw.get("tools") or {})},
        "generation": {**DEFAULT_CONFIG["generation"], **(raw.get("generation") or {})},
    }
    tokens = (raw.get("generation") or {}).get("tokens") or {}
    merged["generation"]["tokens"] = {**DEFAULT_CONFIG["generation"]["tokens"], **tokens}

    unknown_sections = set(raw) - {"test", "tools", "generation"}
    if unknown_sections:
        raise ValueError(f"unknown top-level config section(s): {', '.join(sorted(unknown_sections))}")
    unknown_tokens = set(merged["generation"]["tokens"]) - set(DEFAULT_CONFIG["generation"]["tokens"])
    if unknown_tokens:
        raise ValueError(f"unknown generation token(s): {', '.join(sorted(unknown_tokens))}")
    for key, value in merged["generation"]["tokens"].items():
        if not isinstance(value, bool):
            raise ValueError(f"generation.tokens.{key} must be true or false")
    if not isinstance(merged["generation"]["force_statements_after_break_continue"], bool):
        raise ValueError("generation.force_statements_after_break_continue must be true or false")
    if not isinstance(merged["test"]["generate_only"], bool):
        raise ValueError("test.generate_only must be true or false")
    return merged


def resolve_path(value: str, base: Path) -> Path:
    path = Path(value).expanduser()
    return path.resolve() if path.is_absolute() else (base / path).resolve()


def main() -> int:
    parser = argparse.ArgumentParser(description="Random differential testing for VSC/VSL")
    parser.add_argument("--config", default=None, help="JSON config file (default: vsl_fuzz_config.json next to this script)")
    parser.add_argument("--vsc", default=None, help="override VSC executable path")
    parser.add_argument("--cases", type=int, default=None, help="override number of programs")
    parser.add_argument("--seed", type=int, default=None, help="override master seed; reuse it to reproduce a run")
    parser.add_argument("--out", default=None, help="override directory for failure artifacts")
    parser.add_argument("--timeout", type=float, default=None, help="override timeout per command, seconds")
    parser.add_argument("--vars-min", type=int, default=None, help="override minimum data variables")
    parser.add_argument("--vars-max", type=int, default=None, help="override maximum data variables")
    parser.add_argument("--depth", type=int, default=None, help="override maximum randomized nesting depth")
    parser.add_argument("--budget", type=int, default=None, help="override AST statement budget per program")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--generate-only", dest="generate_only", action="store_true", default=None,
                      help="only generate source files; do not compile")
    mode.add_argument("--run-tests", dest="generate_only", action="store_false",
                      help="run differential tests even if the config says generate_only=true")
    args = parser.parse_args()

    config_path = resolve_path(args.config, Path.cwd()) if args.config else Path(__file__).resolve().with_name("vsl_fuzz_config.json")
    try:
        config = load_config(config_path)
    except ValueError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        print("Create vsl_fuzz_config.json next to this script or pass --config /path/to/config.json.", file=sys.stderr)
        return 2

    test_cfg = config["test"]
    tool_cfg = config["tools"]
    gen_cfg = config["generation"]
    tokens = gen_cfg["tokens"]

    cases = args.cases if args.cases is not None else test_cfg["cases"]
    seed_value = args.seed if args.seed is not None else test_cfg["seed"]
    seed = time.time_ns() if seed_value in (None, "time", "random") else seed_value
    timeout = args.timeout if args.timeout is not None else test_cfg["timeout_seconds"]
    vars_min = args.vars_min if args.vars_min is not None else gen_cfg["vars_min"]
    vars_max = args.vars_max if args.vars_max is not None else gen_cfg["vars_max"]
    depth = args.depth if args.depth is not None else gen_cfg["max_depth"]
    budget = args.budget if args.budget is not None else gen_cfg["statement_budget"]
    generate_only = args.generate_only if args.generate_only is not None else test_cfg["generate_only"]
    force_dead_tail = gen_cfg["force_statements_after_break_continue"]

    if not isinstance(seed, int) or isinstance(seed, bool):
        parser.error("test.seed must be an integer, null, or 'time'")
    for key, value in (("cases", cases), ("vars_min", vars_min), ("vars_max", vars_max),
                       ("max_depth", depth), ("statement_budget", budget)):
        if not isinstance(value, int) or isinstance(value, bool):
            parser.error(f"configuration value {key} must be an integer")
    if not isinstance(timeout, (int, float)) or isinstance(timeout, bool):
        parser.error("timeout_seconds / --timeout must be a number")
    if cases < 1:
        parser.error("test.cases / --cases must be >= 1")
    if vars_min < 2 or vars_max < vars_min:
        parser.error("require 2 <= vars_min / --vars-min <= vars_max / --vars-max")
    if depth < 1 or budget < 1:
        parser.error("max_depth / --depth and statement_budget / --budget must be >= 1")
    if timeout <= 0:
        parser.error("timeout_seconds / --timeout must be > 0")
    if not isinstance(force_dead_tail, bool):
        parser.error("generation.force_statements_after_break_continue must be true or false")

    # A break/continue token without any while construct cannot be emitted.
    if not tokens["while"] and (tokens["break"] or tokens["continue"]):
        print("NOTE: while is disabled, so break/continue cannot be generated even if enabled.")

    config_dir = config_path.parent
    if args.vsc is not None:
        vsc = Path(args.vsc).expanduser().resolve()
    else:
        vsc = resolve_path(str(tool_cfg["vsc"]), config_dir)
    if not generate_only and not vsc.exists():
        print(f"ERROR: VSC executable not found: {vsc}", file=sys.stderr)
        print("Set tools.vsc in the config or pass --vsc /absolute/path/to/build/vsc.", file=sys.stderr)
        return 2

    if args.out is not None:
        output_dir = Path(args.out).expanduser().resolve()
    else:
        output_dir = resolve_path(str(test_cfg["failure_dir"]), config_dir)
    if generate_only:
        output_dir.mkdir(parents=True, exist_ok=True)

    reference_compiler = str(tool_cfg["reference_compiler"])
    assembler = str(tool_cfg["assembler"])
    print(f"Config: {config_path}")
    print(f"Random seed: {seed}")
    print(f"Cases: {cases}; variables: {vars_min}..{vars_max}; max depth: {depth}; budget: {budget}")
    print("Enabled tokens: " + (", ".join(name for name, enabled in tokens.items() if enabled) or "(none)"))
    print(f"Force statements after break/continue: {force_dead_tail}")

    counts = {"pass": 0, "vsc_failed": 0, "assemble_failed": 0, "runtime_mismatch": 0,
              "reference_failed": 0, "timeout": 0, "bad_output": 0}
    failure_paths: list[Path] = []
    total_breaks = 0
    total_continues = 0
    total_direct_breaks = 0
    total_direct_continues = 0
    total_post_control_statements = 0

    for index in range(cases):
        case_seed = seed + index * 104729
        case_rng = random.Random(case_seed ^ 0x5A17)
        data_count = case_rng.randint(vars_min, vars_max)
        generated = ProgramGenerator(case_seed, data_count, depth, budget, tokens, force_dead_tail).generate()
        total_breaks += generated.break_count
        total_continues += generated.continue_count
        total_direct_breaks += generated.direct_break_count
        total_direct_continues += generated.direct_continue_count
        total_post_control_statements += generated.post_control_statement_count
        label = f"case_{index + 1:05d}_seed_{case_seed}"

        run_settings = {
            "config_file": str(config_path),
            "master_seed": seed,
            "case_seed": case_seed,
            "case_index": index + 1,
            "cases": cases,
            "vars_min": vars_min,
            "vars_max": vars_max,
            "max_depth": depth,
            "statement_budget": budget,
            "tokens": tokens,
            "force_statements_after_break_continue": force_dead_tail,
            "vsc": str(vsc),
            "reference_compiler": reference_compiler,
            "assembler": assembler,
            "timeout_seconds": timeout,
        }
        if generate_only:
            case_dir = output_dir / label
            case_dir.mkdir(parents=True, exist_ok=True)
            (case_dir / "case.cpp").write_text(generated.source, encoding="utf-8")
            (case_dir / "run_settings.json").write_text(json.dumps(run_settings, indent=2) + "\n", encoding="utf-8")
            (case_dir / "seed.txt").write_text(str(case_seed) + "\n", encoding="utf-8")
            (case_dir / "control_flow.txt").write_text(
                f"break={generated.break_count}\n"
                f"continue={generated.continue_count}\n"
                f"direct_break={generated.direct_break_count}\n"
                f"direct_continue={generated.direct_continue_count}\n"
                f"post_control_statements={generated.post_control_statement_count}\n",
                encoding="utf-8",
            )
            counts["pass"] += 1
            continue

        temp_path = Path(tempfile.mkdtemp(prefix="vsl_diff_fuzz_"))
        source_path = temp_path / "case.cpp"
        reference_path = temp_path / "reference.cpp"
        (temp_path / "run_settings.json").write_text(json.dumps(run_settings, indent=2) + "\n", encoding="utf-8")
        source_path.write_text(generated.source, encoding="utf-8")
        reference_source = (
            "#include <stdio.h>\n\n"
            + generated.source
            + '\nint main(void) { printf("Result: %d\\n", test()); return 0; }\n'
        )
        reference_path.write_text(reference_source, encoding="utf-8")
        (temp_path / "seed.txt").write_text(
            f"master_seed={seed}\ncase_seed={case_seed}\nindex={index + 1}\n",
            encoding="utf-8",
        )
        logs: dict[str, str] = {}
        category: Optional[str] = None
        detail = ""

        try:
            ref_compile = run_cmd([reference_compiler, "-std=c++17", "-O0", "reference.cpp", "-o", "reference"], temp_path, timeout)
            logs["reference_compile.log"] = f"$ {reference_compiler} -std=c++17 -O0 reference.cpp -o reference\nexit={ref_compile.returncode}\nSTDOUT:\n{ref_compile.stdout}\nSTDERR:\n{ref_compile.stderr}"
            if ref_compile.returncode != 0:
                category = "reference_failed"
                detail = "The generated case did not compile with host g++; likely generator or syntax issue."
            else:
                ref_run = run_cmd(["./reference"], temp_path, timeout)
                logs["reference_run.log"] = f"$ ./reference\nexit={ref_run.returncode}\nSTDOUT:\n{ref_run.stdout}\nSTDERR:\n{ref_run.stderr}"
                if ref_run.returncode != 0:
                    category = "reference_failed"
                    detail = "Reference executable returned a non-zero status."
                else:
                    expected = result_value(ref_run.stdout)
                    if expected is None:
                        category = "bad_output"
                        detail = "Could not parse `Result: <integer>` from host output."
                    else:
                        vsc_run = run_cmd([str(vsc), "case.cpp"], temp_path, timeout)
                        logs["vsc_compile.log"] = f"$ {vsc} case.cpp\nexit={vsc_run.returncode}\nSTDOUT:\n{vsc_run.stdout}\nSTDERR:\n{vsc_run.stderr}"
                        if vsc_run.returncode != 0 or not (temp_path / "a1.s").exists():
                            category = "vsc_failed"
                            detail = "VSC failed or did not produce a1.s. Check vsc_compile.log."
                        else:
                            asm = run_cmd([assembler, "a1.s", "-o", "vsl_exec"], temp_path, timeout)
                            logs["assemble.log"] = f"$ {assembler} a1.s -o vsl_exec\nexit={asm.returncode}\nSTDOUT:\n{asm.stdout}\nSTDERR:\n{asm.stderr}"
                            if asm.returncode != 0:
                                category = "assemble_failed"
                                detail = "GCC could not assemble/link VSC-generated a1.s."
                            else:
                                vsl_run = run_cmd(["./vsl_exec"], temp_path, timeout)
                                logs["vsl_run.log"] = f"$ ./vsl_exec\nexit={vsl_run.returncode}\nSTDOUT:\n{vsl_run.stdout}\nSTDERR:\n{vsl_run.stderr}"
                                actual = result_value(vsl_run.stdout)
                                # Do not treat every non-zero process status as a test failure:
                                # the VSC-generated entry point may return a value in EAX, so
                                # the OS exit status can be non-zero even when the printed
                                # `Result: ...` matches the reference. The printed value is the
                                # differential-test oracle. A signal termination is still a failure.
                                if vsl_run.returncode < 0:
                                    category = "runtime_mismatch"
                                    detail = (
                                        f"VSL executable terminated by signal {-vsl_run.returncode}; "
                                        f"printed result={actual!r}; reference result={expected}."
                                    )
                                elif actual is None:
                                    if vsl_run.returncode != 0:
                                        category = "runtime_mismatch"
                                        detail = (
                                            f"VSL executable exited with status {vsl_run.returncode} "
                                            "and produced no parseable Result line; "
                                            f"reference result={expected}."
                                        )
                                    else:
                                        category = "bad_output"
                                        detail = "Could not parse `Result: <integer>` from VSL output."
                                elif actual != expected:
                                    category = "runtime_mismatch"
                                    detail = (
                                        f"Expected {expected} from g++; got {actual} from "
                                        f"VSC-generated a1.s (process exit status {vsl_run.returncode})."
                                    )
                                else:
                                    counts["pass"] += 1

        except subprocess.TimeoutExpired as exc:
            category = "timeout"
            detail = f"Timed out while running command: {exc.cmd}"
            logs["timeout.log"] = detail + "\n"
        except OSError as exc:
            category = "vsc_failed"
            detail = f"Could not execute a required command: {exc}"
            logs["os_error.log"] = detail + "\n"

        write_logs(temp_path, logs)
        if category is not None:
            counts[category] = counts.get(category, 0) + 1
            saved = save_failure(temp_path, output_dir, label, category, detail)
            failure_paths.append(saved)
            print(f"FAIL [{category}] {label}: {detail}")
        elif (index + 1) % 10 == 0 or index + 1 == cases:
            print(f"progress: {index + 1}/{cases} processed; passed={counts['pass']}")

        shutil.rmtree(temp_path, ignore_errors=True)

    if generate_only:
        print(f"Generated {cases} programs under: {output_dir}")
        print(f"Generated control statements: break={total_breaks}, continue={total_continues}")
        print(f"Direct loop-body controls: break={total_direct_breaks}, continue={total_direct_continues}")
        print(f"Statements forced after break/continue: {total_post_control_statements}")
        print("`break` and `continue` are generated only inside loop bodies; code is also generated after them.")
        print("The generator avoids the comma operator and unary +/-.")
        return 0

    print("\n===== Differential test summary =====")
    print(f"total: {cases}")
    print(f"generated break statements: {total_breaks}")
    print(f"generated continue statements: {total_continues}")
    print(f"direct loop-body break statements: {total_direct_breaks}")
    print(f"direct loop-body continue statements: {total_direct_continues}")
    print(f"statements forced after break/continue: {total_post_control_statements}")
    for key in ["pass", "vsc_failed", "assemble_failed", "runtime_mismatch", "reference_failed", "timeout", "bad_output"]:
        if counts[key]:
            print(f"{key}: {counts[key]}")
    if failure_paths:
        print(f"Failure artifacts: {output_dir}")
        print("Each failing directory includes case.cpp, reference.cpp, a1.s if produced, and command logs.")
        print("To reproduce a case, run VSC manually on that directory's case.cpp and compare reference.cpp.")
        return 1
    print("No differences or compile failures found.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
