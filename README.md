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


### 2026.9.24

**if/else and block scope are now working.**

VSC can now handle:

- nested `if/else`
- nested block scopes
- variable shadowing
- control-flow basic blocks
- conditional branches and jumps

I tested programs containing deeply nested scopes and multiple `if/else` branches, and multiple `return`, the generated executables produced the expected results.

The current implementation is still intentionally simple and self-designed.

Next steps: SSA, -O1 register allocation.


## Project Status
- [x] Lexer
- [x] two-stack Parser
- [ ] Pratt Parser
- [x] AST
- [x] namespace Scope manage
- [x] Control flow Scope manage / Basic Block
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


- `=` `+` `-` `*` `/` `(` `)` `{` `}` `;` `<` `<=` `>` `>=` `==` `!=`
- `int` `return` `if` `else`


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


#### Scope Management & Lexical Scoping

**VSC** implements a hierarchical lexical scoping mechanism to support nested code blocks (`{ ... }`), control-flow branching (`if`/`else`), and correct variable name resolution.

* **Scope Tree Architecture**: Each `Scope` maintains a parent pointer and child references, forming a hierarchical scope tree. Variables are declared within the symbol table of their enclosing scope.
* **Lexical Resolution & Variable Shadowing**: Name lookup starts from the innermost active scope and walks outward through parent scopes. An identifier declared in an inner scope shadows an identifier with the same name in an outer scope.
* **Control-Flow Scoping & Basic Blocks**: `if`/`else` constructs create dedicated scopes for conditions, branches, and control-flow join points. These scopes are later used to organize basic blocks, jump targets, and control-flow edges.
* **Name Disambiguation**: During semantic analysis, variables are assigned unique internal names (for example, `global_a` and `block2_a`) so that shadowed identifiers can be distinguished in the flat intermediate representation.


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
int main()
{
    int a = 8;
    int b = 3;
    int c = 0;

    if (a > b)
    {
        int b = 10;
        c = a + b;

        if (c > 15)
        {
            int a = 2;
            c = c + a;
        }
        else
        {
            c = c - 1;
        }

        b = c - 5;
    }
    else
    {
        c = a - b;
    }

    {
        int a = 4;
        int c = 7;

        if (a < c)
        {
            int b = 2;
            c = c + b;

            if (c >= a)
                a = a + 3;
        }

        b = b + c;
    }

    if (c > a)
    {
        int c = 2;
        c = c + b;

        if (c < a)
            b = b + c;
        else
            b = b - c;

        if (b == 8)
            a = a + 10;
    }
    else
    {
        c = c + 100;
    }


    return a + b + c;
}

```

the **vsc** reads the test-file "**t1**" in current location and generate a assembly-file: **a.s**

```


#========== asm ==========#
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
	sub rsp, 192 

	
b0: 
	mov r10d, 0 
	mov dword ptr [rbp - 20], r10d 
	mov r10d, 3 
	mov dword ptr [rbp - 12], r10d 
	mov r10d, 8 
	mov dword ptr [rbp - 4], r10d 
	mov r11d, dword ptr [rbp - 4] 
	mov r10d, r11d 
	mov dword ptr [rbp - 8], r10d 
	mov r11d, dword ptr [rbp - 12] 
	mov r10d, r11d 
	mov dword ptr [rbp - 16], r10d 
	mov r11d, dword ptr [rbp - 20] 
	mov r10d, r11d 
	mov dword ptr [rbp - 24], r10d 
	
.L_if2: 
	mov r11d, dword ptr [rbp - 8] 
	mov r12d, dword ptr [rbp - 16] 
	cmp r11d, r12d 
	setle r10b 
	movzx r10d, r10b
	mov dword ptr [rbp - 28], r10d 
	jle .L_else11

	mov r10d, 10 
	mov dword ptr [rbp - 32], r10d 
	mov r11d, dword ptr [rbp - 8] 
	mov r10d, r11d 
	mov dword ptr [rbp - 40], r10d 
	mov r11d, dword ptr [rbp - 32] 
	mov r10d, r11d 
	mov dword ptr [rbp - 36], r10d 
	mov r10d, dword ptr [rbp - 40] 
	mov r11d, dword ptr [rbp - 36] 
	add r10d, r11d 
	mov dword ptr [rbp - 40], r10d 
	mov r11d, dword ptr [rbp - 40] 
	mov r10d, r11d 
	mov dword ptr [rbp - 24], r10d 
	
.L_if4: 
	mov r10d, 15 
	mov dword ptr [rbp - 44], r10d 
	mov r11d, dword ptr [rbp - 24] 
	mov r12d, dword ptr [rbp - 44] 
	cmp r11d, r12d 
	setle r10b 
	movzx r10d, r10b
	mov dword ptr [rbp - 48], r10d 
	jle .L_else7

	mov r10d, 2 
	mov dword ptr [rbp - 52], r10d 
	mov r11d, dword ptr [rbp - 24] 
	mov r10d, r11d 
	mov dword ptr [rbp - 60], r10d 
	mov r11d, dword ptr [rbp - 52] 
	mov r10d, r11d 
	mov dword ptr [rbp - 56], r10d 
	mov r10d, dword ptr [rbp - 60] 
	mov r11d, dword ptr [rbp - 56] 
	add r10d, r11d 
	mov dword ptr [rbp - 60], r10d 
	mov r11d, dword ptr [rbp - 60] 
	mov r10d, r11d 
	mov dword ptr [rbp - 24], r10d 
	jmp .L_9

	
