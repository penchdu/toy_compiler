# VSC - Very Simple Compiler

After I studied the book *Engineering a Compiler 3E*, I found the compiler theory is much simpler than I supposed, so I decide to write a toy compiler to kill some time and have fun.

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
- control-flow basic blocks
- conditional branches and jumps

I tested programs containing deeply nested scopes and multiple `if/else` branches, and multiple `return`, the generated executables produced the expected results.

Next steps: SSA, -O1 register allocation.


### 2026.9.29

**score based inst-selecter & wavefront reg-alloc (-O1)**

Implemented an innovative wavefront based reg-alloc method for -O1 optimization:

- **Wavefront Score Model**: Introduced an asymmetric decay wave model for virtual registers. It simulates usage peaks and rapid post-use decay, dynamically scoring registers for allocation choices.  The idea is that for a virtual register (VR), if it is used continuously or near-continuously in the near future, it generates a smoothly rising peak to signal to the allocator that this VR should not be spilled. If it remains unused beyond a certain range, it rapidly drops into a trough, indicating that this VR can be spilled. The allocator compares the peak values of all VRs at the current position to decide which VR to spill.
- Wavefront Register Allocator: driven by wavefront scores (`a1.s`), reducing unnecessary spill compared to the baseline -O0 strategy (`a0.s`). Not spill on every vr use, but spill all vr at the end of every BasicBlock.
- Score-Driven Instruction Scheduler : BasicBlock level, consider critical path latency, functional unit availability (ALU/IMUL/DIV), consumption scores, transient variable, last use, single-line, seems not bad.

Next steps: global-Inst-select and global-reg-alloc

## Project Status
- [x] Lexer
- [x] two-stack Parser
- [ ] Pratt Parser
- [x] AST
- [x] namespace Scope manage
- [x] Control flow Scope manage / BasicBlock
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
- no function calls, no ABI
- type only `int`
- identifiers follow the same naming rules as in the C language



## Compiler components detail

### Lexer
...

### Parser


#### Scope Management & Lexical Scoping


Implements a hierarchical lexical scoping mechanism to support nested code blocks (`{ ... }`), control-flow branching (`if`/`else`), and correct variable name resolution.

- Scope Tree Architecture: Each `Scope` maintains a parent pointer and child references, forming a hierarchical scope tree. Variables are declared within the symbol table of their enclosing scope.
- Lexical Resolution & Variable Shadowing: Name lookup starts from the innermost active scope and walks outward through parent scopes. An identifier declared in an inner scope shadows an identifier with the same name in an outer scope.
- Control-Flow Scoping & Basic Blocks: `if`/`else` constructs create dedicated scopes for conditions, branches, and control-flow join points. These scopes are later used to organize basic blocks, jump targets, and control-flow edges.
- Name Disambiguation: During semantic analysis, variables are assigned unique internal names (for example, `global_a` and `block2_a`) so that shadowed identifiers can be distinguished in the flat intermediate representation.



### DAG Construction
...

### Scope and Semantic Analysis

```c

Semantic analysis performs:

- variable declaration checking
- symbol table construction
- scope resolution
- variable renaming

```

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

```c
-O0:
- simple allocation, load/store based strategy

-O1:
- BasicBlock-level Wavefront

```

### Optimization Goal

```c

-O0:
- no optimize


-O1:
- constant fold
- dead code eliminate

```



## Auto Test



```c

$make
$./test/run_test.sh

```




## Single Test

```c

$make
$./build/vsc test.cpp	#generate a0.s a1.s
$gcc a0.s -o a0
$gcc a1.s -o a1
$./a0
$./a1

```


example `test.cpp`:

```c


int test()
{
	int a0 = 1;
	int a1 = 2;
	int a2 = 3;
	int a3 = 4;

	int b0 = 11;
	int b1 = 12;
	int b2 = 13;
	int b3 = 14;

	int c0 = 21;
	int c1 = 22;
	int c2 = 23;
	int c3 = 24;

	int d0 = 31;
	int d1 = 32;
	int d2 = 33;
	int d3 = 34;

	int i0 = 3;
	int i1 = 2;
	int i2 = 2;
	int i3 = 2;

	while ((i0 = i0 - 1) + a0 - a0 + b0 - b0 > 0)
	{
		a0 = a0 + b1;
		a1 = a1 + b0;

		b0 = b0 + c1;
		b1 = b1 + c0;

		c0 = c0 + d1;
		c1 = c1 + d0;

		d0 = d0 + a1;
		d1 = d1 + a0;

		while ((i1 = i1 - 1) + c0 - c0 + d0 - d0 > 0)
		{
			b0 = b0 + a2;
			b1 = b1 + a3;

			a2 = a2 + c0;
			a3 = a3 + c1;

			c0 = c0 + d2;
			c1 = c1 + d3;

			d2 = d2 + b1;
			d3 = d3 + b0;

			while ((i2 = i2 - 1) + a2 - a2 + b2 - b2 > 0)
			{
				c0 = c0 + b2;
				c1 = c1 + b3;

				b2 = b2 + d0;
				b3 = b3 + d1;

				d0 = d0 + a2;
				d1 = d1 + a3;

				a2 = a2 + c2;
				a3 = a3 + c3;

				while ((i3 = i3 - 1) + c2 - c2 + d2 - d2 > 0)
				{
					d0 = d0 + c0;
					d1 = d1 + c1;

					c2 = c2 + a0;
					c3 = c3 + a1;

					a0 = a0 + d2;
					a1 = a1 + d3;

					b0 = b0 + c2;
					b1 = b1 + c3;
				}

				a2 = a2 + b0 + d1;
				a3 = a3 + b1 + d0;
			}

			b2 = b2 + c0 + a2;
			b3 = b3 + c1 + a3;
		}

		c2 = c2 + d0 + b0;
		c3 = c3 + d1 + b1;
	}

	return a0 + a1 + a2 + a3
	    + b0 + b1 + b2 + b3
	    + c0 + c1 + c2 + c3
	    + d0 + d1 + d2 + d3
	    + i0 + i1 + i2 + i3;
}




```


the `vsc` compile the input into two assembly: `a0.s` and `a1.s`


###  -O0  a0.s  

---

```c

spill every register use
...

```

###  -O1  a1.s  

