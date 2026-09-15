# VSC - Very Simple Compiler

After I studied the book *Engineering a Compiler 3E*, I found the compiler theory is much simpler than what I supposed, (**if you do not want to build a real-world compiler**).
I started this personal experimental "toy" compiler project to practice end‑to‑end compiler implementation.

before I start coding I thought it will be very simple, when I am coding I found it is not that simple even it is a toy compiler.
I used AI for auxiliary work: debugging code snippets, implementing boring utility routines (e.g. token reading, AST dump helpers), and looking up compiler‑related knowledge.

I named the language **vsl (very simple language)**, so the compiler's name is **vsc**.
Project original goal: compile VSL source to x86‑64 / RISC‑V assembly (or binary), which can execute on real CPU hardware.

### process

2026.9.15 

Full VSL‑to‑x86‑64 compiler chain except assembler, outputs x86‑64 assembly; uses GCC for assembling/linking;  the finally executable binary can run on a real x86‑64 CPU.
The implement of all module is simple and straightforward, but not too simple, reg-alloc is -O0 style, load/spill every use.
Didn't implement block, jump, SSA, optimize, and many thing.
Plan to go on: SSA, -O1 reg-alloc, maybe a risc-v backend, also keep everything is simple and self‑designed.


### Project Goal
- lexical analysis
- parsing
- abstract syntax tree construction
- scope management
- semantic analysis
- intermediate representation
- instruction selection
- instruction scheduling
- register allocation
- assembly generation

### Supported Keywords

```
=+-*/(){};
int, return
```

### Language Restrictions
- only one function: `main` with no parameters
- no function calls
- no ABI handling
- only integer operations
- identifiers follow the same naming rules as in the C language




## Compiler Components

### Lexer

### Parser

### DAG Construction

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

### Instruction Select

### Instruction Schedule

Schedule priority:
- critical_path_length


### Register Allocation

-O0:
- correctness
- simple allocation
- load/store based strategy

-O1:
- ...


### Optimization Level

-O0:
- instruction selection
- basic scheduling
- simple register allocation
- assembly generation

-O1:
- constant folding
- dead code elimination
- SSA
- better register allocation
- peephole optimization


## Project Status
- [x] Lexer
- [x] two-stack Parser
- [ ] Pratt Parser
- [x] AST
- [x] namespace Scope manage
- [ ] Control Flow Scope manage
- [x] Symbol table
- [ ] SSA
- [ ] SSA based optimize
- [x] simple Semantic analysis
- [x] Three-address IR
- [x] x64 Instruction selection
- [x] x64 Instruction scheduler
- [x] x64 -O0 Register allocation
- [ ] x64 -O1 Register allocation
- [x] x64 asm


### Example

```c
$make
$./build/a
```

Input:
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


Generated assembly: a.s

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

An executable file named a will be produced.
It runs on Linux‑x86‑64 and prints the return value of the program.

---







