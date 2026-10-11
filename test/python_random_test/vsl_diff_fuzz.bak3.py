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
import os
import random
import re
import shutil
import subprocess
import sys
import tempfile
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
    def __init__(self, seed: int, data_count: int, max_depth: int, budget: int):
        self.rng = random.Random(seed)
        self.seed = seed
        self.data_vars = [f"v{i}" for i in range(data_count)]
        self.counter_vars: list[str] = []
        self.max_depth = max_depth
        self.budget = budget
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
        if can_recurse:
            kinds = ["assign", "if", "while"]
            weights = [6, 3, 2]
        else:
            # At the nesting limit only simple statements are generated, but
            # loop-control statements remain legal inside a loop.
            kinds = ["assign"]
            weights = [10]

        if loop_depth > 0:
            # Increase their chances a little so large fuzz runs exercise them
            # regularly, including without an if/else wrapper.
            kinds.extend(["break", "continue"])
            weights.extend([2, 2])

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
            if force_dead_tail and self.budget > 0:
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
            else_lines = self.block(depth + 1, level + 1, loop_depth, branch_depth + 1)
            out.append(f"{pad}if ({self.condition()})")
            out.append(f"{pad}{{")
            out.extend(then_lines)
            out.append(f"{pad}}}")
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
        # Build a guaranteed outer loop. The body also includes randomized
        # nested branches/loops, with a hard AST budget to keep compilation fast.
        outer = self.new_counter()
        outer_trips = self.rng.randint(1, 4)
        body = self.block(depth=1, level=2, loop_depth=1, branch_depth=0)
        after_loop = self.block(depth=0, level=1, loop_depth=0, branch_depth=0)

        # A guaranteed branch provides branch-merge pressure even when the
        # randomly selected block happens to contain no if statement.
        branch = [
            f"    if ({self.condition()})",
            "    {",
            f"        {self.choose_data()} = {self.expr()};",
            "    }",
            "    else",
            "    {",
            f"        {self.choose_data()} = {self.expr()};",
            "    }",
        ]

        # Counter declarations are emitted after body generation because nested
        # random blocks may have allocated more counters.
        counters = [f"int {name} = 0;" for name in self.counter_vars]
        lines = ["int test()", "{"]
        lines.extend(self.indent(prelude + counters, 1))
        lines.append("")
        lines.append(f"    {outer} = {outer_trips + 1};")
        lines.append(f"    while (({outer} = {outer} - 1) > 0)")
        lines.append("    {")
        lines.extend(body)
        lines.append("    }")
        lines.append("")
        # Keep a little distance from the loops before the final return.
        lines.extend(branch)
        lines.append("")
        lines.extend(after_loop)
        lines.append("")
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


