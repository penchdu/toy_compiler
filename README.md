# VSC - Very Simple Compiler

After I studied the book *Engineering a Compiler 3E*, I found the compiler theory is much simpler than what I supposed, so I decide to write a toy compiler to kill some time and have fun.

Before I start coding I thought it will be very simple, when I am coding I found it is not that simple even it is a toy compiler.

I used AI for auxiliary work: debug, write boring code(read a word from file, dump AST as graph, makefile), and looking up related knowledge.

I named the language **vsl** (very simple language), so the compiler's name is **vsc**.

Project original goal: compile vsl source to x86‑64 or RISC‑V assembly, uses gcc for assembling/linking; run the final executable on CPU.

## Process

### 2026.9.15

The original goal is reached.

Full vsl‑to‑x86‑64 compiler chain except assembler & linker, outputs x86‑64 assembly; gcc assembler & link; the final binary can run on Linux-x86‑64.

Namespace-scope works well. **vsc** does not support function call but **Gemini** wrote a piece of assembly code to call **printf**, I put it in my assembly-dump to print the return value, it works, that is cool! 

I ran a few tests, the vsc-generated and gcc-generated executables produced identical output.

The implement of all module is straightforward and simple, reg-alloc is -O0 style, load/spill every virtual-register use.

Did not implement block, control flow, jump, SSA, optimize, and many other thing.

Plan to go on: SSA, -O1 reg-alloc, maybe a riscv backend, also keep everything simple and self‑designed.


## Project Status
- [x] Lexer
- [x] two-stack Parser
- [ ] Pratt Parser
- [x] AST
- [x] namespace Scope manage
- [ ] Control Flow Scope manage
- [x] Symbol table
- [ ] Liveness analysis
- [ ] SSA
- [ ] base optimize
- [x] basic Semantic analysis
- [x] Three-address IR
- [x] x64 Inst-select
- [x] x64 Inst-schedule
- [x] x64 -O0 reg-alloc
- [ ] x64 -O1 reg-alloc
- [x] x64 asm


## Supported Keywords


- `=` `+` `-` `*` `/` `(` `)` `{` `}` `;`
- `int` `return`


## Language Restrictions
- only one function: `main` with no parameters
- no function calls
- no ABI handling
- type only 'int'
- identifiers follow the same naming rules as in the C language



## Compiler components detail

### Lexer
...

### Parser
...

### DAG Construction
...

### Scope and Semantic Analysis

Semantic analysis performs:

- variable declaration checking
- symbol table construction
- scope resolution
- variable renaming

Example:

Source:

```c
int a;

{
    int a;
}
```

Internal names:

```
global_a
block2_a
```


---


### Intermediate Representation
...

### Instruction Select
...

### Instruction Schedule

Schedule priority:
- critical_path_length

### Register Allocation

-O0:
- simple allocation, load/store based strategy

-O1:
- ...


### Optimization Goal

-O0:
- no optimize


-O1:
- constant fold
- dead code eliminate


## Example

```c
$make
$./build/a
```

Input (the file "t1"):
```c
int a = 0;
int main()
{
    int a = 1;
    int b;

    {
      int a = 0;
      b = a + 2;
    }

    a = 1;
    b = 2;

    return a + b;
}
```

the **vsc** reads the test-file "**t1**" in current location and generate a assembly-file: **a.s**

```

#========== asm ==========
.intel_syntax noprefix
.extern printf 
.section .rodata 
fmt: 
	.string "Result: %d\n" 

.section .text
.global main

main: 
	push rbp
	mov rbp, rsp
	sub rsp, 48

	mov r10d, 0
	mov dword ptr [rbp - 20], r10d
	mov r10d, 2
	mov dword ptr [rbp - 28], r10d
	mov r10d, 1
	mov dword ptr [rbp - 12], r10d
	mov r10d, 1
	mov dword ptr [rbp - 40], r10d
	mov r10d, 2
	mov dword ptr [rbp - 44], r10d
	mov r10d, 0
	mov dword ptr [rbp - 4], r10d
	mov r11d, dword ptr [rbp - 20]
	mov r10d, r11d
	mov dword ptr [rbp - 24], r10d
	mov r11d, dword ptr [rbp - 12]
	mov r10d, r11d
	mov dword ptr [rbp - 16], r10d
	mov r11d, dword ptr [rbp - 4]
	mov r10d, r11d
	mov dword ptr [rbp - 8], r10d
	mov r11d, dword ptr [rbp - 24]
	mov r10d, r11d
	mov dword ptr [rbp - 32], r10d
	mov r11d, dword ptr [rbp - 40]
	mov r10d, r11d
	mov dword ptr [rbp - 16], r10d
	mov r10d, dword ptr [rbp - 32]
	mov r11d, dword ptr [rbp - 28]
	add r10d, r11d
	mov dword ptr [rbp - 32], r10d
	mov r11d, dword ptr [rbp - 16]
	mov r10d, r11d
	mov dword ptr [rbp - 48], r10d
	mov r11d, dword ptr [rbp - 32]
	mov r10d, r11d
	mov dword ptr [rbp - 36], r10d
	mov r11d, dword ptr [rbp - 44]
	mov r10d, r11d
	mov dword ptr [rbp - 36], r10d
	mov r10d, dword ptr [rbp - 48]
	mov r11d, dword ptr [rbp - 36]
	add r10d, r11d
	mov dword ptr [rbp - 48], r10d
	mov r10d, dword ptr [rbp - 48]
	# ---------------- 打印结果 ----------------
	mov esi, r10d		# 第 2 个参数：要打印的整数 (放在 %esi / %rsi)
	lea rdi, [rip + fmt]		# 第 1 个参数：格式化字符串地址 (放在 %rdi)
	mov eax, 0		# x86-64 ABI 规定：变长参数调用前将 eax 清零
	call printf@PLT		# 调用 C 语言的 printf
	# ------------------------------------------
	mov eax, r10d
	mov rsp, rbp
	pop rbp
	ret 

.section .note.GNU-stack,"",@progbits

```

An executable file named "**a**" will be produced in current location.
It can run on Linux‑x86‑64 and print the return int-value of the program.

---