---




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
	sub rsp, 544 

	mov r10d, 1                            #li 1=%0, =%-1,  idx 0, cyc 0
	mov r11d, 2                            #li 2=%2, =%-1,  idx 1, cyc 0
	mov r12d, r10d                         #assign a0=%1, 1=%0,  idx 0, cyc 1
	mov r10d, r11d                         #assign a1=%3, 2=%2,  idx 1, cyc 1
	mov r11d, 3                            #li 3=%4, =%-1,  idx 2, cyc 2
	mov r13d, 4                            #li 4=%6, =%-1,  idx 3, cyc 2
	mov r14d, r11d                         #assign a2=%5, 3=%4,  idx 2, cyc 3
	mov r11d, r13d                         #assign a3=%7, 4=%6,  idx 3, cyc 3
	mov r13d, 11                           #li 11=%8, =%-1,  idx 4, cyc 4
	mov r15d, 12                           #li 12=%10, =%-1,  idx 5, cyc 4
	mov dword ptr [rbp - 8], r12d          #spill a0=%1, =%-1,  idx 735, cyc -1
	mov r12d, r13d                         #assign b0=%9, 11=%8,  idx 4, cyc 5
	mov r13d, r15d                         #assign b1=%11, 12=%10,  idx 5, cyc 5
	mov r15d, 13                           #li 13=%12, =%-1,  idx 6, cyc 6
	mov dword ptr [rbp - 16], r10d         #spill a1=%3, =%-1,  idx 736, cyc -1
	mov r10d, 14                           #li 14=%14, =%-1,  idx 7, cyc 6
	mov dword ptr [rbp - 24], r14d         #spill a2=%5, =%-1,  idx 737, cyc -1
	mov r14d, r15d                         #assign b2=%13, 13=%12,  idx 6, cyc 7
	mov r15d, r10d                         #assign b3=%15, 14=%14,  idx 7, cyc 7
	mov r10d, 21                           #li 21=%16, =%-1,  idx 8, cyc 8
	mov dword ptr [rbp - 32], r11d         #spill a3=%7, =%-1,  idx 738, cyc -1
	mov r11d, 22                           #li 22=%18, =%-1,  idx 9, cyc 8
	mov dword ptr [rbp - 40], r12d         #spill b0=%9, =%-1,  idx 739, cyc -1
	mov r12d, r10d                         #assign c0=%17, 21=%16,  idx 8, cyc 9
	mov r10d, r11d                         #assign c1=%19, 22=%18,  idx 9, cyc 9
	mov r11d, 23                           #li 23=%20, =%-1,  idx 10, cyc 10
	mov dword ptr [rbp - 48], r13d         #spill b1=%11, =%-1,  idx 740, cyc -1
	mov r13d, 24                           #li 24=%22, =%-1,  idx 11, cyc 10
	mov dword ptr [rbp - 56], r14d         #spill b2=%13, =%-1,  idx 741, cyc -1
	mov r14d, r11d                         #assign c2=%21, 23=%20,  idx 10, cyc 11
	mov r11d, r13d                         #assign c3=%23, 24=%22,  idx 11, cyc 11
	mov r13d, 31                           #li 31=%24, =%-1,  idx 12, cyc 12
	mov dword ptr [rbp - 64], r15d         #spill b3=%15, =%-1,  idx 742, cyc -1
	mov r15d, 32                           #li 32=%26, =%-1,  idx 13, cyc 12
	mov dword ptr [rbp - 72], r12d         #spill c0=%17, =%-1,  idx 743, cyc -1
	mov r12d, r13d                         #assign d0=%25, 31=%24,  idx 12, cyc 13
	mov r13d, r15d                         #assign d1=%27, 32=%26,  idx 13, cyc 13
	mov r15d, 33                           #li 33=%28, =%-1,  idx 14, cyc 14
	mov dword ptr [rbp - 80], r10d         #spill c1=%19, =%-1,  idx 744, cyc -1
	mov r10d, 34                           #li 34=%30, =%-1,  idx 15, cyc 14
	mov dword ptr [rbp - 88], r14d         #spill c2=%21, =%-1,  idx 745, cyc -1
	mov r14d, r15d                         #assign d2=%29, 33=%28,  idx 14, cyc 15
	mov r15d, r10d                         #assign d3=%31, 34=%30,  idx 15, cyc 15
	mov r10d, 3                            #li 3=%32, =%-1,  idx 16, cyc 16
	mov dword ptr [rbp - 96], r11d         #spill c3=%23, =%-1,  idx 746, cyc -1
	mov r11d, 2                            #li 2=%34, =%-1,  idx 17, cyc 16
	mov dword ptr [rbp - 104], r12d        #spill d0=%25, =%-1,  idx 747, cyc -1
	mov r12d, r10d                         #assign i0=%33, 3=%32,  idx 16, cyc 17
	mov r10d, r11d                         #assign i1=%35, 2=%34,  idx 17, cyc 17
	mov r11d, 2                            #li 2=%36, =%-1,  idx 18, cyc 18
	mov dword ptr [rbp - 112], r13d        #spill d1=%27, =%-1,  idx 748, cyc -1
	mov r13d, 2                            #li 2=%38, =%-1,  idx 19, cyc 18
	mov dword ptr [rbp - 120], r14d        #spill d2=%29, =%-1,  idx 749, cyc -1
	mov r14d, r11d                         #assign i2=%37, 2=%36,  idx 18, cyc 19
	mov r11d, r13d                         #assign i3=%39, 2=%38,  idx 19, cyc 19


.L_b2_while:


.L_b3_while2_cond:
	mov r13d, 1                            #li 1=%41, =%-1,  idx 21, cyc 0
	mov dword ptr [rbp - 144], r10d        #spill i1=%35, =%-1,  idx 750, cyc -1
	mov r10d, r12d                         #sub -=%42, i0=%33,  idx 20, cyc 0
	sub r10d, r13d                         #sub -=%42, 1=%41,  idx 21, cyc 1
	mov r13d, 0                            #li 0=%40, =%-1,  idx 20, cyc 1
	mov r12d, r10d                         #assign i0=%33, -=%42,  idx 22, cyc 2
	mov r10d, r12d                         #add +=%43, i0=%33,  idx 23, cyc 3
	mov dword ptr [rbp - 160], r11d        #spill i3=%39, =%-1,  idx 751, cyc -1
	mov r11d, dword ptr [rbp - 8]          #alloc a0=%1, =%-1,  idx 752, cyc -1
	add r10d, r11d                         #add +=%43, a0=%1,  idx 24, cyc 4
	mov dword ptr [rbp - 152], r14d        #spill i2=%37, =%-1,  idx 753, cyc -1
	mov r14d, r10d                         #sub -=%44, +=%43,  idx 25, cyc 5
	sub r14d, r11d                         #sub -=%44, a0=%1,  idx 26, cyc 6
	mov r10d, r14d                         #add +=%45, -=%44,  idx 27, cyc 7
	mov r14d, dword ptr [rbp - 40]         #alloc b0=%9, =%-1,  idx 754, cyc -1
	add r10d, r14d                         #add +=%45, b0=%9,  idx 28, cyc 8
	mov dword ptr [rbp - 128], r15d        #spill d3=%31, =%-1,  idx 755, cyc -1
	mov r15d, r10d                         #sub -=%46, +=%45,  idx 29, cyc 9
	sub r15d, r14d                         #sub -=%46, b0=%9,  idx 30, cyc 10
	cmp r15d, r13d                         #cmple -=%46, 0=%40,  idx 31, cyc 11
	setle r10b                             #cmple -=%46, 0=%40,  idx 31, cyc 11
	movzx r10d, r10b                       #cmple -=%46, 0=%40,  idx 31, cyc 11
	jle .L_b32_while2_tail 


