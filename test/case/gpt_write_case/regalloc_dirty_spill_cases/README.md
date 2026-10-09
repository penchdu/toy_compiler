# Dirty-VR spill regression cases for while condition/body handoff

These two programs target the two paths guarded by `cond_will_spill_this_dirty_vr`
inside `ra_wave__while()`.

## 13_cond_spill_covers_dirty_vr.cpp

Reference return value: **73**.

The condition writes `i`, then computes a deliberately right-nested sum before
using that result in the comparison. The body writes `i` and performs `x = x + 0`
on every condition operand, so those VRs are dirty after the body while their
values—and therefore the loop trip count—remain unchanged. The right-nested sum
keeps earlier operands live while later operands are evaluated, creating
register pressure intended to make the condition spill at least one body-dirty
VR.

Verify in allocator logs that the condition block's `x64mc_alloc_wave` contains
`MC_ST` for an `xN` VR that is also resident and dirty in `after_body_vr2pr`. If
this occurs, body-tail recovery should not append another `MC_ST` for that same
VR. Also inspect the logs rather than assuming `i` itself is the selected victim.

## 14_body_tail_must_spill.cpp

Reference return value: **49**.

The condition only updates/tests `i`; the body updates `a`, `b` and `c`, which
remain live after the loop. With this low-pressure condition, the intended path
is that the condition block has no `MC_ST` for those body-dirty VRs and the
body tail writes back any of them that remain resident and dirty.

Verify that `cond_mcs` contains no `MC_ST` for the corresponding `a`, `b`, or
`c` VRs, and that the body tail gets an `MC_ST` tagged `while recover st` for
at least one such resident dirty VR.

## Important

The reference values are from the C/C++ semantics. Whether a case actually
exercises the intended allocator branch depends on register-allocation choices
in the current compiler build, so inspect the generated `cond_mcs` and tail
instructions rather than relying on the filename alone. Run both through the
same `run_test.sh` harness used for the other regression cases.
