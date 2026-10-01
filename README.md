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
- nested `while` `continue` `break`
- nested block scopes
- variable shadowing
- control-flow basic blocks
- conditional branches and jumps

I tested programs containing deeply nested scopes and multiple `if/else` branches, and multiple `return`, the generated executables produced the expected results.

Next steps: SSA, -O1 register allocation.


### 2026.9.29

**Liveness Analysis & Wavefront Register Allocation (-O1)**

Implemented liveness analysis and an innovative wavefront-based register allocation algorithm for -O1 optimization:

- Liveness & Usage Analysis: Added block-level variable usage and liveness analysis to track variable reading, writing, and cross-scope references.
- Wavefront Score Model: Introduced an asymmetric decay wave model for virtual registers. It simulates usage peaks and rapid post-use decay, dynamically scoring registers for allocation choices.  The idea is that for a virtual register (VR), if it is used continuously or near-continuously in the near future, it generates a smoothly rising peak to signal to the allocator that this VR should not be spilled. If it remains unused beyond a certain range, it rapidly drops into a trough, indicating that this VR can be spilled. Ultimately, the allocator compares the peak values of all VRs at the current position to decide which VR to spill.
- Wavefront Register Allocator: Implemented dynamic register allocation driven by wavefront scores (`a1.s`), reducing unnecessary memory spill/load instructions compared to the baseline -O0 strategy (`a0.s`).
- Score-Driven Instruction Scheduler : Refined instruction scheduling using critical path latency, functional unit availability (ALU/IMUL/DIV), and scope-level transient variable consumption scores.

Next steps: global-Inst-select and global-reg-alloc

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
- [x] x64 Inst-schedule (BasicBlock-level, use score)
- [ ] x64 Inst-schedule (global, use score)
- [x] x64 -O0 reg-alloc	(spill/reload)
- [x] x64 -O1 reg-alloc (BasicBlock-level Wavefront)
- [ ] x64 -O1 reg-alloc (global Wavefront)
- [x] x64 asm


## Supported Keywords


- `=` `+` `-` `*` `/` `(` `)` `{` `}` `;` `<` `<=` `>` `>=` `==` `!=`
- `int` `return` `if` `else` `while` `continue` `break`


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

- Score‑Driven Instruction Scheduler: Refined scheduling based on critical‑path latency, functional‑unit availability(ALU/IMUL/DIV), and remaining‑usage hints from scope‑level transient‑variable consumption; no explicit register‑pressure modelling.

### Register Allocation

-O0:
- simple allocation, load/store based strategy

-O1:
- BasicBlock-level Wavefront


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

Input (the file "t1.txt"):
```c


int test()
{
	int a = 1;
	int b = a + 10;
	int c = a + 100;
	int d = a + 1000;
	int e = a + 0;
	int f = 0;
	int sum = 0;
	int i = 0;

	while (i < 12)
	{
		if (a < b)
		{
			a = a + 3;
			e = e + 1;

			if (c < d)
			{
				c = c + 5;
				f = f + 1;
			}
			else
			{
				d = d - 2;
				f = f + 2;
			}
		}
		else
		{
			b = b - 2;
			e = e + 2;
			if (c > d)
			{
				c = c - 3;
				f = f + 3;
			}
			else
			{
				d = d + 4;
				f = f + 4;
			}
		}

		sum = sum + a + b + c + d + e + f;
		a = a + e;
		b = b + f;

		i = i + 1;
	}

	sum = sum + 1;
	return sum;
}





```





the **vsc** reads the test-file "**t1.txt**" in current location and generate assembly-file: **a0.s** and **a1.s**