.L_else7: 
	mov r11d, dword ptr [rbp - 24] 
	mov r10d, r11d 
	mov dword ptr [rbp - 68], r10d 
	mov r10d, 1 
	mov dword ptr [rbp - 64], r10d 
	mov r10d, dword ptr [rbp - 68] 
	mov r11d, dword ptr [rbp - 64] 
	sub r10d, r11d 
	mov dword ptr [rbp - 68], r10d 
	mov r11d, dword ptr [rbp - 68] 
	mov r10d, r11d 
	mov dword ptr [rbp - 24], r10d 
	
.L_9: 
	mov r11d, dword ptr [rbp - 24] 
	mov r10d, r11d 
	mov dword ptr [rbp - 76], r10d 
	mov r10d, 5 
	mov dword ptr [rbp - 72], r10d 
	mov r10d, dword ptr [rbp - 76] 
	mov r11d, dword ptr [rbp - 72] 
	sub r10d, r11d 
	mov dword ptr [rbp - 76], r10d 
	mov r11d, dword ptr [rbp - 76] 
	mov r10d, r11d 
	mov dword ptr [rbp - 36], r10d 
	jmp if_jmp_tail13

	
.L_else11: 
	mov r11d, dword ptr [rbp - 8] 
	mov r10d, r11d 
	mov dword ptr [rbp - 80], r10d 
	mov r10d, dword ptr [rbp - 80] 
	mov r11d, dword ptr [rbp - 16] 
	sub r10d, r11d 
	mov dword ptr [rbp - 80], r10d 
	mov r11d, dword ptr [rbp - 80] 
	mov r10d, r11d 
	mov dword ptr [rbp - 24], r10d 
	
if_jmp_tail13: 
	mov r10d, 7 
	mov dword ptr [rbp - 92], r10d 
	mov r10d, 4 
	mov dword ptr [rbp - 84], r10d 
	mov r11d, dword ptr [rbp - 84] 
	mov r10d, r11d 
	mov dword ptr [rbp - 88], r10d 
	mov r11d, dword ptr [rbp - 92] 
	mov r10d, r11d 
	mov dword ptr [rbp - 96], r10d 
	
.L_if15: 
	mov r11d, dword ptr [rbp - 88] 
	mov r12d, dword ptr [rbp - 96] 
	cmp r11d, r12d 
	setge r10b 
	movzx r10d, r10b
	mov dword ptr [rbp - 100], r10d 
	jge .L_21

	mov r10d, 2 
	mov dword ptr [rbp - 104], r10d 
	mov r11d, dword ptr [rbp - 96] 
	mov r10d, r11d 
	mov dword ptr [rbp - 112], r10d 
	mov r11d, dword ptr [rbp - 104] 
	mov r10d, r11d 
	mov dword ptr [rbp - 108], r10d 
	mov r10d, dword ptr [rbp - 112] 
	mov r11d, dword ptr [rbp - 108] 
	add r10d, r11d 
	mov dword ptr [rbp - 112], r10d 
	mov r11d, dword ptr [rbp - 112] 
	mov r10d, r11d 
	mov dword ptr [rbp - 96], r10d 
	
.L_if17: 
	mov r11d, dword ptr [rbp - 96] 
	mov r12d, dword ptr [rbp - 88] 
	cmp r11d, r12d 
	setl r10b 
	movzx r10d, r10b
	mov dword ptr [rbp - 116], r10d 
	jl scp_jmp_lable_reuse20

	mov r11d, dword ptr [rbp - 88] 
	mov r10d, r11d 
	mov dword ptr [rbp - 124], r10d 
	mov r10d, 3 
	mov dword ptr [rbp - 120], r10d 
	mov r10d, dword ptr [rbp - 124] 
	mov r11d, dword ptr [rbp - 120] 
	add r10d, r11d 
	mov dword ptr [rbp - 124], r10d 
	mov r11d, dword ptr [rbp - 124] 
	mov r10d, r11d 
	mov dword ptr [rbp - 88], r10d 
	jmp scp_jmp_lable_reuse20

	
scp_jmp_lable_reuse20: 
	jmp .L_21

	
.L_21: 
	mov r11d, dword ptr [rbp - 16] 
	mov r10d, r11d 
	mov dword ptr [rbp - 128], r10d 
	mov r10d, dword ptr [rbp - 128] 
	mov r11d, dword ptr [rbp - 96] 
	add r10d, r11d 
	mov dword ptr [rbp - 128], r10d 
	mov r11d, dword ptr [rbp - 128] 
	mov r10d, r11d 
	mov dword ptr [rbp - 16], r10d 
	