.L_b4_while2_body:
	mov r10d, r11d                         #add +=%48, a0=%1,  idx 32, cyc 0
	mov r15d, dword ptr [rbp - 16]         #alloc a1=%3, =%-1,  idx 756, cyc -1
	mov r13d, r15d                         #add +=%49, a1=%3,  idx 35, cyc 0
	mov dword ptr [rbp - 16], r15d         #spill a1=%3, =%-1,  idx 757, cyc -1
	mov r15d, dword ptr [rbp - 48]         #alloc b1=%11, =%-1,  idx 758, cyc -1
	add r10d, r15d                         #add +=%48, b1=%11,  idx 33, cyc 1
	add r13d, r14d                         #add +=%49, b0=%9,  idx 36, cyc 1
	mov r11d, r10d                         #assign a0=%1, +=%48,  idx 34, cyc 2
	mov r10d, r13d                         #assign a1=%3, +=%49,  idx 37, cyc 2
	mov r13d, r14d                         #add +=%50, b0=%9,  idx 38, cyc 3
	mov dword ptr [rbp - 8], r11d          #spill a0=%1, =%-1,  idx 759, cyc -1
	mov r11d, r15d                         #add +=%51, b1=%11,  idx 41, cyc 3
	mov dword ptr [rbp - 16], r10d         #spill a1=%3, =%-1,  idx 760, cyc -1
	mov r10d, dword ptr [rbp - 80]         #alloc c1=%19, =%-1,  idx 761, cyc -1
	add r13d, r10d                         #add +=%50, c1=%19,  idx 39, cyc 4
	mov dword ptr [rbp - 136], r12d        #spill i0=%33, =%-1,  idx 762, cyc -1
	mov r12d, dword ptr [rbp - 72]         #alloc c0=%17, =%-1,  idx 763, cyc -1
	add r11d, r12d                         #add +=%51, c0=%17,  idx 42, cyc 4
	mov r14d, r13d                         #assign b0=%9, +=%50,  idx 40, cyc 5
	mov r15d, r11d                         #assign b1=%11, +=%51,  idx 43, cyc 5
	mov r11d, r12d                         #add +=%52, c0=%17,  idx 44, cyc 6
	mov r13d, r10d                         #add +=%53, c1=%19,  idx 47, cyc 6
	mov dword ptr [rbp - 40], r14d         #spill b0=%9, =%-1,  idx 764, cyc -1
	mov r14d, dword ptr [rbp - 112]        #alloc d1=%27, =%-1,  idx 765, cyc -1
	add r11d, r14d                         #add +=%52, d1=%27,  idx 45, cyc 7
	mov dword ptr [rbp - 48], r15d         #spill b1=%11, =%-1,  idx 766, cyc -1
	mov r15d, dword ptr [rbp - 104]        #alloc d0=%25, =%-1,  idx 767, cyc -1
	add r13d, r15d                         #add +=%53, d0=%25,  idx 48, cyc 7
	mov r12d, r11d                         #assign c0=%17, +=%52,  idx 46, cyc 8
	mov r10d, r13d                         #assign c1=%19, +=%53,  idx 49, cyc 8
	mov r11d, r15d                         #add +=%54, d0=%25,  idx 50, cyc 9
	mov r13d, r14d                         #add +=%55, d1=%27,  idx 53, cyc 9
	mov dword ptr [rbp - 72], r12d         #spill c0=%17, =%-1,  idx 768, cyc -1
	mov r12d, dword ptr [rbp - 16]         #alloc a1=%3, =%-1,  idx 769, cyc -1
	add r11d, r12d                         #add +=%54, a1=%3,  idx 51, cyc 10
	mov dword ptr [rbp - 80], r10d         #spill c1=%19, =%-1,  idx 770, cyc -1
	mov r10d, dword ptr [rbp - 8]          #alloc a0=%1, =%-1,  idx 771, cyc -1
	add r13d, r10d                         #add +=%55, a0=%1,  idx 54, cyc 10
	mov r15d, r11d                         #assign d0=%25, +=%54,  idx 52, cyc 11
	mov r14d, r13d                         #assign d1=%27, +=%55,  idx 55, cyc 11


.L_b5_while:


.L_b6_while5_cond:
	mov r11d, 1                            #li 1=%57, =%-1,  idx 23, cyc 0
	mov dword ptr [rbp - 112], r14d        #spill d1=%27, =%-1,  idx 772, cyc -1
	mov r14d, dword ptr [rbp - 144]        #alloc i1=%35, =%-1,  idx 773, cyc -1
	mov r13d, r14d                         #sub -=%58, i1=%35,  idx 56, cyc 0
	sub r13d, r11d                         #sub -=%58, 1=%57,  idx 57, cyc 1
	mov r11d, 0                            #li 0=%56, =%-1,  idx 22, cyc 1
	mov r14d, r13d                         #assign i1=%35, -=%58,  idx 58, cyc 2
	mov r13d, r14d                         #add +=%59, i1=%35,  idx 59, cyc 3
	mov dword ptr [rbp - 104], r15d        #spill d0=%25, =%-1,  idx 774, cyc -1
	mov r15d, dword ptr [rbp - 72]         #alloc c0=%17, =%-1,  idx 775, cyc -1
	add r13d, r15d                         #add +=%59, c0=%17,  idx 60, cyc 4
	mov dword ptr [rbp - 228], r11d        #spill 0=%56, =%-1,  idx 776, cyc -1
	mov r11d, r13d                         #sub -=%60, +=%59,  idx 61, cyc 5
	sub r11d, r15d                         #sub -=%60, c0=%17,  idx 62, cyc 6
	mov r13d, r11d                         #add +=%61, -=%60,  idx 63, cyc 7
	mov r11d, dword ptr [rbp - 104]        #alloc d0=%25, =%-1,  idx 777, cyc -1
	add r13d, r11d                         #add +=%61, d0=%25,  idx 64, cyc 8
	mov dword ptr [rbp - 8], r10d          #spill a0=%1, =%-1,  idx 778, cyc -1
	mov r10d, r13d                         #sub -=%62, +=%61,  idx 65, cyc 9
	sub r10d, r11d                         #sub -=%62, d0=%25,  idx 66, cyc 10
	mov dword ptr [rbp - 16], r12d         #spill a1=%3, =%-1,  idx 779, cyc -1
	mov r12d, dword ptr [rbp - 228]        #alloc 0=%56, =%-1,  idx 780, cyc -1
	cmp r10d, r12d                         #cmple -=%62, 0=%56,  idx 67, cyc 11
	setle r13b                             #cmple -=%62, 0=%56,  idx 67, cyc 11
	movzx r13d, r13b                       #cmple -=%62, 0=%56,  idx 67, cyc 11
	jle .L_b27_while5_tail 


