# VSL / x64 寄存器分配回归用例

每个 `.cpp` 都是独立输入文件，入口函数均为 `int test()`；请逐个编译/运行，不要把这些文件一起链接。
预期返回值按源码的整数运算语义手工写出的等价 Python 模拟计算；它们是测试 oracle，不代表已在你的编译器上实测通过。

| 文件 | 主要覆盖点 | 预期返回值 |
|---|---|---:|
| `case01_loop_if_dirty.cpp` | 循环内 if/else；同一组变量跨多次迭代被修改 | 79 |
| `case02_nested_while_if.cpp` | 双层 while，内层分支修改不同变量，外层循环后再合流 | 353 |
| `case03_nested_if_in_loop.cpp` | 循环中的嵌套 if；分支内部再次分叉 | 123 |
| `case04_zero_iteration.cpp` | while 循环一次也不执行；循环后继续使用原变量 | 36 |
| `case05_register_pressure.cpp` | 12 个以上变量、双层循环、分支更新与循环后汇总 | 497 |
| `case06_assignment_in_condition.cpp` | while 条件本身带赋值；条件变量及 carry 在循环体中变化 | -43 |
| `case07_three_level_loop.cpp` | 三层 while；每层循环后都有分支或后续使用 | 1659 |
| `case08_mixed_branch_merge.cpp` | 循环中多层 if/else；分支后有共同修改，循环后再次分支 | 472 |
| `case09_two_sequential_loops.cpp` | 两个连续循环；第一个循环写入的值在第二个循环使用 | 63 |
| `case10_many_live_values.cpp` | 较多长生命周期变量在循环体和循环后反复使用 | 37 |

## 建议优先关注

- `case02`、`case07`：内层循环结束后继续执行外层逻辑，检查循环状态恢复。
- `case04`：循环体零次执行，检查预分配/条件路径是否错误写回或破坏初值。
- `case06`：循环条件带赋值，检查条件块、循环体与下一轮条件之间的 dirty 状态。
- `case08`：多个分支汇合后继续修改变量，并在循环结束后再次分支。
- `case09`：前一循环的结果被后一循环继续读写，检查跨循环的值是否一致。
- `case05`、`case10`：变量数量较多，重点检查 spill/reload 与寄存器状态的一致性。