def main() -> int:
    parser = argparse.ArgumentParser(description="Random differential testing for VSC/VSL")
    parser.add_argument("--vsc", default="./build/vsc", help="path to the VSC executable (default: ./build/vsc)")
    parser.add_argument("--cases", type=int, default=100, help="number of generated programs (default: 100)")
    parser.add_argument("--seed", type=int, default=1, help="master seed; reuse it to reproduce a run")
    parser.add_argument("--out", default="fuzz_failures", help="directory for failure artifacts")
    parser.add_argument("--timeout", type=float, default=3.0, help="timeout per compile/run command, seconds")
    parser.add_argument("--vars-min", type=int, default=8, help="minimum data variables (default: 8)")
    parser.add_argument("--vars-max", type=int, default=140, help="maximum data variables (default: 140)")
    parser.add_argument("--depth", type=int, default=30, help="maximum randomized nesting depth (default: 30)")
    parser.add_argument("--budget", type=int, default=320, help="AST statement budget per program (default: 320)")
    parser.add_argument("--generate-only", action="store_true", help="only generate source files; do not compile")
    args = parser.parse_args()

    if args.cases < 1:
        parser.error("--cases must be >= 1")
    if args.vars_min < 2 or args.vars_max < args.vars_min:
        parser.error("require 2 <= --vars-min <= --vars-max")
    if args.depth < 1 or args.budget < 1:
        parser.error("--depth and --budget must be >= 1")

    vsc = Path(args.vsc).expanduser().resolve()
    if not args.generate_only and not vsc.exists():
        print(f"ERROR: VSC executable not found: {vsc}", file=sys.stderr)
        print("Run from the VSC repository root, or pass --vsc /absolute/path/to/build/vsc.", file=sys.stderr)
        return 2

    output_dir = Path(args.out).expanduser().resolve()
    if args.generate_only:
        output_dir.mkdir(parents=True, exist_ok=True)

    counts = {"pass": 0, "vsc_failed": 0, "assemble_failed": 0, "runtime_mismatch": 0,
              "reference_failed": 0, "timeout": 0, "bad_output": 0}
    failure_paths: list[Path] = []
    total_breaks = 0
    total_continues = 0
    total_direct_breaks = 0
    total_direct_continues = 0
    total_post_control_statements = 0

    for index in range(args.cases):
        case_seed = args.seed + index * 104729
        case_rng = random.Random(case_seed ^ 0x5A17)
        data_count = case_rng.randint(args.vars_min, args.vars_max)
        generated = ProgramGenerator(case_seed, data_count, args.depth, args.budget).generate()
        total_breaks += generated.break_count
        total_continues += generated.continue_count
        total_direct_breaks += generated.direct_break_count
        total_direct_continues += generated.direct_continue_count
        total_post_control_statements += generated.post_control_statement_count
        label = f"case_{index + 1:05d}_seed_{case_seed}"

        if args.generate_only:
            case_dir = output_dir / label
            case_dir.mkdir(parents=True, exist_ok=True)
            (case_dir / "case.cpp").write_text(generated.source, encoding="utf-8")
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
        source_path.write_text(generated.source, encoding="utf-8")
        reference_source = (
            "#include <stdio.h>\n\n"
            + generated.source
            + '\nint main(void) { printf("Result: %d\\n", test()); return 0; }\n'
        )
        reference_path.write_text(reference_source, encoding="utf-8")
        (temp_path / "seed.txt").write_text(
            f"master_seed={args.seed}\ncase_seed={case_seed}\nindex={index + 1}\n",
            encoding="utf-8",
        )
        logs: dict[str, str] = {}
        category: Optional[str] = None
        detail = ""

        try:
            ref_compile = run_cmd(["g++", "-std=c++17", "-O0", "reference.cpp", "-o", "reference"], temp_path, args.timeout)
            logs["reference_compile.log"] = f"$ g++ -std=c++17 -O0 reference.cpp -o reference\nexit={ref_compile.returncode}\nSTDOUT:\n{ref_compile.stdout}\nSTDERR:\n{ref_compile.stderr}"
            if ref_compile.returncode != 0:
                category = "reference_failed"
                detail = "The generated case did not compile with host g++; likely generator or syntax issue."
            else:
                ref_run = run_cmd(["./reference"], temp_path, args.timeout)
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
                        vsc_run = run_cmd([str(vsc), "case.cpp"], temp_path, args.timeout)
                        logs["vsc_compile.log"] = f"$ {vsc} case.cpp\nexit={vsc_run.returncode}\nSTDOUT:\n{vsc_run.stdout}\nSTDERR:\n{vsc_run.stderr}"
                        if vsc_run.returncode != 0 or not (temp_path / "a1.s").exists():
                            category = "vsc_failed"
                            detail = "VSC failed or did not produce a1.s. Check vsc_compile.log."
                        else:
                            asm = run_cmd(["gcc", "a1.s", "-o", "vsl_exec"], temp_path, args.timeout)
                            logs["assemble.log"] = f"$ gcc a1.s -o vsl_exec\nexit={asm.returncode}\nSTDOUT:\n{asm.stdout}\nSTDERR:\n{asm.stderr}"
                            if asm.returncode != 0:
                                category = "assemble_failed"
                                detail = "GCC could not assemble/link VSC-generated a1.s."
                            else:
                                vsl_run = run_cmd(["./vsl_exec"], temp_path, args.timeout)
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
        elif (index + 1) % 10 == 0 or index + 1 == args.cases:
            print(f"progress: {index + 1}/{args.cases} processed; passed={counts['pass']}")

        shutil.rmtree(temp_path, ignore_errors=True)

    if args.generate_only:
        print(f"Generated {args.cases} programs under: {output_dir}")
        print(f"Generated control statements: break={total_breaks}, continue={total_continues}")
        print(f"Direct loop-body controls: break={total_direct_breaks}, continue={total_direct_continues}")
        print(f"Statements forced after break/continue: {total_post_control_statements}")
        print("`break` and `continue` are generated only inside loop bodies; code is also generated after them.")
        print("The generator still avoids the comma operator and unary +/-.")
        return 0

    print("\n===== Differential test summary =====")
    print(f"total: {args.cases}")
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
