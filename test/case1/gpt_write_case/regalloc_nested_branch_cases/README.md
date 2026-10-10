# Nested branch / while dirty-spill regression cases

These cases are intended to stress the wave allocator around loop-condition state, nested scopes, `if/else` joins, and dirty values that remain live after one or more loops. They are intentionally written as `int test()` to match the project's current examples.

## Focus of each case

| File | Primary stress point | GCC reference result |
|---|---|---:|
| `15_deep_if_dirty_inside_nested_loops.cpp` | Outer and inner loop counters are modified in deeper branches; three loop levels and nested `if/else` | 247 |
| `16_many_live_values_branch_rejoin.cpp` | 16 live data values, nested branches, repeated branch joins, values consumed after the loop | 793 |
| `17_loop_inside_only_one_branch.cpp` | Inner loop exists only in one arm; the other arm has a different loop and both arms feed the outer loop | 321 |
| `18_nested_counter_alias_and_deep_rejoin.cpp` | Deep branch mutates outer loop counters, followed by more nested-loop work and outer-scope uses | 270 |
| `19_zero_trip_then_branch_heavy_siblings.cpp` | Zero-trip loop followed by two sibling loops with nested branches | 280 |
| `20_pressure_at_deep_branch_return_all.cpp` | 20 live data values, three nested loops, deepest branch mutates values returned later | 719 |
| `21_condition_pressure_deep_branch_dirty.cpp` | Condition uses many values and mutates the loop counter; nested branch body dirties values live after exit | 438 |
| `22_outer_body_is_branch_only.cpp` | Outer while body's own block has no ordinary statements, only nested `if/else`; useful for testing non-recursive body scanning | 1787 |
| `23_outer_body_only_inner_loop_and_branch.cpp` | Outer while body contains only an inner loop; the inner loop body contains only `if/else` branches | 913 |

The reference results above are calculated by GCC using a small wrapper that calls `test()` and prints its return value. They are **reference values, not a claim that the current VSC allocator passes these tests**.

## Run a case through VSC

Extract the ZIP and place `regalloc_nested_branch_cases/` under your VSC project root. From the project root, run one case at a time because `a0.s` and `a1.s` are overwritten:

```sh
make
./build/vsc regalloc_nested_branch_cases/15_deep_if_dirty_inside_nested_loops.cpp
gcc a0.s -o /tmp/ra_a0
gcc a1.s -o /tmp/ra_a1
/tmp/ra_a0
/tmp/ra_a1
```

Compare both printed results with the GCC reference result for that case. Use the same commands with another case filename. A mismatch between `a1` and the reference is a likely wave-allocation issue; if `a0` also differs, first check whether the case exposed a more general code-generation or parser issue.

## Compile the GCC references independently

Each file defines `test()` rather than `main()`. To obtain the reference result for one case, create a wrapper such as:

```c
#include <stdio.h>
#include "15_deep_if_dirty_inside_nested_loops.cpp"
int main(void) { printf("%d\\n", test()); return 0; }
```

Then compile the wrapper with GCC. The expected values are listed in the table above.