.L_b7_while5_body:
	mov r12d, dword ptr [rbp - 40]         #alloc b0=%9, =%-1,  idx 781, cyc -1
	mov r10d, r12d                         #add +=%64, b0=%9,  idx 68, cyc 0
	mov dword ptr [rbp - 104], r11d        #spill d0=%25, =%-1,  idx 782, cyc -1
	mov r11d, dword ptr [rbp - 48]         #alloc b1=%11, =%-1,  idx 783, cyc -1
	mov r13d, r11d                         #add +=%65, b1=%11,  idx 71, cyc 0
	mov dword ptr [rbp - 72], r15d         #spill c0=%17, =%-1,  idx 784, cyc -1
	mov r15d, dword ptr [rbp - 24]         #alloc a2=%5, =%-1,  idx 785, cyc -1
	add r10d, r15d                         #add +=%64, a2=%5,  idx 69, cyc 1
	mov dword ptr [rbp - 24], r15d         #spill a2=%5, =%-1,  idx 786, cyc -1
	mov r15d, dword ptr [rbp - 32]         #alloc a3=%7, =%-1,  idx 787, cyc -1
	add r13d, r15d                         #add +=%65, a3=%7,  idx 72, cyc 1
	mov r12d, r10d                         #assign b0=%9, +=%64,  idx 70, cyc 2
	mov r11d, r13d                         #assign b1=%11, +=%65,  idx 73, cyc 2
	mov r13d, dword ptr [rbp - 24]         #alloc a2=%5, =%-1,  idx 788, cyc -1
	mov r10d, r13d                         #add +=%66, a2=%5,  idx 74, cyc 3
	mov dword ptr [rbp - 40], r12d         #spill b0=%9, =%-1,  idx 789, cyc -1
	mov r12d, r15d                         #add +=%67, a3=%7,  idx 77, cyc 3
	mov dword ptr [rbp - 48], r11d         #spill b1=%11, =%-1,  idx 790, cyc -1
	mov r11d, dword ptr [rbp - 72]         #alloc c0=%17, =%-1,  idx 791, cyc -1
	add r10d, r11d                         #add +=%66, c0=%17,  idx 75, cyc 4
	mov dword ptr [rbp - 144], r14d        #spill i1=%35, =%-1,  idx 792, cyc -1
	mov r14d, dword ptr [rbp - 80]         #alloc c1=%19, =%-1,  idx 793, cyc -1
	add r12d, r14d                         #add +=%67, c1=%19,  idx 78, cyc 4
	mov r13d, r10d                         #assign a2=%5, +=%66,  idx 76, cyc 5
	mov r15d, r12d                         #assign a3=%7, +=%67,  idx 79, cyc 5
	mov r10d, r11d                         #add +=%68, c0=%17,  idx 80, cyc 6
	mov r12d, r14d                         #add +=%69, c1=%19,  idx 83, cyc 6
	mov dword ptr [rbp - 24], r13d         #spill a2=%5, =%-1,  idx 794, cyc -1
	mov r13d, dword ptr [rbp - 120]        #alloc d2=%29, =%-1,  idx 795, cyc -1
	add r10d, r13d                         #add +=%68, d2=%29,  idx 81, cyc 7
	mov dword ptr [rbp - 32], r15d         #spill a3=%7, =%-1,  idx 796, cyc -1
	mov r15d, dword ptr [rbp - 128]        #alloc d3=%31, =%-1,  idx 797, cyc -1
	add r12d, r15d                         #add +=%69, d3=%31,  idx 84, cyc 7
	mov r11d, r10d                         #assign c0=%17, +=%68,  idx 82, cyc 8
	mov r14d, r12d                         #assign c1=%19, +=%69,  idx 85, cyc 8
	mov r10d, r13d                         #add +=%70, d2=%29,  idx 86, cyc 9
	mov r12d, r15d                         #add +=%71, d3=%31,  idx 89, cyc 9
	mov dword ptr [rbp - 72], r11d         #spill c0=%17, =%-1,  idx 798, cyc -1
	mov r11d, dword ptr [rbp - 48]         #alloc b1=%11, =%-1,  idx 799, cyc -1
	add r10d, r11d                         #add +=%70, b1=%11,  idx 87, cyc 10
	mov dword ptr [rbp - 80], r14d         #spill c1=%19, =%-1,  idx 800, cyc -1
	mov r14d, dword ptr [rbp - 40]         #alloc b0=%9, =%-1,  idx 801, cyc -1
	add r12d, r14d                         #add +=%71, b0=%9,  idx 90, cyc 10
	mov r13d, r10d                         #assign d2=%29, +=%70,  idx 88, cyc 11
	mov r15d, r12d                         #assign d3=%31, +=%71,  idx 91, cyc 11


.L_b8_while:


.L_b9_while8_cond:
	mov r10d, 1                            #li 1=%73, =%-1,  idx 25, cyc 0
	mov dword ptr [rbp - 120], r13d        #spill d2=%29, =%-1,  idx 802, cyc -1
	mov r13d, dword ptr [rbp - 152]        #alloc i2=%37, =%-1,  idx 803, cyc -1
	mov r12d, r13d                         #sub -=%74, i2=%37,  idx 92, cyc 0
	sub r12d, r10d                         #sub -=%74, 1=%73,  idx 93, cyc 1
	mov r10d, 0                            #li 0=%72, =%-1,  idx 24, cyc 1
	mov r13d, r12d                         #assign i2=%37, -=%74,  idx 94, cyc 2
	mov r12d, r13d                         #add +=%75, i2=%37,  idx 95, cyc 3
	mov dword ptr [rbp - 128], r15d        #spill d3=%31, =%-1,  idx 804, cyc -1
	mov r15d, dword ptr [rbp - 24]         #alloc a2=%5, =%-1,  idx 805, cyc -1
	add r12d, r15d                         #add +=%75, a2=%5,  idx 96, cyc 4
	mov dword ptr [rbp - 292], r10d        #spill 0=%72, =%-1,  idx 806, cyc -1
	mov r10d, r12d                         #sub -=%76, +=%75,  idx 97, cyc 5
	sub r10d, r15d                         #sub -=%76, a2=%5,  idx 98, cyc 6
	mov r12d, r10d                         #add +=%77, -=%76,  idx 99, cyc 7
	mov r10d, dword ptr [rbp - 56]         #alloc b2=%13, =%-1,  idx 807, cyc -1
	add r12d, r10d                         #add +=%77, b2=%13,  idx 100, cyc 8
	mov dword ptr [rbp - 40], r14d         #spill b0=%9, =%-1,  idx 808, cyc -1
	mov r14d, r12d                         #sub -=%78, +=%77,  idx 101, cyc 9
	sub r14d, r10d                         #sub -=%78, b2=%13,  idx 102, cyc 10
	mov dword ptr [rbp - 48], r11d         #spill b1=%11, =%-1,  idx 809, cyc -1
	mov r11d, dword ptr [rbp - 292]        #alloc 0=%72, =%-1,  idx 810, cyc -1
	cmp r14d, r11d                         #cmple -=%78, 0=%72,  idx 103, cyc 11
	setle r12b                             #cmple -=%78, 0=%72,  idx 103, cyc 11
	movzx r12d, r12b                       #cmple -=%78, 0=%72,  idx 103, cyc 11
	jle .L_b22_while8_tail 


