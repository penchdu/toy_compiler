# VSL 随机差分测试工具

这个脚本为 VSL 生成小型随机程序，把同一份 `int test()` 程序分别交给宿主机 `g++` 和你的 VSC 编译器执行，再比较结果。

生成的 VSL 输入只使用当前测试目标需要的保守子集：`int`、赋值、二元 `+` / `-`、除以正整数常量、比较、`if/else` 和有界 `while`。**不会生成一元正负号（如 `-5`、`+3`），不会生成 `continue`，也不会使用逗号运算符。** 初始变量值使用非负整数；变量的值仍然可能通过二元减法变成负数。 每个循环都有独立计数器，并且循环计数器不会被随机的数据赋值修改，避免随机测试自己陷入死循环。

## 放到项目里运行

把 `vsl_diff_fuzz.py` 放到 VSC 仓库根目录，确保已经执行过 `make` 并存在 `./build/vsc`，然后：

```bash
python3 vsl_diff_fuzz.py --cases 100 --seed 1
```

建议先跑 100 个确认工作流正常，再增加到 1000 或更多：

```bash
python3 vsl_diff_fuzz.py --cases 1000 --seed 1 --timeout 3
```

参数说明：

- `--cases 1000`：生成多少个程序。
- `--seed 1`：随机种子。相同参数会生成相同顺序、相同内容的程序，便于复现。
- `--vsc ./build/vsc`：VSC 可执行文件位置。
- `--timeout 3`：单条编译或运行命令的超时秒数。
- `--out fuzz_failures`：保存失败现场的目录。
- `--vars-min 8 --vars-max 14`：每个程序的数据变量数量，会制造不同程度的寄存器压力。
- `--depth 3 --budget 32`：随机嵌套深度和每个程序的随机语句预算。

也可以只生成源码、不运行编译器：

```bash
python3 vsl_diff_fuzz.py --generate-only --cases 20 --seed 1 --out generated_cases
```

## 如何判断结果

每个 case 的流程是：

1. 生成 `case.cpp`，源码只包含 `int test()`，供 VSC 使用。
2. 给同一份函数加一个普通 C++ `main()`，生成 `reference.cpp`，用 `g++ -O0` 作为语义参考。
3. 运行 `./build/vsc case.cpp`，由 VSC 生成 `a1.s`。
4. 运行 `gcc a1.s -o vsl_exec` 并执行。
5. 比较两边打印的 `Result: <整数>`。

全部通过时会显示 `No differences or compile failures found.`。若失败，脚本保留该 case 的 `case.cpp`、`reference.cpp`、生成的 `a1.s`（若存在）、各阶段日志和 `FAILURE.txt`，路径默认在 `fuzz_failures/`。

失败类型会区分：

- `vsc_failed`：VSC 编译失败或没生成 `a1.s`。
- `assemble_failed`：GCC 无法汇编/链接 `a1.s`。
- `runtime_mismatch`：双方执行结果不一致或 VSL 可执行程序异常退出。
- `reference_failed`：随机生成的程序连宿主机 `g++` 都无法编译/执行，优先检查生成器。
- `timeout`：某条编译或运行命令超时。

## 注意

- 这是差分测试，不是正确性证明；测试通过表示当前这些随机样本没有发现差异。
- `a1.s` 是当前脚本选用的寄存器分配输出。如果要同时检查未优化的 `a0.s`，可为它增加第二条编译/执行路径。
- 如果以后 VSL 的语法支持范围变了，应相应扩展生成器，而不是把不支持的语法错误当成分配器 bug。