```





#========== a0.s ==========#
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
	sub rsp, 224 
	mov r10d, 0                            #li, idx 14, cyc 0
	mov dword ptr [rbp - 60], r10d         #spill, idx 93, cyc -1
	mov r10d, 0                            #li, idx 12, cyc 0
	mov dword ptr [rbp - 52], r10d         #spill, idx 94, cyc -1
	mov r10d, 0                            #li, idx 10, cyc 0
	mov dword ptr [rbp - 44], r10d         #spill, idx 95, cyc -1
	mov r10d, 0                            #li, idx 8, cyc 0
	mov dword ptr [rbp - 36], r10d         #spill, idx 96, cyc -1
	mov r10d, 1000                         #li, idx 6, cyc 0
	mov dword ptr [rbp - 28], r10d         #spill, idx 97, cyc -1
	mov r10d, 100                          #li, idx 4, cyc 0
	mov dword ptr [rbp - 20], r10d         #spill, idx 98, cyc -1
	mov r10d, 10                           #li, idx 2, cyc 0
	mov dword ptr [rbp - 12], r10d         #spill, idx 99, cyc -1
	mov r10d, 1                            #li, idx 0, cyc 0
	mov dword ptr [rbp - 4], r10d          #spill, idx 100, cyc -1
	mov r11d, dword ptr [rbp - 4]          #alloc, idx 101, cyc -1
	mov r10d, r11d                         #assign, idx 1, cyc 1
	mov dword ptr [rbp - 8], r10d          #spill, idx 102, cyc -1
	mov r11d, dword ptr [rbp - 12]         #alloc, idx 103, cyc -1
	mov r10d, r11d                         #assign, idx 3, cyc 1
	mov dword ptr [rbp - 16], r10d         #spill, idx 104, cyc -1
	mov r11d, dword ptr [rbp - 20]         #alloc, idx 105, cyc -1
	mov r10d, r11d                         #assign, idx 5, cyc 1
	mov dword ptr [rbp - 24], r10d         #spill, idx 106, cyc -1
	mov r11d, dword ptr [rbp - 28]         #alloc, idx 107, cyc -1
	mov r10d, r11d                         #assign, idx 7, cyc 1
	mov dword ptr [rbp - 32], r10d         #spill, idx 108, cyc -1
	mov r11d, dword ptr [rbp - 36]         #alloc, idx 109, cyc -1
	mov r10d, r11d                         #assign, idx 9, cyc 1
	mov dword ptr [rbp - 40], r10d         #spill, idx 110, cyc -1
	mov r11d, dword ptr [rbp - 44]         #alloc, idx 111, cyc -1
	mov r10d, r11d                         #assign, idx 11, cyc 1
	mov dword ptr [rbp - 48], r10d         #spill, idx 112, cyc -1
	mov r11d, dword ptr [rbp - 52]         #alloc, idx 113, cyc -1
	mov r10d, r11d                         #assign, idx 13, cyc 1
	mov dword ptr [rbp - 56], r10d         #spill, idx 114, cyc -1
	mov r11d, dword ptr [rbp - 60]         #alloc, idx 115, cyc -1
	mov r10d, r11d                         #assign, idx 15, cyc 1
	mov dword ptr [rbp - 64], r10d         #spill, idx 116, cyc -1


.L_b3_while_cond:
	mov r10d, 12                           #li, idx 16, cyc 0
	mov dword ptr [rbp - 68], r10d         #spill, idx 117, cyc -1
	mov r11d, dword ptr [rbp - 64]         #alloc, idx 118, cyc -1
	mov r12d, dword ptr [rbp - 68]         #alloc, idx 119, cyc -1
	cmp r11d, r12d                         #cmpge, idx 17, cyc 1
	setge r10b                             #cmpge, idx 17, cyc 1
	movzx r10d, r10b                       #cmpge, idx 17, cyc 1
	mov dword ptr [rbp - 72], r10d         #spill, idx 120, cyc -1
	jge .L_b32_jmp_tail_of_while 


.L_b4_while_body:


.L_b6_if_cond:
	mov r11d, dword ptr [rbp - 8]          #alloc, idx 121, cyc -1
	mov r12d, dword ptr [rbp - 16]         #alloc, idx 122, cyc -1
	cmp r11d, r12d                         #cmpge, idx 18, cyc 0
	setge r10b                             #cmpge, idx 18, cyc 0
	movzx r10d, r10b                       #cmpge, idx 18, cyc 0
	mov dword ptr [rbp - 76], r10d         #spill, idx 123, cyc -1
	jge .L_b17_if_else 
	mov r11d, dword ptr [rbp - 40]         #alloc, idx 124, cyc -1
	mov r10d, r11d                         #add, idx 24, cyc 0
	mov dword ptr [rbp - 92], r10d         #spill, idx 125, cyc -1
	mov r10d, 1                            #li, idx 23, cyc 0
	mov dword ptr [rbp - 88], r10d         #spill, idx 126, cyc -1
	mov r11d, dword ptr [rbp - 8]          #alloc, idx 127, cyc -1
	mov r10d, r11d                         #add, idx 20, cyc 0
	mov dword ptr [rbp - 84], r10d         #spill, idx 128, cyc -1
	mov r10d, 3                            #li, idx 19, cyc 0
	mov dword ptr [rbp - 80], r10d         #spill, idx 129, cyc -1
	mov r10d, dword ptr [rbp - 84]         #alloc, idx 130, cyc -1
	mov r11d, dword ptr [rbp - 80]         #alloc, idx 131, cyc -1
	add r10d, r11d                         #add, idx 21, cyc 1
	mov dword ptr [rbp - 84], r10d         #spill, idx 132, cyc -1
	mov r10d, dword ptr [rbp - 92]         #alloc, idx 133, cyc -1
	mov r11d, dword ptr [rbp - 88]         #alloc, idx 134, cyc -1
	add r10d, r11d                         #add, idx 25, cyc 2
	mov dword ptr [rbp - 92], r10d         #spill, idx 135, cyc -1
	mov r11d, dword ptr [rbp - 84]         #alloc, idx 136, cyc -1
	mov r10d, r11d                         #assign, idx 22, cyc 2
	mov dword ptr [rbp - 8], r10d          #spill, idx 137, cyc -1
	mov r11d, dword ptr [rbp - 92]         #alloc, idx 138, cyc -1
	mov r10d, r11d                         #assign, idx 26, cyc 3
	mov dword ptr [rbp - 40], r10d         #spill, idx 139, cyc -1


.L_b9_if_cond:
	mov r11d, dword ptr [rbp - 24]         #alloc, idx 140, cyc -1
	mov r12d, dword ptr [rbp - 32]         #alloc, idx 141, cyc -1
	cmp r11d, r12d                         #cmpge, idx 27, cyc 0
	setge r10b                             #cmpge, idx 27, cyc 0
	movzx r10d, r10b                       #cmpge, idx 27, cyc 0
	mov dword ptr [rbp - 96], r10d         #spill, idx 142, cyc -1
	jge .L_b12_if_else 
	mov r11d, dword ptr [rbp - 48]         #alloc, idx 143, cyc -1
	mov r10d, r11d                         #add, idx 33, cyc 0
	mov dword ptr [rbp - 112], r10d        #spill, idx 144, cyc -1
	mov r10d, 1                            #li, idx 32, cyc 0
	mov dword ptr [rbp - 108], r10d        #spill, idx 145, cyc -1
	mov r11d, dword ptr [rbp - 24]         #alloc, idx 146, cyc -1
	mov r10d, r11d                         #add, idx 29, cyc 0
	mov dword ptr [rbp - 104], r10d        #spill, idx 147, cyc -1
	mov r10d, 5                            #li, idx 28, cyc 0
	mov dword ptr [rbp - 100], r10d        #spill, idx 148, cyc -1
	mov r10d, dword ptr [rbp - 104]        #alloc, idx 149, cyc -1
	mov r11d, dword ptr [rbp - 100]        #alloc, idx 150, cyc -1
	add r10d, r11d                         #add, idx 30, cyc 1
	mov dword ptr [rbp - 104], r10d        #spill, idx 151, cyc -1
	mov r10d, dword ptr [rbp - 112]        #alloc, idx 152, cyc -1
	mov r11d, dword ptr [rbp - 108]        #alloc, idx 153, cyc -1
	add r10d, r11d                         #add, idx 34, cyc 2
	mov dword ptr [rbp - 112], r10d        #spill, idx 154, cyc -1
	mov r11d, dword ptr [rbp - 104]        #alloc, idx 155, cyc -1
	mov r10d, r11d                         #assign, idx 31, cyc 2
	mov dword ptr [rbp - 24], r10d         #spill, idx 156, cyc -1
	mov r11d, dword ptr [rbp - 112]        #alloc, idx 157, cyc -1
	mov r10d, r11d                         #assign, idx 35, cyc 3
	mov dword ptr [rbp - 48], r10d         #spill, idx 158, cyc -1
	jmp .L_b14_jmp_tail_of_if 


.L_b12_if_else:
	mov r11d, dword ptr [rbp - 48]         #alloc, idx 159, cyc -1
	mov r10d, r11d                         #add, idx 41, cyc 0
	mov dword ptr [rbp - 128], r10d        #spill, idx 160, cyc -1
	mov r10d, 2                            #li, idx 40, cyc 0
	mov dword ptr [rbp - 124], r10d        #spill, idx 161, cyc -1
	mov r11d, dword ptr [rbp - 32]         #alloc, idx 162, cyc -1
	mov r10d, r11d                         #sub, idx 37, cyc 0
	mov dword ptr [rbp - 120], r10d        #spill, idx 163, cyc -1
	mov r10d, 2                            #li, idx 36, cyc 0
	mov dword ptr [rbp - 116], r10d        #spill, idx 164, cyc -1
	mov r10d, dword ptr [rbp - 120]        #alloc, idx 165, cyc -1
	mov r11d, dword ptr [rbp - 116]        #alloc, idx 166, cyc -1
	sub r10d, r11d                         #sub, idx 38, cyc 1
	mov dword ptr [rbp - 120], r10d        #spill, idx 167, cyc -1
	mov r10d, dword ptr [rbp - 128]        #alloc, idx 168, cyc -1
	mov r11d, dword ptr [rbp - 124]        #alloc, idx 169, cyc -1
	add r10d, r11d                         #add, idx 42, cyc 2
	mov dword ptr [rbp - 128], r10d        #spill, idx 170, cyc -1
	mov r11d, dword ptr [rbp - 120]        #alloc, idx 171, cyc -1
	mov r10d, r11d                         #assign, idx 39, cyc 2
	mov dword ptr [rbp - 32], r10d         #spill, idx 172, cyc -1
	mov r11d, dword ptr [rbp - 128]        #alloc, idx 173, cyc -1
	mov r10d, r11d                         #assign, idx 43, cyc 3
	mov dword ptr [rbp - 48], r10d         #spill, idx 174, cyc -1


.L_b14_jmp_tail_of_if:
	jmp .L_b28_jmp_tail_of_if 


.L_b17_if_else:
	mov r11d, dword ptr [rbp - 40]         #alloc, idx 175, cyc -1
	mov r10d, r11d                         #add, idx 49, cyc 0
	mov dword ptr [rbp - 144], r10d        #spill, idx 176, cyc -1
	mov r10d, 2                            #li, idx 48, cyc 0
	mov dword ptr [rbp - 140], r10d        #spill, idx 177, cyc -1
	mov r11d, dword ptr [rbp - 16]         #alloc, idx 178, cyc -1
	mov r10d, r11d                         #sub, idx 45, cyc 0
	mov dword ptr [rbp - 136], r10d        #spill, idx 179, cyc -1
	mov r10d, 2                            #li, idx 44, cyc 0
	mov dword ptr [rbp - 132], r10d        #spill, idx 180, cyc -1
	mov r10d, dword ptr [rbp - 136]        #alloc, idx 181, cyc -1
	mov r11d, dword ptr [rbp - 132]        #alloc, idx 182, cyc -1
	sub r10d, r11d                         #sub, idx 46, cyc 1
	mov dword ptr [rbp - 136], r10d        #spill, idx 183, cyc -1
	mov r10d, dword ptr [rbp - 144]        #alloc, idx 184, cyc -1
	mov r11d, dword ptr [rbp - 140]        #alloc, idx 185, cyc -1
	add r10d, r11d                         #add, idx 50, cyc 2
	mov dword ptr [rbp - 144], r10d        #spill, idx 186, cyc -1
	mov r11d, dword ptr [rbp - 136]        #alloc, idx 187, cyc -1
	mov r10d, r11d                         #assign, idx 47, cyc 2
	mov dword ptr [rbp - 16], r10d         #spill, idx 188, cyc -1
	mov r11d, dword ptr [rbp - 144]        #alloc, idx 189, cyc -1
	mov r10d, r11d                         #assign, idx 51, cyc 3
	mov dword ptr [rbp - 40], r10d         #spill, idx 190, cyc -1


.L_b19_if_cond:
	mov r11d, dword ptr [rbp - 24]         #alloc, idx 191, cyc -1
	mov r12d, dword ptr [rbp - 32]         #alloc, idx 192, cyc -1
	cmp r11d, r12d                         #cmple, idx 52, cyc 0
	setle r10b                             #cmple, idx 52, cyc 0
	movzx r10d, r10b                       #cmple, idx 52, cyc 0
	mov dword ptr [rbp - 148], r10d        #spill, idx 193, cyc -1
	jle .L_b22_if_else 
	mov r11d, dword ptr [rbp - 48]         #alloc, idx 194, cyc -1
	mov r10d, r11d                         #add, idx 58, cyc 0
	mov dword ptr [rbp - 164], r10d        #spill, idx 195, cyc -1
	mov r10d, 3                            #li, idx 57, cyc 0
	mov dword ptr [rbp - 160], r10d        #spill, idx 196, cyc -1
	mov r11d, dword ptr [rbp - 24]         #alloc, idx 197, cyc -1
	mov r10d, r11d                         #sub, idx 54, cyc 0
	mov dword ptr [rbp - 156], r10d        #spill, idx 198, cyc -1
	mov r10d, 3                            #li, idx 53, cyc 0
	mov dword ptr [rbp - 152], r10d        #spill, idx 199, cyc -1
	mov r10d, dword ptr [rbp - 156]        #alloc, idx 200, cyc -1
	mov r11d, dword ptr [rbp - 152]        #alloc, idx 201, cyc -1
	sub r10d, r11d                         #sub, idx 55, cyc 1
	mov dword ptr [rbp - 156], r10d        #spill, idx 202, cyc -1
	mov r10d, dword ptr [rbp - 164]        #alloc, idx 203, cyc -1
	mov r11d, dword ptr [rbp - 160]        #alloc, idx 204, cyc -1
	add r10d, r11d                         #add, idx 59, cyc 2
	mov dword ptr [rbp - 164], r10d        #spill, idx 205, cyc -1
	mov r11d, dword ptr [rbp - 156]        #alloc, idx 206, cyc -1
	mov r10d, r11d                         #assign, idx 56, cyc 2
	mov dword ptr [rbp - 24], r10d         #spill, idx 207, cyc -1
	mov r11d, dword ptr [rbp - 164]        #alloc, idx 208, cyc -1
	mov r10d, r11d                         #assign, idx 60, cyc 3
	mov dword ptr [rbp - 48], r10d         #spill, idx 209, cyc -1
	jmp .L_b24_jmp_tail_of_if 


.L_b22_if_else:
	mov r11d, dword ptr [rbp - 48]         #alloc, idx 210, cyc -1
	mov r10d, r11d                         #add, idx 66, cyc 0
	mov dword ptr [rbp - 180], r10d        #spill, idx 211, cyc -1
	mov r10d, 4                            #li, idx 65, cyc 0
	mov dword ptr [rbp - 176], r10d        #spill, idx 212, cyc -1
	mov r11d, dword ptr [rbp - 32]         #alloc, idx 213, cyc -1
	mov r10d, r11d                         #add, idx 62, cyc 0
	mov dword ptr [rbp - 172], r10d        #spill, idx 214, cyc -1
	mov r10d, 4                            #li, idx 61, cyc 0
	mov dword ptr [rbp - 168], r10d        #spill, idx 215, cyc -1
	mov r10d, dword ptr [rbp - 172]        #alloc, idx 216, cyc -1
	mov r11d, dword ptr [rbp - 168]        #alloc, idx 217, cyc -1
	add r10d, r11d                         #add, idx 63, cyc 1
	mov dword ptr [rbp - 172], r10d        #spill, idx 218, cyc -1
	mov r10d, dword ptr [rbp - 180]        #alloc, idx 219, cyc -1
	mov r11d, dword ptr [rbp - 176]        #alloc, idx 220, cyc -1
	add r10d, r11d                         #add, idx 67, cyc 2
	mov dword ptr [rbp - 180], r10d        #spill, idx 221, cyc -1
	mov r11d, dword ptr [rbp - 172]        #alloc, idx 222, cyc -1
	mov r10d, r11d                         #assign, idx 64, cyc 2
	mov dword ptr [rbp - 32], r10d         #spill, idx 223, cyc -1
	mov r11d, dword ptr [rbp - 180]        #alloc, idx 224, cyc -1
	mov r10d, r11d                         #assign, idx 68, cyc 3
	mov dword ptr [rbp - 48], r10d         #spill, idx 225, cyc -1


.L_b24_jmp_tail_of_if:


.L_b28_jmp_tail_of_if:
	mov r11d, dword ptr [rbp - 56]         #alloc, idx 226, cyc -1
	mov r10d, r11d                         #add, idx 69, cyc 0
	mov dword ptr [rbp - 184], r10d        #spill, idx 227, cyc -1
	mov r11d, dword ptr [rbp - 64]         #alloc, idx 228, cyc -1
	mov r10d, r11d                         #add, idx 89, cyc 0
	mov dword ptr [rbp - 220], r10d        #spill, idx 229, cyc -1
	mov r10d, 1                            #li, idx 88, cyc 0
	mov dword ptr [rbp - 216], r10d        #spill, idx 230, cyc -1
	mov r11d, dword ptr [rbp - 16]         #alloc, idx 231, cyc -1
	mov r10d, r11d                         #add, idx 85, cyc 0
	mov dword ptr [rbp - 212], r10d        #spill, idx 232, cyc -1
	mov r11d, dword ptr [rbp - 8]          #alloc, idx 233, cyc -1
	mov r10d, r11d                         #add, idx 82, cyc 0
	mov dword ptr [rbp - 208], r10d        #spill, idx 234, cyc -1
	mov r10d, dword ptr [rbp - 184]        #alloc, idx 235, cyc -1
	mov r11d, dword ptr [rbp - 8]          #alloc, idx 236, cyc -1
	add r10d, r11d                         #add, idx 70, cyc 1
	mov dword ptr [rbp - 184], r10d        #spill, idx 237, cyc -1
	mov r11d, dword ptr [rbp - 184]        #alloc, idx 238, cyc -1
	mov r10d, r11d                         #add, idx 71, cyc 2
	mov dword ptr [rbp - 188], r10d        #spill, idx 239, cyc -1
	mov r10d, dword ptr [rbp - 208]        #alloc, idx 240, cyc -1
	mov r11d, dword ptr [rbp - 40]         #alloc, idx 241, cyc -1
	add r10d, r11d                         #add, idx 83, cyc 2
	mov dword ptr [rbp - 208], r10d        #spill, idx 242, cyc -1
	mov r10d, dword ptr [rbp - 188]        #alloc, idx 243, cyc -1
	mov r11d, dword ptr [rbp - 16]         #alloc, idx 244, cyc -1
	add r10d, r11d                         #add, idx 72, cyc 3
	mov dword ptr [rbp - 188], r10d        #spill, idx 245, cyc -1
	mov r11d, dword ptr [rbp - 208]        #alloc, idx 246, cyc -1
	mov r10d, r11d                         #assign, idx 84, cyc 3
	mov dword ptr [rbp - 8], r10d          #spill, idx 247, cyc -1
	mov r11d, dword ptr [rbp - 188]        #alloc, idx 248, cyc -1
	mov r10d, r11d                         #add, idx 73, cyc 4
	mov dword ptr [rbp - 192], r10d        #spill, idx 249, cyc -1
	mov r10d, dword ptr [rbp - 212]        #alloc, idx 250, cyc -1
	mov r11d, dword ptr [rbp - 48]         #alloc, idx 251, cyc -1
	add r10d, r11d                         #add, idx 86, cyc 4
	mov dword ptr [rbp - 212], r10d        #spill, idx 252, cyc -1
	mov r10d, dword ptr [rbp - 192]        #alloc, idx 253, cyc -1
	mov r11d, dword ptr [rbp - 24]         #alloc, idx 254, cyc -1
	add r10d, r11d                         #add, idx 74, cyc 5
	mov dword ptr [rbp - 192], r10d        #spill, idx 255, cyc -1
	mov r11d, dword ptr [rbp - 212]        #alloc, idx 256, cyc -1
	mov r10d, r11d                         #assign, idx 87, cyc 5
	mov dword ptr [rbp - 16], r10d         #spill, idx 257, cyc -1
	mov r11d, dword ptr [rbp - 192]        #alloc, idx 258, cyc -1
	mov r10d, r11d                         #add, idx 75, cyc 6
	mov dword ptr [rbp - 196], r10d        #spill, idx 259, cyc -1
	mov r10d, dword ptr [rbp - 220]        #alloc, idx 260, cyc -1
	mov r11d, dword ptr [rbp - 216]        #alloc, idx 261, cyc -1
	add r10d, r11d                         #add, idx 90, cyc 6
	mov dword ptr [rbp - 220], r10d        #spill, idx 262, cyc -1
	mov r10d, dword ptr [rbp - 196]        #alloc, idx 263, cyc -1
	mov r11d, dword ptr [rbp - 32]         #alloc, idx 264, cyc -1
	add r10d, r11d                         #add, idx 76, cyc 7
	mov dword ptr [rbp - 196], r10d        #spill, idx 265, cyc -1
	mov r11d, dword ptr [rbp - 220]        #alloc, idx 266, cyc -1
	mov r10d, r11d                         #assign, idx 91, cyc 7
	mov dword ptr [rbp - 64], r10d         #spill, idx 267, cyc -1
	mov r11d, dword ptr [rbp - 196]        #alloc, idx 268, cyc -1
	mov r10d, r11d                         #add, idx 77, cyc 8
	mov dword ptr [rbp - 200], r10d        #spill, idx 269, cyc -1
	mov r10d, dword ptr [rbp - 200]        #alloc, idx 270, cyc -1
	mov r11d, dword ptr [rbp - 40]         #alloc, idx 271, cyc -1
	add r10d, r11d                         #add, idx 78, cyc 9
	mov dword ptr [rbp - 200], r10d        #spill, idx 272, cyc -1
	mov r11d, dword ptr [rbp - 200]        #alloc, idx 273, cyc -1
	mov r10d, r11d                         #add, idx 79, cyc 10
	mov dword ptr [rbp - 204], r10d        #spill, idx 274, cyc -1
	mov r10d, dword ptr [rbp - 204]        #alloc, idx 275, cyc -1
	mov r11d, dword ptr [rbp - 48]         #alloc, idx 276, cyc -1
	add r10d, r11d                         #add, idx 80, cyc 11
	mov dword ptr [rbp - 204], r10d        #spill, idx 277, cyc -1
	mov r11d, dword ptr [rbp - 204]        #alloc, idx 278, cyc -1
	mov r10d, r11d                         #assign, idx 81, cyc 12
	mov dword ptr [rbp - 56], r10d         #spill, idx 279, cyc -1
	jmp .L_b3_while_cond 


.L_b32_jmp_tail_of_while:
	mov r10d, dword ptr [rbp - 56]         #alloc, idx 280, cyc -1
	#---------------- print ret ----------------# 
	mov esi, r10d 
	lea rdi, [rip + fmt] 
	mov eax, 0 
	call printf@PLT 
	#------------------------------------------# 
	mov eax, r10d 
	jmp .L_return 


.L_return:
	mov rsp, rbp 
	pop rbp 
	ret 

.section .note.GNU-stack,"",@progbits



```