.L_b10_while8_body:
	mov r12d, dword ptr [rbp - 72]         #alloc c0=%17, =%-1,  idx 811, cyc -1
	mov r11d, r12d                         #add +=%80, c0=%17,  idx 104, cyc 0
	mov dword ptr [rbp - 24], r15d         #spill a2=%5, =%-1,  idx 812, cyc -1
	mov r15d, dword ptr [rbp - 80]         #alloc c1=%19, =%-1,  idx 813, cyc -1
	mov r14d, r15d                         #add +=%81, c1=%19,  idx 107, cyc 0
	add r11d, r10d                         #add +=%80, b2=%13,  idx 105, cyc 1
	mov dword ptr [rbp - 56], r10d         #spill b2=%13, =%-1,  idx 814, cyc -1
	mov r10d, dword ptr [rbp - 64]         #alloc b3=%15, =%-1,  idx 815, cyc -1
	add r14d, r10d                         #add +=%81, b3=%15,  idx 108, cyc 1
	mov r12d, r11d                         #assign c0=%17, +=%80,  idx 106, cyc 2
	mov r15d, r14d                         #assign c1=%19, +=%81,  idx 109, cyc 2
	mov r14d, dword ptr [rbp - 56]         #alloc b2=%13, =%-1,  idx 816, cyc -1
	mov r11d, r14d                         #add +=%82, b2=%13,  idx 110, cyc 3
	mov dword ptr [rbp - 72], r12d         #spill c0=%17, =%-1,  idx 817, cyc -1
	mov r12d, r10d                         #add +=%83, b3=%15,  idx 113, cyc 3
	mov dword ptr [rbp - 80], r15d         #spill c1=%19, =%-1,  idx 818, cyc -1
	mov r15d, dword ptr [rbp - 104]        #alloc d0=%25, =%-1,  idx 819, cyc -1
	add r11d, r15d                         #add +=%82, d0=%25,  idx 111, cyc 4
	mov dword ptr [rbp - 152], r13d        #spill i2=%37, =%-1,  idx 820, cyc -1
	mov r13d, dword ptr [rbp - 112]        #alloc d1=%27, =%-1,  idx 821, cyc -1
	add r12d, r13d                         #add +=%83, d1=%27,  idx 114, cyc 4
	mov r14d, r11d                         #assign b2=%13, +=%82,  idx 112, cyc 5
	mov r10d, r12d                         #assign b3=%15, +=%83,  idx 115, cyc 5
	mov r11d, r15d                         #add +=%84, d0=%25,  idx 116, cyc 6
	mov r12d, r13d                         #add +=%85, d1=%27,  idx 119, cyc 6
	mov dword ptr [rbp - 56], r14d         #spill b2=%13, =%-1,  idx 822, cyc -1
	mov r14d, dword ptr [rbp - 24]         #alloc a2=%5, =%-1,  idx 823, cyc -1
	add r11d, r14d                         #add +=%84, a2=%5,  idx 117, cyc 7
	mov dword ptr [rbp - 64], r10d         #spill b3=%15, =%-1,  idx 824, cyc -1
	mov r10d, dword ptr [rbp - 32]         #alloc a3=%7, =%-1,  idx 825, cyc -1
	add r12d, r10d                         #add +=%85, a3=%7,  idx 120, cyc 7
	mov r15d, r11d                         #assign d0=%25, +=%84,  idx 118, cyc 8
	mov r13d, r12d                         #assign d1=%27, +=%85,  idx 121, cyc 8
	mov r11d, r14d                         #add +=%86, a2=%5,  idx 122, cyc 9
	mov r12d, r10d                         #add +=%87, a3=%7,  idx 125, cyc 9
	mov dword ptr [rbp - 104], r15d        #spill d0=%25, =%-1,  idx 826, cyc -1
	mov r15d, dword ptr [rbp - 88]         #alloc c2=%21, =%-1,  idx 827, cyc -1
	add r11d, r15d                         #add +=%86, c2=%21,  idx 123, cyc 10
	mov dword ptr [rbp - 112], r13d        #spill d1=%27, =%-1,  idx 828, cyc -1
	mov r13d, dword ptr [rbp - 96]         #alloc c3=%23, =%-1,  idx 829, cyc -1
	add r12d, r13d                         #add +=%87, c3=%23,  idx 126, cyc 10
	mov r14d, r11d                         #assign a2=%5, +=%86,  idx 124, cyc 11
	mov r10d, r12d                         #assign a3=%7, +=%87,  idx 127, cyc 11


.L_b11_while:


.L_b12_while11_cond:
	mov r11d, 1                            #li 1=%89, =%-1,  idx 27, cyc 0
	mov dword ptr [rbp - 32], r10d         #spill a3=%7, =%-1,  idx 830, cyc -1
	mov r10d, dword ptr [rbp - 160]        #alloc i3=%39, =%-1,  idx 831, cyc -1
	mov r12d, r10d                         #sub -=%90, i3=%39,  idx 128, cyc 0
	sub r12d, r11d                         #sub -=%90, 1=%89,  idx 129, cyc 1
	mov r11d, 0                            #li 0=%88, =%-1,  idx 26, cyc 1
	mov r10d, r12d                         #assign i3=%39, -=%90,  idx 130, cyc 2
	mov r12d, r10d                         #add +=%91, i3=%39,  idx 131, cyc 3
	add r12d, r15d                         #add +=%91, c2=%21,  idx 132, cyc 4
	mov dword ptr [rbp - 96], r13d         #spill c3=%23, =%-1,  idx 832, cyc -1
	mov r13d, r12d                         #sub -=%92, +=%91,  idx 133, cyc 5
	sub r13d, r15d                         #sub -=%92, c2=%21,  idx 134, cyc 6
	mov r12d, r13d                         #add +=%93, -=%92,  idx 135, cyc 7
	mov r13d, dword ptr [rbp - 120]        #alloc d2=%29, =%-1,  idx 833, cyc -1
	add r12d, r13d                         #add +=%93, d2=%29,  idx 136, cyc 8
	mov dword ptr [rbp - 24], r14d         #spill a2=%5, =%-1,  idx 834, cyc -1
	mov r14d, r12d                         #sub -=%94, +=%93,  idx 137, cyc 9
	sub r14d, r13d                         #sub -=%94, d2=%29,  idx 138, cyc 10
	cmp r14d, r11d                         #cmple -=%94, 0=%88,  idx 139, cyc 11
	setle r12b                             #cmple -=%94, 0=%88,  idx 139, cyc 11
	movzx r12d, r12b                       #cmple -=%94, 0=%88,  idx 139, cyc 11
	jle .L_b17_while11_tail 