.L_if23: 
	mov r11d, dword ptr [rbp - 24] 
	mov r12d, dword ptr [rbp - 8] 
	cmp r11d, r12d 
	setle r10b 
	movzx r10d, r10b
	mov dword ptr [rbp - 132], r10d 
	jle .L_else35

	mov r10d, 2 
	mov dword ptr [rbp - 136], r10d 
	mov r11d, dword ptr [rbp - 136] 
	mov r10d, r11d 
	mov dword ptr [rbp - 140], r10d 
	mov r11d, dword ptr [rbp - 140] 
	mov r10d, r11d 
	mov dword ptr [rbp - 144], r10d 
	mov r10d, dword ptr [rbp - 144] 
	mov r11d, dword ptr [rbp - 16] 
	add r10d, r11d 
	mov dword ptr [rbp - 144], r10d 
	mov r11d, dword ptr [rbp - 144] 
	mov r10d, r11d 
	mov dword ptr [rbp - 140], r10d 
	
.L_if25: 
	mov r11d, dword ptr [rbp - 140] 
	mov r12d, dword ptr [rbp - 8] 
	cmp r11d, r12d 
	setge r10b 
	movzx r10d, r10b
	mov dword ptr [rbp - 148], r10d 
	jge .L_else28

	mov r11d, dword ptr [rbp - 16] 
	mov r10d, r11d 
	mov dword ptr [rbp - 152], r10d 
	mov r10d, dword ptr [rbp - 152] 
	mov r11d, dword ptr [rbp - 140] 
	add r10d, r11d 
	mov dword ptr [rbp - 152], r10d 
	mov r11d, dword ptr [rbp - 152] 
	mov r10d, r11d 
	mov dword ptr [rbp - 16], r10d 
	jmp if_jmp_tail30

	
.L_else28: 
	mov r11d, dword ptr [rbp - 16] 
	mov r10d, r11d 
	mov dword ptr [rbp - 156], r10d 
	mov r10d, dword ptr [rbp - 156] 
	mov r11d, dword ptr [rbp - 140] 
	sub r10d, r11d 
	mov dword ptr [rbp - 156], r10d 
	mov r11d, dword ptr [rbp - 156] 
	mov r10d, r11d 
	mov dword ptr [rbp - 16], r10d 
	
if_jmp_tail30: 
	
.L_if31: 
	mov r10d, 8 
	mov dword ptr [rbp - 160], r10d 
	mov r11d, dword ptr [rbp - 16] 
	mov r12d, dword ptr [rbp - 160] 
	cmp r11d, r12d 
	setne r10b 
	movzx r10d, r10b
	mov dword ptr [rbp - 164], r10d 
	jne scp_jmp_lable_reuse34

	mov r11d, dword ptr [rbp - 8] 
	mov r10d, r11d 
	mov dword ptr [rbp - 172], r10d 
	mov r10d, 10 
	mov dword ptr [rbp - 168], r10d 
	mov r10d, dword ptr [rbp - 172] 
	mov r11d, dword ptr [rbp - 168] 
	add r10d, r11d 
	mov dword ptr [rbp - 172], r10d 
	mov r11d, dword ptr [rbp - 172] 
	mov r10d, r11d 
	mov dword ptr [rbp - 8], r10d 
	jmp scp_jmp_lable_reuse34

	
scp_jmp_lable_reuse34: 
	jmp .L_37

	
.L_else35: 
	mov r11d, dword ptr [rbp - 24] 
	mov r10d, r11d 
	mov dword ptr [rbp - 180], r10d 
	mov r10d, 100 
	mov dword ptr [rbp - 176], r10d 
	mov r10d, dword ptr [rbp - 180] 
	mov r11d, dword ptr [rbp - 176] 
	add r10d, r11d 
	mov dword ptr [rbp - 180], r10d 
	mov r11d, dword ptr [rbp - 180] 
	mov r10d, r11d 
	mov dword ptr [rbp - 24], r10d 
	
.L_37: 
	mov r11d, dword ptr [rbp - 8] 
	mov r10d, r11d 
	mov dword ptr [rbp - 184], r10d 
	mov r10d, dword ptr [rbp - 184] 
	mov r11d, dword ptr [rbp - 16] 
	add r10d, r11d 
	mov dword ptr [rbp - 184], r10d 
	mov r11d, dword ptr [rbp - 184] 
	mov r10d, r11d 
	mov dword ptr [rbp - 188], r10d 
	mov r10d, dword ptr [rbp - 188] 
	mov r11d, dword ptr [rbp - 24] 
	add r10d, r11d 
	mov dword ptr [rbp - 188], r10d 
	mov r10d, dword ptr [rbp - 188] 
	# ---------------- print return value ----------------
	mov esi, r10d		# 第 2 个参数：要打印的整数
	lea rdi, [rip + fmt]		# 第 1 个参数：格式化字符串地址
	mov eax, 0		# x86-64 ABI 规定：变长参数调用前将 eax 清零
	call printf@PLT 
	# ------------------------------------------
	mov eax, r10d 
	jmp .L_return

	
.L_return: 
	mov rsp, rbp 
	pop rbp 
	ret 

.section .note.GNU-stack,"",@progbits 


```

An executable file named "**a**" will be produced in current location.
It can run on Linux‑x86‑64 and print the return int-value of the program.

---