```


#========== a1.s ==========#
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
	sub rsp, 256 
	mov r10d, 1                            #li, idx 0, cyc 0
	mov r11d, 10                           #li, idx 2, cyc 0
	mov r12d, r10d                         #assign, idx 1, cyc 1
	mov r10d, 100                          #li, idx 6, cyc 1
	mov r13d, 1000                         #li, idx 10, cyc 2
	mov r14d, 0                            #li, idx 14, cyc 2
	mov r15d, r12d                         #add, idx 3, cyc 3
	mov dword ptr [rbp - 36], r13d         #spill, idx 321, cyc -1
	mov r13d, r12d                         #add, idx 7, cyc 3
	add r15d, r11d                         #add, idx 4, cyc 4
	add r13d, r10d                         #add, idx 8, cyc 4
	mov r10d, r12d                         #add, idx 11, cyc 5
	mov r11d, r12d                         #add, idx 15, cyc 5
	mov dword ptr [rbp - 16], r15d         #spill, idx 322, cyc -1
	mov r15d, dword ptr [rbp - 36]         #alloc, idx 323, cyc -1
	add r10d, r15d                         #add, idx 12, cyc 6
	add r11d, r14d                         #add, idx 16, cyc 6
	mov r14d, 0                            #li, idx 18, cyc 7
	mov r15d, 0                            #li, idx 20, cyc 7
	mov dword ptr [rbp - 40], r10d         #spill, idx 324, cyc -1
	mov r10d, 0                            #li, idx 22, cyc 8
	mov dword ptr [rbp - 8], r12d          #spill, idx 325, cyc -1
	mov dword ptr [rbp - 60], r14d         #spill, idx 326, cyc -1
	mov r14d, dword ptr [rbp - 16]         #alloc, idx 327, cyc -1
	mov r12d, r14d                         #assign, idx 5, cyc 8
	mov r14d, r13d                         #assign, idx 9, cyc 9
	mov dword ptr [rbp - 68], r15d         #spill, idx 328, cyc -1
	mov r15d, dword ptr [rbp - 40]         #alloc, idx 329, cyc -1
	mov r13d, r15d                         #assign, idx 13, cyc 9
	mov r15d, r11d                         #assign, idx 17, cyc 10
	mov dword ptr [rbp - 20], r12d         #spill, idx 330, cyc -1
	mov r12d, dword ptr [rbp - 60]         #alloc, idx 331, cyc -1
	mov r11d, r12d                         #assign, idx 19, cyc 10
	mov dword ptr [rbp - 32], r14d         #spill, idx 332, cyc -1
	mov r14d, dword ptr [rbp - 68]         #alloc, idx 333, cyc -1
	mov r12d, r14d                         #assign, idx 21, cyc 11
	mov r14d, r10d                         #assign, idx 23, cyc 11
	mov dword ptr [rbp - 64], r11d         #spill, idx 334, cyc -1
	mov dword ptr [rbp - 72], r12d         #spill, idx 335, cyc -1
	mov dword ptr [rbp - 44], r13d         #spill, idx 336, cyc -1
	mov dword ptr [rbp - 80], r14d         #spill, idx 337, cyc -1
	mov dword ptr [rbp - 56], r15d         #spill, idx 338, cyc -1


.L_b3_while_cond:
	mov r10d, 12                           #li, idx 24, cyc 0
	mov r12d, dword ptr [rbp - 80]         #alloc, idx 339, cyc -1
	cmp r12d, r10d                         #cmpge, idx 25, cyc 1
	setge r11b                             #cmpge, idx 25, cyc 1
	movzx r11d, r11b                       #cmpge, idx 25, cyc 1
	jge .L_b32_jmp_tail_of_while 


.L_b4_while_body:


.L_b6_if_cond:
	mov r11d, dword ptr [rbp - 8]          #alloc, idx 340, cyc -1
	mov r12d, dword ptr [rbp - 20]         #alloc, idx 341, cyc -1
	cmp r11d, r12d                         #cmpge, idx 26, cyc 0
	setge r10b                             #cmpge, idx 26, cyc 0
	movzx r10d, r10b                       #cmpge, idx 26, cyc 0
	jge .L_b17_if_else 
	mov r10d, 3                            #li, idx 27, cyc 0
	mov r12d, dword ptr [rbp - 8]          #alloc, idx 342, cyc -1
	mov r11d, r12d                         #add, idx 28, cyc 0
	add r11d, r10d                         #add, idx 29, cyc 1
	mov r10d, 1                            #li, idx 31, cyc 1
	mov r12d, r11d                         #assign, idx 30, cyc 2
	mov r13d, dword ptr [rbp - 56]         #alloc, idx 343, cyc -1
	mov r11d, r13d                         #add, idx 32, cyc 2
	add r11d, r10d                         #add, idx 33, cyc 3
	mov r13d, r11d                         #assign, idx 34, cyc 4
	mov dword ptr [rbp - 8], r12d          #spill, idx 344, cyc -1
	mov dword ptr [rbp - 56], r13d         #spill, idx 345, cyc -1


.L_b9_if_cond:
	mov r11d, dword ptr [rbp - 32]         #alloc, idx 346, cyc -1
	mov r12d, dword ptr [rbp - 44]         #alloc, idx 347, cyc -1
	cmp r11d, r12d                         #cmpge, idx 35, cyc 0
	setge r10b                             #cmpge, idx 35, cyc 0
	movzx r10d, r10b                       #cmpge, idx 35, cyc 0
	jge .L_b12_if_else 
	mov r10d, 5                            #li, idx 36, cyc 0
	mov r12d, dword ptr [rbp - 32]         #alloc, idx 348, cyc -1
	mov r11d, r12d                         #add, idx 37, cyc 0
	add r11d, r10d                         #add, idx 38, cyc 1
	mov r10d, 1                            #li, idx 40, cyc 1
	mov r12d, r11d                         #assign, idx 39, cyc 2
	mov r13d, dword ptr [rbp - 64]         #alloc, idx 349, cyc -1
	mov r11d, r13d                         #add, idx 41, cyc 2
	add r11d, r10d                         #add, idx 42, cyc 3
	mov r13d, r11d                         #assign, idx 43, cyc 4
	mov dword ptr [rbp - 32], r12d         #spill, idx 350, cyc -1
	mov dword ptr [rbp - 64], r13d         #spill, idx 351, cyc -1
	jmp .L_b14_jmp_tail_of_if 


.L_b12_if_else:
	mov r10d, 2                            #li, idx 44, cyc 0
	mov r12d, dword ptr [rbp - 44]         #alloc, idx 352, cyc -1
	mov r11d, r12d                         #sub, idx 45, cyc 0
	sub r11d, r10d                         #sub, idx 46, cyc 1
	mov r10d, 2                            #li, idx 48, cyc 1
	mov r12d, r11d                         #assign, idx 47, cyc 2
	mov r13d, dword ptr [rbp - 64]         #alloc, idx 353, cyc -1
	mov r11d, r13d                         #add, idx 49, cyc 2
	add r11d, r10d                         #add, idx 50, cyc 3
	mov r13d, r11d                         #assign, idx 51, cyc 4
	mov dword ptr [rbp - 44], r12d         #spill, idx 354, cyc -1
	mov dword ptr [rbp - 64], r13d         #spill, idx 355, cyc -1


.L_b14_jmp_tail_of_if:
	jmp .L_b28_jmp_tail_of_if 


.L_b17_if_else:
	mov r10d, 2                            #li, idx 52, cyc 0
	mov r12d, dword ptr [rbp - 20]         #alloc, idx 356, cyc -1
	mov r11d, r12d                         #sub, idx 53, cyc 0
	sub r11d, r10d                         #sub, idx 54, cyc 1
	mov r10d, 2                            #li, idx 56, cyc 1
	mov r12d, r11d                         #assign, idx 55, cyc 2
	mov r13d, dword ptr [rbp - 56]         #alloc, idx 357, cyc -1
	mov r11d, r13d                         #add, idx 57, cyc 2
	add r11d, r10d                         #add, idx 58, cyc 3
	mov r13d, r11d                         #assign, idx 59, cyc 4
	mov dword ptr [rbp - 20], r12d         #spill, idx 358, cyc -1
	mov dword ptr [rbp - 56], r13d         #spill, idx 359, cyc -1


.L_b19_if_cond:
	mov r11d, dword ptr [rbp - 32]         #alloc, idx 360, cyc -1
	mov r12d, dword ptr [rbp - 44]         #alloc, idx 361, cyc -1
	cmp r11d, r12d                         #cmple, idx 60, cyc 0
	setle r10b                             #cmple, idx 60, cyc 0
	movzx r10d, r10b                       #cmple, idx 60, cyc 0
	jle .L_b22_if_else 
	mov r10d, 3                            #li, idx 61, cyc 0
	mov r12d, dword ptr [rbp - 32]         #alloc, idx 362, cyc -1
	mov r11d, r12d                         #sub, idx 62, cyc 0
	sub r11d, r10d                         #sub, idx 63, cyc 1
	mov r10d, 3                            #li, idx 65, cyc 1
	mov r12d, r11d                         #assign, idx 64, cyc 2
	mov r13d, dword ptr [rbp - 64]         #alloc, idx 363, cyc -1
	mov r11d, r13d                         #add, idx 66, cyc 2
	add r11d, r10d                         #add, idx 67, cyc 3
	mov r13d, r11d                         #assign, idx 68, cyc 4
	mov dword ptr [rbp - 32], r12d         #spill, idx 364, cyc -1
	mov dword ptr [rbp - 64], r13d         #spill, idx 365, cyc -1
	jmp .L_b24_jmp_tail_of_if 


.L_b22_if_else:
	mov r10d, 4                            #li, idx 69, cyc 0
	mov r12d, dword ptr [rbp - 44]         #alloc, idx 366, cyc -1
	mov r11d, r12d                         #add, idx 70, cyc 0
	add r11d, r10d                         #add, idx 71, cyc 1
	mov r10d, 4                            #li, idx 73, cyc 1
	mov r12d, r11d                         #assign, idx 72, cyc 2
	mov r13d, dword ptr [rbp - 64]         #alloc, idx 367, cyc -1
	mov r11d, r13d                         #add, idx 74, cyc 2
	add r11d, r10d                         #add, idx 75, cyc 3
	mov r13d, r11d                         #assign, idx 76, cyc 4
	mov dword ptr [rbp - 44], r12d         #spill, idx 368, cyc -1
	mov dword ptr [rbp - 64], r13d         #spill, idx 369, cyc -1


.L_b24_jmp_tail_of_if:


.L_b28_jmp_tail_of_if:
	mov r11d, dword ptr [rbp - 72]         #alloc, idx 370, cyc -1
	mov r10d, r11d                         #add, idx 77, cyc 0
	mov r13d, dword ptr [rbp - 8]          #alloc, idx 371, cyc -1
	mov r12d, r13d                         #add, idx 90, cyc 0
	add r10d, r13d                         #add, idx 78, cyc 1
	mov r15d, dword ptr [rbp - 20]         #alloc, idx 372, cyc -1
	mov r14d, r15d                         #add, idx 93, cyc 1
	mov r11d, r10d                         #add, idx 79, cyc 2
	mov r10d, 1                            #li, idx 96, cyc 2
	add r11d, r15d                         #add, idx 80, cyc 3
	mov dword ptr [rbp - 224], r12d        #spill, idx 373, cyc -1
	mov dword ptr [rbp - 228], r14d        #spill, idx 374, cyc -1
	mov r14d, dword ptr [rbp - 80]         #alloc, idx 375, cyc -1
	mov r12d, r14d                         #add, idx 97, cyc 3
	mov r13d, r11d                         #add, idx 81, cyc 4
	add r12d, r10d                         #add, idx 98, cyc 4
	mov r14d, r12d                         #assign, idx 99, cyc 5
	mov r10d, dword ptr [rbp - 32]         #alloc, idx 376, cyc -1
	add r13d, r10d                         #add, idx 82, cyc 5
	mov r11d, r13d                         #add, idx 83, cyc 6
	mov r12d, dword ptr [rbp - 224]        #alloc, idx 377, cyc -1
	mov r13d, dword ptr [rbp - 56]         #alloc, idx 378, cyc -1
	add r12d, r13d                         #add, idx 91, cyc 6
	mov r15d, r12d                         #assign, idx 92, cyc 7
	mov r12d, dword ptr [rbp - 44]         #alloc, idx 379, cyc -1
	add r11d, r12d                         #add, idx 84, cyc 7
	mov dword ptr [rbp - 80], r14d         #spill, idx 380, cyc -1
	mov r14d, r11d                         #add, idx 85, cyc 8
	mov r11d, dword ptr [rbp - 228]        #alloc, idx 381, cyc -1
	mov r10d, dword ptr [rbp - 64]         #alloc, idx 382, cyc -1
	add r11d, r10d                         #add, idx 94, cyc 8
	mov dword ptr [rbp - 8], r15d          #spill, idx 383, cyc -1
	mov r15d, r11d                         #assign, idx 95, cyc 9
	add r14d, r13d                         #add, idx 86, cyc 9
	mov r11d, r14d                         #add, idx 87, cyc 10
	add r11d, r10d                         #add, idx 88, cyc 11
	mov r14d, r11d                         #assign, idx 89, cyc 12
	mov dword ptr [rbp - 72], r14d         #spill, idx 384, cyc -1
	mov dword ptr [rbp - 20], r15d         #spill, idx 385, cyc -1
	jmp .L_b3_while_cond 


.L_b32_jmp_tail_of_while:
	mov r10d, 1                            #li, idx 100, cyc 0
	mov r12d, dword ptr [rbp - 72]         #alloc, idx 386, cyc -1
	mov r11d, r12d                         #add, idx 101, cyc 0
	add r11d, r10d                         #add, idx 102, cyc 1
	mov r12d, r11d                         #assign, idx 103, cyc 2
	#---------------- print ret ----------------# 
	mov esi, r12d 
	lea rdi, [rip + fmt] 
	mov eax, 0 
	call printf@PLT 
	#------------------------------------------# 
	mov eax, r12d 
	mov dword ptr [rbp - 72], r12d         #spill, idx 387, cyc -1
	jmp .L_return 


.L_return:
	mov rsp, rbp 
	pop rbp 
	ret 

.section .note.GNU-stack,"",@progbits



```

Executable files named "**a0**" and "**a1**" will be produced in current location.
It can run on Linux‑x86‑64 and print the return int-value of the program.

---