.L_b13_while11_body:
	mov r12d, dword ptr [rbp - 104]        #alloc d0=%25, =%-1,  idx 835, cyc -1
	mov r11d, r12d                         #add +=%96, d0=%25,  idx 140, cyc 0
	mov dword ptr [rbp - 120], r13d        #spill d2=%29, =%-1,  idx 836, cyc -1
	mov r13d, dword ptr [rbp - 112]        #alloc d1=%27, =%-1,  idx 837, cyc -1
	mov r14d, r13d                         #add +=%97, d1=%27,  idx 143, cyc 0
	mov dword ptr [rbp - 88], r15d         #spill c2=%21, =%-1,  idx 838, cyc -1
	mov r15d, dword ptr [rbp - 72]         #alloc c0=%17, =%-1,  idx 839, cyc -1
	add r11d, r15d                         #add +=%96, c0=%17,  idx 141, cyc 1
	mov dword ptr [rbp - 72], r15d         #spill c0=%17, =%-1,  idx 840, cyc -1
	mov r15d, dword ptr [rbp - 80]         #alloc c1=%19, =%-1,  idx 841, cyc -1
	add r14d, r15d                         #add +=%97, c1=%19,  idx 144, cyc 1
	mov r12d, r11d                         #assign d0=%25, +=%96,  idx 142, cyc 2
	mov r13d, r14d                         #assign d1=%27, +=%97,  idx 145, cyc 2
	mov r14d, dword ptr [rbp - 40]         #alloc b0=%9, =%-1,  idx 842, cyc -1
	mov r11d, r14d                         #add +=%102, b0=%9,  idx 158, cyc 3
	mov dword ptr [rbp - 80], r15d         #spill c1=%19, =%-1,  idx 843, cyc -1
	mov dword ptr [rbp - 104], r12d        #spill d0=%25, =%-1,  idx 844, cyc -1
	mov r12d, dword ptr [rbp - 48]         #alloc b1=%11, =%-1,  idx 845, cyc -1
	mov r15d, r12d                         #add +=%103, b1=%11,  idx 161, cyc 3
	mov dword ptr [rbp - 112], r13d        #spill d1=%27, =%-1,  idx 846, cyc -1
	mov dword ptr [rbp - 412], r11d        #spill +=%102, =%-1,  idx 847, cyc -1
	mov r11d, dword ptr [rbp - 88]         #alloc c2=%21, =%-1,  idx 848, cyc -1
	mov r13d, r11d                         #add +=%98, c2=%21,  idx 146, cyc 4
	mov dword ptr [rbp - 40], r14d         #spill b0=%9, =%-1,  idx 849, cyc -1
	mov dword ptr [rbp - 160], r10d        #spill i3=%39, =%-1,  idx 850, cyc -1
	mov r10d, dword ptr [rbp - 96]         #alloc c3=%23, =%-1,  idx 851, cyc -1
	mov r14d, r10d                         #add +=%99, c3=%23,  idx 149, cyc 4
	mov dword ptr [rbp - 48], r12d         #spill b1=%11, =%-1,  idx 852, cyc -1
	mov r12d, dword ptr [rbp - 8]          #alloc a0=%1, =%-1,  idx 853, cyc -1
	add r13d, r12d                         #add +=%98, a0=%1,  idx 147, cyc 5
	mov dword ptr [rbp - 416], r15d        #spill +=%103, =%-1,  idx 854, cyc -1
	mov r15d, dword ptr [rbp - 16]         #alloc a1=%3, =%-1,  idx 855, cyc -1
	add r14d, r15d                         #add +=%99, a1=%3,  idx 150, cyc 5
	mov r11d, r13d                         #assign c2=%21, +=%98,  idx 148, cyc 6
	mov r10d, r14d                         #assign c3=%23, +=%99,  idx 151, cyc 6
	mov r13d, dword ptr [rbp - 412]        #alloc +=%102, =%-1,  idx 856, cyc -1
	add r13d, r11d                         #add +=%102, c2=%21,  idx 159, cyc 7
	mov r14d, dword ptr [rbp - 416]        #alloc +=%103, =%-1,  idx 857, cyc -1
	add r14d, r10d                         #add +=%103, c3=%23,  idx 162, cyc 7
	mov dword ptr [rbp - 16], r15d         #spill a1=%3, =%-1,  idx 858, cyc -1
	mov r15d, r13d                         #assign b0=%9, +=%102,  idx 160, cyc 8
	mov r13d, r14d                         #assign b1=%11, +=%103,  idx 163, cyc 8
	mov r14d, r12d                         #add +=%100, a0=%1,  idx 152, cyc 9
	mov dword ptr [rbp - 88], r11d         #spill c2=%21, =%-1,  idx 859, cyc -1
	mov dword ptr [rbp - 40], r15d         #spill b0=%9, =%-1,  idx 860, cyc -1
	mov r15d, dword ptr [rbp - 16]         #alloc a1=%3, =%-1,  idx 861, cyc -1
	mov r11d, r15d                         #add +=%101, a1=%3,  idx 155, cyc 9
	mov dword ptr [rbp - 96], r10d         #spill c3=%23, =%-1,  idx 862, cyc -1
	mov r10d, dword ptr [rbp - 120]        #alloc d2=%29, =%-1,  idx 863, cyc -1
	add r14d, r10d                         #add +=%100, d2=%29,  idx 153, cyc 10
	mov dword ptr [rbp - 48], r13d         #spill b1=%11, =%-1,  idx 864, cyc -1
	mov r13d, dword ptr [rbp - 128]        #alloc d3=%31, =%-1,  idx 865, cyc -1
	add r11d, r13d                         #add +=%101, d3=%31,  idx 156, cyc 10
	mov r12d, r14d                         #assign a0=%1, +=%100,  idx 154, cyc 11
	mov r15d, r11d                         #assign a1=%3, +=%101,  idx 157, cyc 11


.L_b16_while11_body_suffix:
	mov dword ptr [rbp - 8], r12d          #while recover spill a0=%1, =%-1,  idx 866, cyc -1
	mov dword ptr [rbp - 16], r15d         #while recover spill a1=%3, =%-1,  idx 867, cyc -1
	mov dword ptr [rbp - 120], r10d        #while recover spill d2=%29, =%-1,  idx 868, cyc -1
	mov dword ptr [rbp - 128], r13d        #while recover spill d3=%31, =%-1,  idx 869, cyc -1
	mov r14d, dword ptr [rbp - 24]         #while 4, ld  a2=%5, =%-1,  idx 870, cyc -1
	mov r10d, dword ptr [rbp - 32]         #while 4, ld  a3=%7, =%-1,  idx 871, cyc -1
	mov r15d, dword ptr [rbp - 88]         #while 4, ld  c2=%21, =%-1,  idx 872, cyc -1
	mov r13d, dword ptr [rbp - 96]         #while 4, ld  c3=%23, =%-1,  idx 873, cyc -1
	jmp .L_b12_while11_cond 


.L_b17_while11_tail:


.L_b18:
	mov r12d, dword ptr [rbp - 24]         #alloc a2=%5, =%-1,  idx 874, cyc -1
	mov r11d, r12d                         #add +=%104, a2=%5,  idx 164, cyc 0
	mov dword ptr [rbp - 120], r13d        #spill d2=%29, =%-1,  idx 875, cyc -1
	mov r13d, dword ptr [rbp - 32]         #alloc a3=%7, =%-1,  idx 876, cyc -1
	mov r14d, r13d                         #add +=%106, a3=%7,  idx 169, cyc 0
	mov dword ptr [rbp - 88], r15d         #spill c2=%21, =%-1,  idx 877, cyc -1
	mov r15d, dword ptr [rbp - 40]         #alloc b0=%9, =%-1,  idx 878, cyc -1
	add r11d, r15d                         #add +=%104, b0=%9,  idx 165, cyc 1
	mov dword ptr [rbp - 24], r12d         #spill a2=%5, =%-1,  idx 879, cyc -1
	mov r12d, dword ptr [rbp - 48]         #alloc b1=%11, =%-1,  idx 880, cyc -1
	add r14d, r12d                         #add +=%106, b1=%11,  idx 170, cyc 1
	mov dword ptr [rbp - 32], r13d         #spill a3=%7, =%-1,  idx 881, cyc -1
	mov r13d, r11d                         #add +=%105, +=%104,  idx 166, cyc 2
	mov r11d, r14d                         #add +=%107, +=%106,  idx 171, cyc 2
	mov r14d, dword ptr [rbp - 112]        #alloc d1=%27, =%-1,  idx 882, cyc -1
	add r13d, r14d                         #add +=%105, d1=%27,  idx 167, cyc 3
	mov dword ptr [rbp - 40], r15d         #spill b0=%9, =%-1,  idx 883, cyc -1
	mov r15d, dword ptr [rbp - 104]        #alloc d0=%25, =%-1,  idx 884, cyc -1
	add r11d, r15d                         #add +=%107, d0=%25,  idx 172, cyc 3
	mov dword ptr [rbp - 48], r12d         #spill b1=%11, =%-1,  idx 885, cyc -1
	mov r12d, r13d                         #assign a2=%5, +=%105,  idx 168, cyc 4
	mov r13d, r11d                         #assign a3=%7, +=%107,  idx 173, cyc 4


.L_b21_while8_body_suffix:
	mov dword ptr [rbp - 24], r12d         #while spill, cond-ld st a2=%5, =%-1,  idx 886, cyc -1
	mov dword ptr [rbp - 24], r12d         #while recover spill a2=%5, =%-1,  idx 887, cyc -1
	mov dword ptr [rbp - 32], r13d         #while recover spill a3=%7, =%-1,  idx 888, cyc -1
	mov dword ptr [rbp - 104], r15d        #while recover spill d0=%25, =%-1,  idx 889, cyc -1
	mov dword ptr [rbp - 112], r14d        #while recover spill d1=%27, =%-1,  idx 890, cyc -1
	mov dword ptr [rbp - 160], r10d        #while recover spill i3=%39, =%-1,  idx 891, cyc -1
	mov r14d, dword ptr [rbp - 40]         #while 4, ld  b0=%9, =%-1,  idx 892, cyc -1
	mov r11d, dword ptr [rbp - 48]         #while 4, ld  b1=%11, =%-1,  idx 893, cyc -1
	mov r13d, dword ptr [rbp - 120]        #while 4, ld  d2=%29, =%-1,  idx 894, cyc -1
	mov r15d, dword ptr [rbp - 128]        #while 4, ld  d3=%31, =%-1,  idx 895, cyc -1
	jmp .L_b9_while8_cond 


.L_b22_while8_tail:


.L_b23:
	mov r11d, r10d                         #add +=%108, b2=%13,  idx 174, cyc 0
	mov r14d, dword ptr [rbp - 64]         #alloc b3=%15, =%-1,  idx 896, cyc -1
	mov r12d, r14d                         #add +=%110, b3=%15,  idx 179, cyc 0
	mov dword ptr [rbp - 24], r15d         #spill a2=%5, =%-1,  idx 897, cyc -1
	mov r15d, dword ptr [rbp - 72]         #alloc c0=%17, =%-1,  idx 898, cyc -1
	add r11d, r15d                         #add +=%108, c0=%17,  idx 175, cyc 1
	mov dword ptr [rbp - 56], r10d         #spill b2=%13, =%-1,  idx 899, cyc -1
	mov r10d, dword ptr [rbp - 80]         #alloc c1=%19, =%-1,  idx 900, cyc -1
	add r12d, r10d                         #add +=%110, c1=%19,  idx 180, cyc 1
	mov dword ptr [rbp - 64], r14d         #spill b3=%15, =%-1,  idx 901, cyc -1
	mov r14d, r11d                         #add +=%109, +=%108,  idx 176, cyc 2
	mov r11d, r12d                         #add +=%111, +=%110,  idx 181, cyc 2
	mov r12d, dword ptr [rbp - 24]         #alloc a2=%5, =%-1,  idx 902, cyc -1
	add r14d, r12d                         #add +=%109, a2=%5,  idx 177, cyc 3
	mov dword ptr [rbp - 72], r15d         #spill c0=%17, =%-1,  idx 903, cyc -1
	mov r15d, dword ptr [rbp - 32]         #alloc a3=%7, =%-1,  idx 904, cyc -1
	add r11d, r15d                         #add +=%111, a3=%7,  idx 182, cyc 3
	mov dword ptr [rbp - 80], r10d         #spill c1=%19, =%-1,  idx 905, cyc -1
	mov r10d, r14d                         #assign b2=%13, +=%109,  idx 178, cyc 4
	mov r14d, r11d                         #assign b3=%15, +=%111,  idx 183, cyc 4


.L_b26_while5_body_suffix:
	mov dword ptr [rbp - 24], r12d         #while recover spill a2=%5, =%-1,  idx 906, cyc -1
	mov dword ptr [rbp - 32], r15d         #while recover spill a3=%7, =%-1,  idx 907, cyc -1
	mov dword ptr [rbp - 56], r10d         #while recover spill b2=%13, =%-1,  idx 908, cyc -1
	mov dword ptr [rbp - 64], r14d         #while recover spill b3=%15, =%-1,  idx 909, cyc -1
	mov dword ptr [rbp - 152], r13d        #while recover spill i2=%37, =%-1,  idx 910, cyc -1
	mov r10d, dword ptr [rbp - 8]          #while 4, ld  a0=%1, =%-1,  idx 911, cyc -1
	mov r12d, dword ptr [rbp - 16]         #while 4, ld  a1=%3, =%-1,  idx 912, cyc -1
	mov r15d, dword ptr [rbp - 104]        #while 4, ld  d0=%25, =%-1,  idx 913, cyc -1
	mov r14d, dword ptr [rbp - 112]        #while 4, ld  d1=%27, =%-1,  idx 914, cyc -1
	jmp .L_b6_while5_cond 


.L_b27_while5_tail:


.L_b28:
	mov r12d, dword ptr [rbp - 88]         #alloc c2=%21, =%-1,  idx 915, cyc -1
	mov r10d, r12d                         #add +=%112, c2=%21,  idx 184, cyc 0
	mov dword ptr [rbp - 88], r12d         #spill c2=%21, =%-1,  idx 916, cyc -1
	mov r12d, dword ptr [rbp - 96]         #alloc c3=%23, =%-1,  idx 917, cyc -1
	mov r13d, r12d                         #add +=%114, c3=%23,  idx 189, cyc 0
	add r10d, r11d                         #add +=%112, d0=%25,  idx 185, cyc 1
	mov dword ptr [rbp - 96], r12d         #spill c3=%23, =%-1,  idx 918, cyc -1
	mov r12d, dword ptr [rbp - 112]        #alloc d1=%27, =%-1,  idx 919, cyc -1
	add r13d, r12d                         #add +=%114, d1=%27,  idx 190, cyc 1
	mov dword ptr [rbp - 104], r11d        #spill d0=%25, =%-1,  idx 920, cyc -1
	mov r11d, r10d                         #add +=%113, +=%112,  idx 186, cyc 2
	mov r10d, r13d                         #add +=%115, +=%114,  idx 191, cyc 2
	mov r13d, dword ptr [rbp - 40]         #alloc b0=%9, =%-1,  idx 921, cyc -1
	add r11d, r13d                         #add +=%113, b0=%9,  idx 187, cyc 3
	mov dword ptr [rbp - 72], r15d         #spill c0=%17, =%-1,  idx 922, cyc -1
	mov r15d, dword ptr [rbp - 48]         #alloc b1=%11, =%-1,  idx 923, cyc -1
	add r10d, r15d                         #add +=%115, b1=%11,  idx 192, cyc 3
	mov dword ptr [rbp - 112], r12d        #spill d1=%27, =%-1,  idx 924, cyc -1
	mov r12d, r11d                         #assign c2=%21, +=%113,  idx 188, cyc 4
	mov r11d, r10d                         #assign c3=%23, +=%115,  idx 193, cyc 4


.L_b31_while2_body_suffix:
	mov dword ptr [rbp - 40], r13d         #while spill, cond-ld st b0=%9, =%-1,  idx 925, cyc -1
	mov dword ptr [rbp - 40], r13d         #while recover spill b0=%9, =%-1,  idx 926, cyc -1
	mov dword ptr [rbp - 48], r15d         #while recover spill b1=%11, =%-1,  idx 927, cyc -1
	mov dword ptr [rbp - 88], r12d         #while recover spill c2=%21, =%-1,  idx 928, cyc -1
	mov dword ptr [rbp - 96], r11d         #while recover spill c3=%23, =%-1,  idx 929, cyc -1
	mov r15d, dword ptr [rbp - 128]        #while 4, ld  d3=%31, =%-1,  idx 930, cyc -1
	mov r12d, dword ptr [rbp - 136]        #while 4, ld  i0=%33, =%-1,  idx 931, cyc -1
	mov r10d, r14d                         #while 2, assign i1=%35 =%-1, =%-1,  idx 0, cyc -1
	mov r14d, dword ptr [rbp - 152]        #while 4, ld  i2=%37, =%-1,  idx 932, cyc -1
	mov r11d, dword ptr [rbp - 160]        #while 4, ld  i3=%39, =%-1,  idx 933, cyc -1
	jmp .L_b3_while2_cond 


.L_b32_while2_tail:


.L_b33:
	mov r10d, r11d                         #add +=%116, a0=%1,  idx 194, cyc 0
	mov r11d, dword ptr [rbp - 16]         #alloc a1=%3, =%-1,  idx 934, cyc -1
	add r10d, r11d                         #add +=%116, a1=%3,  idx 195, cyc 1
	mov r11d, r10d                         #add +=%117, +=%116,  idx 196, cyc 2
	mov r10d, dword ptr [rbp - 24]         #alloc a2=%5, =%-1,  idx 935, cyc -1
	add r11d, r10d                         #add +=%117, a2=%5,  idx 197, cyc 3
	mov r10d, r11d                         #add +=%118, +=%117,  idx 198, cyc 4
	mov r11d, dword ptr [rbp - 32]         #alloc a3=%7, =%-1,  idx 936, cyc -1
	add r10d, r11d                         #add +=%118, a3=%7,  idx 199, cyc 5
	mov r11d, r10d                         #add +=%119, +=%118,  idx 200, cyc 6
	add r11d, r14d                         #add +=%119, b0=%9,  idx 201, cyc 7
	mov r10d, r11d                         #add +=%120, +=%119,  idx 202, cyc 8
	mov r11d, dword ptr [rbp - 48]         #alloc b1=%11, =%-1,  idx 937, cyc -1
	add r10d, r11d                         #add +=%120, b1=%11,  idx 203, cyc 9
	mov r11d, r10d                         #add +=%121, +=%120,  idx 204, cyc 10
	mov r10d, dword ptr [rbp - 56]         #alloc b2=%13, =%-1,  idx 938, cyc -1
	add r11d, r10d                         #add +=%121, b2=%13,  idx 205, cyc 11
	mov r10d, r11d                         #add +=%122, +=%121,  idx 206, cyc 12
	mov r11d, dword ptr [rbp - 64]         #alloc b3=%15, =%-1,  idx 939, cyc -1
	add r10d, r11d                         #add +=%122, b3=%15,  idx 207, cyc 13
	mov r11d, r10d                         #add +=%123, +=%122,  idx 208, cyc 14
	mov r10d, dword ptr [rbp - 72]         #alloc c0=%17, =%-1,  idx 940, cyc -1
	add r11d, r10d                         #add +=%123, c0=%17,  idx 209, cyc 15
	mov r10d, r11d                         #add +=%124, +=%123,  idx 210, cyc 16
	mov r11d, dword ptr [rbp - 80]         #alloc c1=%19, =%-1,  idx 941, cyc -1
	add r10d, r11d                         #add +=%124, c1=%19,  idx 211, cyc 17
	mov r11d, r10d                         #add +=%125, +=%124,  idx 212, cyc 18
	mov r10d, dword ptr [rbp - 88]         #alloc c2=%21, =%-1,  idx 942, cyc -1
	add r11d, r10d                         #add +=%125, c2=%21,  idx 213, cyc 19
	mov r10d, r11d                         #add +=%126, +=%125,  idx 214, cyc 20
	mov r11d, dword ptr [rbp - 96]         #alloc c3=%23, =%-1,  idx 943, cyc -1
	add r10d, r11d                         #add +=%126, c3=%23,  idx 215, cyc 21
	mov r11d, r10d                         #add +=%127, +=%126,  idx 216, cyc 22
	mov r10d, dword ptr [rbp - 104]        #alloc d0=%25, =%-1,  idx 944, cyc -1
	add r11d, r10d                         #add +=%127, d0=%25,  idx 217, cyc 23
	mov r10d, r11d                         #add +=%128, +=%127,  idx 218, cyc 24
	mov r11d, dword ptr [rbp - 112]        #alloc d1=%27, =%-1,  idx 945, cyc -1
	add r10d, r11d                         #add +=%128, d1=%27,  idx 219, cyc 25
	mov r11d, r10d                         #add +=%129, +=%128,  idx 220, cyc 26
	mov r10d, dword ptr [rbp - 120]        #alloc d2=%29, =%-1,  idx 946, cyc -1
	add r11d, r10d                         #add +=%129, d2=%29,  idx 221, cyc 27
	mov r10d, r11d                         #add +=%130, +=%129,  idx 222, cyc 28
	mov r11d, dword ptr [rbp - 128]        #alloc d3=%31, =%-1,  idx 947, cyc -1
	add r10d, r11d                         #add +=%130, d3=%31,  idx 223, cyc 29
	mov r11d, r10d                         #add +=%131, +=%130,  idx 224, cyc 30
	add r11d, r12d                         #add +=%131, i0=%33,  idx 225, cyc 31
	mov r10d, r11d                         #add +=%132, +=%131,  idx 226, cyc 32
	mov r11d, dword ptr [rbp - 144]        #alloc i1=%35, =%-1,  idx 948, cyc -1
	add r10d, r11d                         #add +=%132, i1=%35,  idx 227, cyc 33
	mov r11d, r10d                         #add +=%133, +=%132,  idx 228, cyc 34
	mov r10d, dword ptr [rbp - 152]        #alloc i2=%37, =%-1,  idx 949, cyc -1
	add r11d, r10d                         #add +=%133, i2=%37,  idx 229, cyc 35
	mov r10d, r11d                         #add +=%134, +=%133,  idx 230, cyc 36
	mov r11d, dword ptr [rbp - 160]        #alloc i3=%39, =%-1,  idx 950, cyc -1
	add r10d, r11d                         #add +=%134, i3=%39,  idx 231, cyc 37
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





