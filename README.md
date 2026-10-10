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
	int a1 = 3;
	int a2 = 5;
	int a3 = 7;
	int b0 = 2;
	int b1 = 4;
	int b2 = 6;
	int b3 = 8;
	int c0 = 9;
	int c1 = 11;
	int c2 = 13;
	int c3 = 15;
	int d0 = 10;
	int d1 = 12;
	int d2 = 14;
	int d3 = 16;
	int skip = 1;
	int carry = 3;

	while ((skip = skip - 1) > 0)
	{
		if (carry > 0)
		{
			a0 = a0 + b0;
			b0 = b0 + c0;
		}
		else
		{
			c0 = c0 + d0;
			d0 = d0 + a0;
		}
	}

	a0 = (a0 + skip) / 2 + carry;
	c0 = (c0 + skip) / 2 + a0;

	int p = 6;

	while ((p = p - 1) > 0)
	{
		if (p > 2)
		{
			a1 = (a1 + b1) / 2 + p;
			c1 = (c1 + d1) / 2 + a1;
			int q = 5;
			while ((q = q - 1) > 0)
			{
				if (q > 2)
				{
					b2 = (b2 + c2) / 2 + q;
					d2 = (d2 + a2) / 2 + p;
				}
				else
				{
					c2 = (c2 + a3) / 2 + q;
					b1 = (b1 + d3) / 2 + p;
				}
			}
		}
		else
		{
			a3 = (a3 + c3) / 2 + p;
			b3 = (b3 + d3) / 2 + p;
			if (a3 > b3)
			{
				c3 = (c3 + a3) / 2 + carry;
				d3 = (d3 + b3) / 2 + p;
			}
			else
			{
				c2 = (c2 + b3) / 2 + carry;
				d2 = (d2 + a3) / 2 + p;
			}
		}

		carry = carry + 1;
	}

	int s = 5;
	while ((s = s - 1) > 0)
	{
		if (s > 2)
		{
			d0 = (d0 + c3) / 2 + s;
			a0 = (a0 + b3) / 2 + s;
		}
		else
		{
			d1 = (d1 + c2) / 2 + s;
			a1 = (a1 + b2) / 2 + s;
		}
	}

	return a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3
	    + c0 + c1 + c2 + c3 + d0 + d1 + d2 + d3 + skip + p + s + carry;
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
	sub rsp, 656 

	mov r10d, 1                            #li 1=%0, =%-1,  cyc 0
	mov r11d, 3                            #li 3=%2, =%-1,  cyc 0
	mov r12d, r10d                         #assign a0=%1, 1=%0,  cyc 1
	#victim %0, r10d                       #victim check_dead_vr 1=%0, =%-1,  cyc -1
	mov r10d, r11d                         #assign a1=%3, 3=%2,  cyc 1
	#victim %2, r11d                       #victim check_dead_vr 3=%2, =%-1,  cyc -1
	mov r11d, 5                            #li 5=%4, =%-1,  cyc 2
	mov r13d, 7                            #li 7=%6, =%-1,  cyc 2
	mov r14d, r11d                         #assign a2=%5, 5=%4,  cyc 3
	#victim %4, r11d                       #victim check_dead_vr 5=%4, =%-1,  cyc -1
	mov r11d, r13d                         #assign a3=%7, 7=%6,  cyc 3
	#victim %6, r13d                       #victim check_dead_vr 7=%6, =%-1,  cyc -1
	mov r13d, 2                            #li 2=%8, =%-1,  cyc 4
	mov r15d, 4                            #li 4=%10, =%-1,  cyc 4
	#victim %1, r12d                       #victim a0=%1, =%-1,  cyc -1
	mov dword ptr [rbp - 8], r12d          #spill a0=%1, =%-1,  cyc -1
	mov r12d, r13d                         #assign b0=%9, 2=%8,  cyc 5
	#victim %8, r13d                       #victim check_dead_vr 2=%8, =%-1,  cyc -1
	mov r13d, r15d                         #assign b1=%11, 4=%10,  cyc 5
	#victim %10, r15d                      #victim check_dead_vr 4=%10, =%-1,  cyc -1
	mov r15d, 6                            #li 6=%12, =%-1,  cyc 6
	#victim %3, r10d                       #victim a1=%3, =%-1,  cyc -1
	mov dword ptr [rbp - 16], r10d         #spill a1=%3, =%-1,  cyc -1
	mov r10d, 8                            #li 8=%14, =%-1,  cyc 6
	#victim %5, r14d                       #victim a2=%5, =%-1,  cyc -1
	mov dword ptr [rbp - 24], r14d         #spill a2=%5, =%-1,  cyc -1
	mov r14d, r15d                         #assign b2=%13, 6=%12,  cyc 7
	#victim %12, r15d                      #victim check_dead_vr 6=%12, =%-1,  cyc -1
	mov r15d, r10d                         #assign b3=%15, 8=%14,  cyc 7
	#victim %14, r10d                      #victim check_dead_vr 8=%14, =%-1,  cyc -1
	mov r10d, 9                            #li 9=%16, =%-1,  cyc 8
	#victim %7, r11d                       #victim a3=%7, =%-1,  cyc -1
	mov dword ptr [rbp - 32], r11d         #spill a3=%7, =%-1,  cyc -1
	mov r11d, 11                           #li 11=%18, =%-1,  cyc 8
	#victim %9, r12d                       #victim b0=%9, =%-1,  cyc -1
	mov dword ptr [rbp - 40], r12d         #spill b0=%9, =%-1,  cyc -1
	mov r12d, r10d                         #assign c0=%17, 9=%16,  cyc 9
	#victim %16, r10d                      #victim check_dead_vr 9=%16, =%-1,  cyc -1
	mov r10d, r11d                         #assign c1=%19, 11=%18,  cyc 9
	#victim %18, r11d                      #victim check_dead_vr 11=%18, =%-1,  cyc -1
	mov r11d, 13                           #li 13=%20, =%-1,  cyc 10
	#victim %11, r13d                      #victim b1=%11, =%-1,  cyc -1
	mov dword ptr [rbp - 48], r13d         #spill b1=%11, =%-1,  cyc -1
	mov r13d, 15                           #li 15=%22, =%-1,  cyc 10
	#victim %13, r14d                      #victim b2=%13, =%-1,  cyc -1
	mov dword ptr [rbp - 56], r14d         #spill b2=%13, =%-1,  cyc -1
	mov r14d, r11d                         #assign c2=%21, 13=%20,  cyc 11
	#victim %20, r11d                      #victim check_dead_vr 13=%20, =%-1,  cyc -1
	mov r11d, r13d                         #assign c3=%23, 15=%22,  cyc 11
	#victim %22, r13d                      #victim check_dead_vr 15=%22, =%-1,  cyc -1
	mov r13d, 10                           #li 10=%24, =%-1,  cyc 12
	#victim %15, r15d                      #victim b3=%15, =%-1,  cyc -1
	mov dword ptr [rbp - 64], r15d         #spill b3=%15, =%-1,  cyc -1
	mov r15d, 12                           #li 12=%26, =%-1,  cyc 12
	#victim %17, r12d                      #victim c0=%17, =%-1,  cyc -1
	mov dword ptr [rbp - 72], r12d         #spill c0=%17, =%-1,  cyc -1
	mov r12d, r13d                         #assign d0=%25, 10=%24,  cyc 13
	#victim %24, r13d                      #victim check_dead_vr 10=%24, =%-1,  cyc -1
	mov r13d, r15d                         #assign d1=%27, 12=%26,  cyc 13
	#victim %26, r15d                      #victim check_dead_vr 12=%26, =%-1,  cyc -1
	mov r15d, 14                           #li 14=%28, =%-1,  cyc 14
	#victim %19, r10d                      #victim c1=%19, =%-1,  cyc -1
	mov dword ptr [rbp - 80], r10d         #spill c1=%19, =%-1,  cyc -1
	mov r10d, 16                           #li 16=%30, =%-1,  cyc 14
	#victim %21, r14d                      #victim c2=%21, =%-1,  cyc -1
	mov dword ptr [rbp - 88], r14d         #spill c2=%21, =%-1,  cyc -1
	mov r14d, r15d                         #assign d2=%29, 14=%28,  cyc 15
	#victim %28, r15d                      #victim check_dead_vr 14=%28, =%-1,  cyc -1
	mov r15d, r10d                         #assign d3=%31, 16=%30,  cyc 15
	#victim %30, r10d                      #victim check_dead_vr 16=%30, =%-1,  cyc -1
	mov r10d, 1                            #li 1=%32, =%-1,  cyc 16
	#victim %23, r11d                      #victim c3=%23, =%-1,  cyc -1
	mov dword ptr [rbp - 96], r11d         #spill c3=%23, =%-1,  cyc -1
	mov r11d, 3                            #li 3=%34, =%-1,  cyc 16
	#victim %25, r12d                      #victim d0=%25, =%-1,  cyc -1
	mov dword ptr [rbp - 104], r12d        #spill d0=%25, =%-1,  cyc -1
	mov r12d, r10d                         #assign skip=%33, 1=%32,  cyc 17
	#victim %32, r10d                      #victim check_dead_vr 1=%32, =%-1,  cyc -1
	mov r10d, r11d                         #assign carry=%35, 3=%34,  cyc 17
	#victim %34, r11d                      #victim check_dead_vr 3=%34, =%-1,  cyc -1


.L_b2_while:


.L_b3_while2_cond:
	mov r11d, 1                            #li 1=%37, =%-1,  cyc 0
	#victim %35, r10d                      #victim carry=%35, =%-1,  cyc -1
	mov dword ptr [rbp - 144], r10d        #spill carry=%35, =%-1,  cyc -1
	mov r10d, 0                            #li 0=%36, =%-1,  cyc 0
	#victim %27, r13d                      #victim d1=%27, =%-1,  cyc -1
	mov dword ptr [rbp - 112], r13d        #spill d1=%27, =%-1,  cyc -1
	mov r13d, r12d                         #sub -=%38, skip=%33,  cyc 1
	sub r13d, r11d                         #sub -=%38, 1=%37,  cyc 2
	#victim %37, r11d                      #victim check_dead_vr 1=%37, =%-1,  cyc -1
	mov r12d, r13d                         #assign skip=%33, -=%38,  cyc 3
	#victim %38, r13d                      #victim check_dead_vr -=%38, =%-1,  cyc -1
	cmp r12d, r10d                         #cmple skip=%33, 0=%36,  cyc 4
	setle r11b                             #cmple skip=%33, 0=%36,  cyc 4
	movzx r11d, r11b                       #cmple skip=%33, 0=%36,  cyc 4
	#victim %39, r11d                      #victim check_dead_vr >=%39, =%-1,  cyc -1
	#victim %36, r10d                      #victim check_dead_vr 0=%36, =%-1,  cyc -1
	jle .L_b15_while2_tail 


.L_b4_while2_body:


.L_b6_if5_cond:
	mov r10d, 0                            #li 0=%40, =%-1,  cyc 0
	mov r13d, dword ptr [rbp - 144]        #ld carry=%35, =%-1,  cyc -1
	cmp r13d, r10d                         #cmple carry=%35, 0=%40,  cyc 1
	setle r11b                             #cmple carry=%35, 0=%40,  cyc 1
	movzx r11d, r11b                       #cmple carry=%35, 0=%40,  cyc 1
	#victim %41, r11d                      #victim check_dead_vr >=%41, =%-1,  cyc -1
	#victim %40, r10d                      #victim check_dead_vr 0=%40, =%-1,  cyc -1
	#victim %33, r12d                      #victim skip=%33, =%-1,  cyc -1
	mov dword ptr [rbp - 136], r12d        #spill skip=%33, =%-1,  cyc -1
	#victim %35, r13d                      #victim carry=%35, =%-1,  cyc -1
	#victim %29, r14d                      #victim d2=%29, =%-1,  cyc -1
	mov dword ptr [rbp - 120], r14d        #spill d2=%29, =%-1,  cyc -1
	#victim %31, r15d                      #victim d3=%31, =%-1,  cyc -1
	mov dword ptr [rbp - 128], r15d        #spill d3=%31, =%-1,  cyc -1
	jle .L_b9_if5_else 


.L_b7_if5_then:
	mov r11d, dword ptr [rbp - 8]          #ld a0=%1, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%42, a0=%1,  cyc 0
	mov r13d, dword ptr [rbp - 40]         #ld b0=%9, =%-1,  cyc -1
	mov r12d, r13d                         #add +=%43, b0=%9,  cyc 0
	add r10d, r13d                         #add +=%42, b0=%9,  cyc 1
	mov r14d, dword ptr [rbp - 72]         #ld c0=%17, =%-1,  cyc -1
	add r12d, r14d                         #add +=%43, c0=%17,  cyc 1
	mov r11d, r10d                         #assign a0=%1, +=%42,  cyc 2
	#victim %42, r10d                      #victim check_dead_vr +=%42, =%-1,  cyc -1
	mov r13d, r12d                         #assign b0=%9, +=%43,  cyc 2
	#victim %43, r12d                      #victim check_dead_vr +=%43, =%-1,  cyc -1


.L_b8:
	#victim %1, r11d                       #victim a0=%1, =%-1,  cyc -1
	mov dword ptr [rbp - 8], r11d          #spill a0=%1, =%-1,  cyc -1
	#victim %9, r13d                       #victim b0=%9, =%-1,  cyc -1
	mov dword ptr [rbp - 40], r13d         #spill b0=%9, =%-1,  cyc -1
	#victim %17, r14d                      #victim c0=%17, =%-1,  cyc -1
	jmp .L_b11_if5_tail 


.L_b9_if5_else:
	mov r11d, dword ptr [rbp - 72]         #ld c0=%17, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%44, c0=%17,  cyc 0
	mov r13d, dword ptr [rbp - 104]        #ld d0=%25, =%-1,  cyc -1
	mov r12d, r13d                         #add +=%45, d0=%25,  cyc 0
	add r10d, r13d                         #add +=%44, d0=%25,  cyc 1
	mov r14d, dword ptr [rbp - 8]          #ld a0=%1, =%-1,  cyc -1
	add r12d, r14d                         #add +=%45, a0=%1,  cyc 1
	mov r11d, r10d                         #assign c0=%17, +=%44,  cyc 2
	#victim %44, r10d                      #victim check_dead_vr +=%44, =%-1,  cyc -1
	mov r13d, r12d                         #assign d0=%25, +=%45,  cyc 2
	#victim %45, r12d                      #victim check_dead_vr +=%45, =%-1,  cyc -1


.L_b10:
	#victim %17, r11d                      #victim c0=%17, =%-1,  cyc -1
	mov dword ptr [rbp - 72], r11d         #spill c0=%17, =%-1,  cyc -1
	#victim %25, r13d                      #victim d0=%25, =%-1,  cyc -1
	mov dword ptr [rbp - 104], r13d        #spill d0=%25, =%-1,  cyc -1
	#victim %1, r14d                       #victim a0=%1, =%-1,  cyc -1
	jmp .L_b11_if5_tail 


.L_b11_if5_tail:


.L_b13_while2_body_tail:
	mov r13d, dword ptr [rbp - 112]        #while 4, ld  d1=%27, =%-1,  cyc -1
	mov r14d, dword ptr [rbp - 120]        #while 4, ld  d2=%29, =%-1,  cyc -1
	mov r15d, dword ptr [rbp - 128]        #while 4, ld  d3=%31, =%-1,  cyc -1
	mov r12d, dword ptr [rbp - 136]        #while 4, ld  skip=%33, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 144]        #while 4, ld  carry=%35, =%-1,  cyc -1
	jmp .L_b3_while2_cond 


.L_b15_while2_tail:


.L_b16:
	mov r10d, 2                            #li 2=%46, =%-1,  cyc 0
	mov r13d, dword ptr [rbp - 72]         #ld c0=%17, =%-1,  cyc -1
	mov r11d, r13d                         #add +=%51, c0=%17,  cyc 0
	add r11d, r12d                         #add +=%51, skip=%33,  cyc 1
	#victim %29, r14d                      #victim d2=%29, =%-1,  cyc -1
	mov dword ptr [rbp - 120], r14d        #spill d2=%29, =%-1,  cyc -1
	mov r14d, 2                            #li 2=%50, =%-1,  cyc 1
	#victim %31, r15d                      #victim d3=%31, =%-1,  cyc -1
	mov dword ptr [rbp - 128], r15d        #spill d3=%31, =%-1,  cyc -1
	mov r15d, r11d                         #div /=%52, +=%51,  cyc 2
	#victim %51, r11d                      #victim check_dead_vr +=%51, =%-1,  cyc -1
	#victim %46, r10d                      #victim 2=%46, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 8]          #ld a0=%1, =%-1,  cyc -1
	mov r11d, r10d                         #add +=%47, a0=%1,  cyc 2
	mov eax, r15d 
	cdq 
	idiv r14d 
	mov r15d, eax                          #div /=%52, 2=%50,  cyc 3
	#victim %50, r14d                      #victim check_dead_vr 2=%50, =%-1,  cyc -1
	add r11d, r12d                         #add +=%47, skip=%33,  cyc 3
	mov r14d, 6                            #li 6=%54, =%-1,  cyc 3
	#victim %17, r13d                      #victim c0=%17, =%-1,  cyc -1
	mov r13d, r11d                         #div /=%48, +=%47,  cyc 4
	#victim %47, r11d                      #victim check_dead_vr +=%47, =%-1,  cyc -1
	mov r11d, r14d                         #assign p=%55, 6=%54,  cyc 4
	#victim %54, r14d                      #victim check_dead_vr 6=%54, =%-1,  cyc -1
	mov r14d, 2                            #li 2=%46, =%-1,  cyc -1
	mov eax, r13d 
	cdq 
	idiv r14d 
	mov r13d, eax                          #div /=%48, 2=%46,  cyc 13
	#victim %46, r14d                      #victim check_dead_vr 2=%46, =%-1,  cyc -1
	mov r14d, r15d                         #add +=%53, /=%52,  cyc 13
	#victim %52, r15d                      #victim check_dead_vr /=%52, =%-1,  cyc -1
	mov r15d, r13d                         #add +=%49, /=%48,  cyc 23
	#victim %48, r13d                      #victim check_dead_vr /=%48, =%-1,  cyc -1
	mov r13d, dword ptr [rbp - 144]        #ld carry=%35, =%-1,  cyc -1
	add r15d, r13d                         #add +=%49, carry=%35,  cyc 24
	mov r10d, r15d                         #assign a0=%1, +=%49,  cyc 25
	#victim %49, r15d                      #victim check_dead_vr +=%49, =%-1,  cyc -1
	add r14d, r10d                         #add +=%53, a0=%1,  cyc 26
	mov r15d, r14d                         #assign c0=%17, +=%53,  cyc 27
	#victim %53, r14d                      #victim check_dead_vr +=%53, =%-1,  cyc -1


.L_b17_while:


.L_b18_while17_cond:
	mov r14d, 1                            #li 1=%57, =%-1,  cyc 0
	#victim %1, r10d                       #victim a0=%1, =%-1,  cyc -1
	mov dword ptr [rbp - 8], r10d          #spill a0=%1, =%-1,  cyc -1
	mov r10d, 0                            #li 0=%56, =%-1,  cyc 0
	#victim %33, r12d                      #victim skip=%33, =%-1,  cyc -1
	mov dword ptr [rbp - 136], r12d        #spill skip=%33, =%-1,  cyc -1
	mov r12d, r11d                         #sub -=%58, p=%55,  cyc 1
	sub r12d, r14d                         #sub -=%58, 1=%57,  cyc 2
	#victim %57, r14d                      #victim check_dead_vr 1=%57, =%-1,  cyc -1
	mov r11d, r12d                         #assign p=%55, -=%58,  cyc 3
	#victim %58, r12d                      #victim check_dead_vr -=%58, =%-1,  cyc -1
	cmp r11d, r10d                         #cmple p=%55, 0=%56,  cyc 4
	setle r12b                             #cmple p=%55, 0=%56,  cyc 4
	movzx r12d, r12b                       #cmple p=%55, 0=%56,  cyc 4
	#victim %59, r12d                      #victim check_dead_vr >=%59, =%-1,  cyc -1
	#victim %56, r10d                      #victim check_dead_vr 0=%56, =%-1,  cyc -1
	jle .L_b53_while17_tail 


.L_b19_while17_body:


.L_b21_if20_cond:
	mov r10d, 2                            #li 2=%60, =%-1,  cyc 0
	cmp r11d, r10d                         #cmple p=%55, 2=%60,  cyc 1
	setle r12b                             #cmple p=%55, 2=%60,  cyc 1
	movzx r12d, r12b                       #cmple p=%55, 2=%60,  cyc 1
	#victim %61, r12d                      #victim check_dead_vr >=%61, =%-1,  cyc -1
	#victim %60, r10d                      #victim check_dead_vr 2=%60, =%-1,  cyc -1
	#victim %55, r11d                      #victim p=%55, =%-1,  cyc -1
	mov dword ptr [rbp - 224], r11d        #spill p=%55, =%-1,  cyc -1
	#victim %35, r13d                      #victim carry=%35, =%-1,  cyc -1
	#victim %17, r15d                      #victim c0=%17, =%-1,  cyc -1
	mov dword ptr [rbp - 72], r15d         #spill c0=%17, =%-1,  cyc -1
	jle .L_b38_if20_else 


.L_b22_if20_then:
	mov r10d, 2                            #li 2=%62, =%-1,  cyc 0
	mov r12d, dword ptr [rbp - 80]         #ld c1=%19, =%-1,  cyc -1
	mov r11d, r12d                         #add +=%67, c1=%19,  cyc 0
	mov r13d, dword ptr [rbp - 112]        #ld d1=%27, =%-1,  cyc -1
	add r11d, r13d                         #add +=%67, d1=%27,  cyc 1
	mov r14d, 2                            #li 2=%66, =%-1,  cyc 1
	mov r15d, r11d                         #div /=%68, +=%67,  cyc 2
	#victim %67, r11d                      #victim check_dead_vr +=%67, =%-1,  cyc -1
	#victim %62, r10d                      #victim 2=%62, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 16]         #ld a1=%3, =%-1,  cyc -1
	mov r11d, r10d                         #add +=%63, a1=%3,  cyc 2
	mov eax, r15d 
	cdq 
	idiv r14d 
	mov r15d, eax                          #div /=%68, 2=%66,  cyc 3
	#victim %66, r14d                      #victim check_dead_vr 2=%66, =%-1,  cyc -1
	mov r14d, dword ptr [rbp - 48]         #ld b1=%11, =%-1,  cyc -1
	add r11d, r14d                         #add +=%63, b1=%11,  cyc 3
	#victim %19, r12d                      #victim c1=%19, =%-1,  cyc -1
	mov r12d, 5                            #li 5=%70, =%-1,  cyc 3
	#victim %27, r13d                      #victim d1=%27, =%-1,  cyc -1
	mov r13d, r11d                         #div /=%64, +=%63,  cyc 4
	#victim %63, r11d                      #victim check_dead_vr +=%63, =%-1,  cyc -1
	mov r11d, r12d                         #assign q=%71, 5=%70,  cyc 4
	#victim %70, r12d                      #victim check_dead_vr 5=%70, =%-1,  cyc -1
	mov r12d, 2                            #li 2=%62, =%-1,  cyc -1
	mov eax, r13d 
	cdq 
	idiv r12d 
	mov r13d, eax                          #div /=%64, 2=%62,  cyc 13
	#victim %62, r12d                      #victim check_dead_vr 2=%62, =%-1,  cyc -1
	mov r12d, r15d                         #add +=%69, /=%68,  cyc 13
	#victim %68, r15d                      #victim check_dead_vr /=%68, =%-1,  cyc -1
	mov r15d, r13d                         #add +=%65, /=%64,  cyc 23
	#victim %64, r13d                      #victim check_dead_vr /=%64, =%-1,  cyc -1
	mov r13d, dword ptr [rbp - 224]        #ld p=%55, =%-1,  cyc -1
	add r15d, r13d                         #add +=%65, p=%55,  cyc 24
	mov r10d, r15d                         #assign a1=%3, +=%65,  cyc 25
	#victim %65, r15d                      #victim check_dead_vr +=%65, =%-1,  cyc -1
	add r12d, r10d                         #add +=%69, a1=%3,  cyc 26
	mov r15d, r12d                         #assign c1=%19, +=%69,  cyc 27
	#victim %69, r12d                      #victim check_dead_vr +=%69, =%-1,  cyc -1


.L_b23_while:


.L_b24_while23_cond:
	mov r12d, 1                            #li 1=%73, =%-1,  cyc 0
	#victim %3, r10d                       #victim a1=%3, =%-1,  cyc -1
	mov dword ptr [rbp - 16], r10d         #spill a1=%3, =%-1,  cyc -1
	mov r10d, 0                            #li 0=%72, =%-1,  cyc 0
	#victim %55, r13d                      #victim p=%55, =%-1,  cyc -1
	mov r13d, r11d                         #sub -=%74, q=%71,  cyc 1
	sub r13d, r12d                         #sub -=%74, 1=%73,  cyc 2
	#victim %73, r12d                      #victim check_dead_vr 1=%73, =%-1,  cyc -1
	mov r11d, r13d                         #assign q=%71, -=%74,  cyc 3
	#victim %74, r13d                      #victim check_dead_vr -=%74, =%-1,  cyc -1
	cmp r11d, r10d                         #cmple q=%71, 0=%72,  cyc 4
	setle r12b                             #cmple q=%71, 0=%72,  cyc 4
	movzx r12d, r12b                       #cmple q=%71, 0=%72,  cyc 4
	#victim %75, r12d                      #victim check_dead_vr >=%75, =%-1,  cyc -1
	#victim %72, r10d                      #victim check_dead_vr 0=%72, =%-1,  cyc -1
	jle .L_b35_while23_tail 


.L_b25_while23_body:


.L_b27_if26_cond:
	mov r10d, 2                            #li 2=%76, =%-1,  cyc 0
	cmp r11d, r10d                         #cmple q=%71, 2=%76,  cyc 1
	setle r12b                             #cmple q=%71, 2=%76,  cyc 1
	movzx r12d, r12b                       #cmple q=%71, 2=%76,  cyc 1
	#victim %77, r12d                      #victim check_dead_vr >=%77, =%-1,  cyc -1
	#victim %76, r10d                      #victim check_dead_vr 2=%76, =%-1,  cyc -1
	#victim %71, r11d                      #victim q=%71, =%-1,  cyc -1
	mov dword ptr [rbp - 288], r11d        #spill q=%71, =%-1,  cyc -1
	#victim %11, r14d                      #victim b1=%11, =%-1,  cyc -1
	#victim %19, r15d                      #victim c1=%19, =%-1,  cyc -1
	mov dword ptr [rbp - 80], r15d         #spill c1=%19, =%-1,  cyc -1
	jle .L_b30_if26_else 


.L_b28_if26_then:
	mov r11d, dword ptr [rbp - 56]         #ld b2=%13, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%79, b2=%13,  cyc 0
	mov r13d, dword ptr [rbp - 120]        #ld d2=%29, =%-1,  cyc -1
	mov r12d, r13d                         #add +=%83, d2=%29,  cyc 0
	mov r14d, dword ptr [rbp - 88]         #ld c2=%21, =%-1,  cyc -1
	add r10d, r14d                         #add +=%79, c2=%21,  cyc 1
	mov r15d, dword ptr [rbp - 24]         #ld a2=%5, =%-1,  cyc -1
	add r12d, r15d                         #add +=%83, a2=%5,  cyc 1
	#victim %13, r11d                      #victim b2=%13, =%-1,  cyc -1
	mov r11d, r10d                         #div /=%80, +=%79,  cyc 2
	#victim %79, r10d                      #victim check_dead_vr +=%79, =%-1,  cyc -1
	mov r10d, r12d                         #div /=%84, +=%83,  cyc 2
	#victim %83, r12d                      #victim check_dead_vr +=%83, =%-1,  cyc -1
	mov r12d, 2                            #li 2=%78, =%-1,  cyc 3
	#victim %29, r13d                      #victim d2=%29, =%-1,  cyc -1
	mov r13d, 2                            #li 2=%82, =%-1,  cyc 3
	mov eax, r11d 
	cdq 
	idiv r12d 
	mov r11d, eax                          #div /=%80, 2=%78,  cyc 4
	#victim %78, r12d                      #victim check_dead_vr 2=%78, =%-1,  cyc -1
	mov eax, r10d 
	cdq 
	idiv r13d 
	mov r10d, eax                          #div /=%84, 2=%82,  cyc 14
	#victim %82, r13d                      #victim check_dead_vr 2=%82, =%-1,  cyc -1
	mov r12d, r11d                         #add +=%81, /=%80,  cyc 14
	#victim %80, r11d                      #victim check_dead_vr /=%80, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 288]        #ld q=%71, =%-1,  cyc -1
	add r12d, r11d                         #add +=%81, q=%71,  cyc 15
	mov r13d, r12d                         #assign b2=%13, +=%81,  cyc 16
	#victim %81, r12d                      #victim check_dead_vr +=%81, =%-1,  cyc -1
	mov r12d, r10d                         #add +=%85, /=%84,  cyc 24
	#victim %84, r10d                      #victim check_dead_vr /=%84, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 224]        #ld p=%55, =%-1,  cyc -1
	add r12d, r10d                         #add +=%85, p=%55,  cyc 25
	#victim %21, r14d                      #victim c2=%21, =%-1,  cyc -1
	mov r14d, r12d                         #assign d2=%29, +=%85,  cyc 26
	#victim %85, r12d                      #victim check_dead_vr +=%85, =%-1,  cyc -1


.L_b29:
	#victim %55, r10d                      #victim p=%55, =%-1,  cyc -1
	#victim %71, r11d                      #victim q=%71, =%-1,  cyc -1
	#victim %13, r13d                      #victim b2=%13, =%-1,  cyc -1
	mov dword ptr [rbp - 56], r13d         #spill b2=%13, =%-1,  cyc -1
	#victim %29, r14d                      #victim d2=%29, =%-1,  cyc -1
	mov dword ptr [rbp - 120], r14d        #spill d2=%29, =%-1,  cyc -1
	#victim %5, r15d                       #victim a2=%5, =%-1,  cyc -1
	jmp .L_b32_if26_tail 


.L_b30_if26_else:
	mov r11d, dword ptr [rbp - 88]         #ld c2=%21, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%87, c2=%21,  cyc 0
	mov r13d, dword ptr [rbp - 48]         #ld b1=%11, =%-1,  cyc -1
	mov r12d, r13d                         #add +=%91, b1=%11,  cyc 0
	mov r14d, dword ptr [rbp - 32]         #ld a3=%7, =%-1,  cyc -1
	add r10d, r14d                         #add +=%87, a3=%7,  cyc 1
	mov r15d, dword ptr [rbp - 128]        #ld d3=%31, =%-1,  cyc -1
	add r12d, r15d                         #add +=%91, d3=%31,  cyc 1
	#victim %21, r11d                      #victim c2=%21, =%-1,  cyc -1
	mov r11d, r10d                         #div /=%88, +=%87,  cyc 2
	#victim %87, r10d                      #victim check_dead_vr +=%87, =%-1,  cyc -1
	mov r10d, r12d                         #div /=%92, +=%91,  cyc 2
	#victim %91, r12d                      #victim check_dead_vr +=%91, =%-1,  cyc -1
	mov r12d, 2                            #li 2=%86, =%-1,  cyc 3
	#victim %11, r13d                      #victim b1=%11, =%-1,  cyc -1
	mov r13d, 2                            #li 2=%90, =%-1,  cyc 3
	mov eax, r11d 
	cdq 
	idiv r12d 
	mov r11d, eax                          #div /=%88, 2=%86,  cyc 4
	#victim %86, r12d                      #victim check_dead_vr 2=%86, =%-1,  cyc -1
	mov eax, r10d 
	cdq 
	idiv r13d 
	mov r10d, eax                          #div /=%92, 2=%90,  cyc 14
	#victim %90, r13d                      #victim check_dead_vr 2=%90, =%-1,  cyc -1
	mov r12d, r11d                         #add +=%89, /=%88,  cyc 14
	#victim %88, r11d                      #victim check_dead_vr /=%88, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 288]        #ld q=%71, =%-1,  cyc -1
	add r12d, r11d                         #add +=%89, q=%71,  cyc 15
	mov r13d, r12d                         #assign c2=%21, +=%89,  cyc 16
	#victim %89, r12d                      #victim check_dead_vr +=%89, =%-1,  cyc -1
	mov r12d, r10d                         #add +=%93, /=%92,  cyc 24
	#victim %92, r10d                      #victim check_dead_vr /=%92, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 224]        #ld p=%55, =%-1,  cyc -1
	add r12d, r10d                         #add +=%93, p=%55,  cyc 25
	#victim %7, r14d                       #victim a3=%7, =%-1,  cyc -1
	mov r14d, r12d                         #assign b1=%11, +=%93,  cyc 26
	#victim %93, r12d                      #victim check_dead_vr +=%93, =%-1,  cyc -1


.L_b31:
	#victim %55, r10d                      #victim p=%55, =%-1,  cyc -1
	#victim %71, r11d                      #victim q=%71, =%-1,  cyc -1
	#victim %21, r13d                      #victim c2=%21, =%-1,  cyc -1
	mov dword ptr [rbp - 88], r13d         #spill c2=%21, =%-1,  cyc -1
	#victim %11, r14d                      #victim b1=%11, =%-1,  cyc -1
	mov dword ptr [rbp - 48], r14d         #spill b1=%11, =%-1,  cyc -1
	#victim %31, r15d                      #victim d3=%31, =%-1,  cyc -1
	jmp .L_b32_if26_tail 


.L_b32_if26_tail:


.L_b34_while23_body_tail:
	mov r10d, dword ptr [rbp - 16]         #while 4, ld  a1=%3, =%-1,  cyc -1
	mov r14d, dword ptr [rbp - 48]         #while 4, ld  b1=%11, =%-1,  cyc -1
	mov r15d, dword ptr [rbp - 80]         #while 4, ld  c1=%19, =%-1,  cyc -1
	mov r13d, dword ptr [rbp - 224]        #while 4, ld  p=%55, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 288]        #while 4, ld  q=%71, =%-1,  cyc -1
	jmp .L_b24_while23_cond 


.L_b35_while23_tail:
	#victim %71, r11d                      #victim check_dead_vr q=%71, =%-1,  cyc -1


.L_b37:
	#victim %11, r14d                      #victim b1=%11, =%-1,  cyc -1
	#victim %19, r15d                      #victim c1=%19, =%-1,  cyc -1
	mov dword ptr [rbp - 80], r15d         #spill c1=%19, =%-1,  cyc -1
	jmp .L_b49_if20_tail 


.L_b38_if20_else:
	mov r11d, dword ptr [rbp - 32]         #ld a3=%7, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%95, a3=%7,  cyc 0
	mov r13d, dword ptr [rbp - 64]         #ld b3=%15, =%-1,  cyc -1
	mov r12d, r13d                         #add +=%99, b3=%15,  cyc 0
	mov r14d, dword ptr [rbp - 96]         #ld c3=%23, =%-1,  cyc -1
	add r10d, r14d                         #add +=%95, c3=%23,  cyc 1
	mov r15d, dword ptr [rbp - 128]        #ld d3=%31, =%-1,  cyc -1
	add r12d, r15d                         #add +=%99, d3=%31,  cyc 1
	#victim %7, r11d                       #victim a3=%7, =%-1,  cyc -1
	mov r11d, r10d                         #div /=%96, +=%95,  cyc 2
	#victim %95, r10d                      #victim check_dead_vr +=%95, =%-1,  cyc -1
	mov r10d, r12d                         #div /=%100, +=%99,  cyc 2
	#victim %99, r12d                      #victim check_dead_vr +=%99, =%-1,  cyc -1
	mov r12d, 2                            #li 2=%94, =%-1,  cyc 3
	#victim %15, r13d                      #victim b3=%15, =%-1,  cyc -1
	mov r13d, 2                            #li 2=%98, =%-1,  cyc 3
	mov eax, r11d 
	cdq 
	idiv r12d 
	mov r11d, eax                          #div /=%96, 2=%94,  cyc 4
	#victim %94, r12d                      #victim check_dead_vr 2=%94, =%-1,  cyc -1
	mov eax, r10d 
	cdq 
	idiv r13d 
	mov r10d, eax                          #div /=%100, 2=%98,  cyc 14
	#victim %98, r13d                      #victim check_dead_vr 2=%98, =%-1,  cyc -1
	mov r12d, r11d                         #add +=%97, /=%96,  cyc 14
	#victim %96, r11d                      #victim check_dead_vr /=%96, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 224]        #ld p=%55, =%-1,  cyc -1
	add r12d, r11d                         #add +=%97, p=%55,  cyc 15
	mov r13d, r12d                         #assign a3=%7, +=%97,  cyc 16
	#victim %97, r12d                      #victim check_dead_vr +=%97, =%-1,  cyc -1
	mov r12d, r10d                         #add +=%101, /=%100,  cyc 24
	#victim %100, r10d                     #victim check_dead_vr /=%100, =%-1,  cyc -1
	add r12d, r11d                         #add +=%101, p=%55,  cyc 25
	mov r10d, r12d                         #assign b3=%15, +=%101,  cyc 26
	#victim %101, r12d                     #victim check_dead_vr +=%101, =%-1,  cyc -1


.L_b40_if39_cond:
	cmp r13d, r10d                         #cmple a3=%7, b3=%15,  cyc 0
	setle r12b                             #cmple a3=%7, b3=%15,  cyc 0
	movzx r12d, r12b                       #cmple a3=%7, b3=%15,  cyc 0
	#victim %102, r12d                     #victim check_dead_vr >=%102, =%-1,  cyc -1
	#victim %15, r10d                      #victim b3=%15, =%-1,  cyc -1
	mov dword ptr [rbp - 64], r10d         #spill b3=%15, =%-1,  cyc -1
	#victim %55, r11d                      #victim p=%55, =%-1,  cyc -1
	#victim %7, r13d                       #victim a3=%7, =%-1,  cyc -1
	mov dword ptr [rbp - 32], r13d         #spill a3=%7, =%-1,  cyc -1
	#victim %23, r14d                      #victim c3=%23, =%-1,  cyc -1
	#victim %31, r15d                      #victim d3=%31, =%-1,  cyc -1
	jle .L_b43_if39_else 


.L_b41_if39_then:
	mov r11d, dword ptr [rbp - 96]         #ld c3=%23, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%104, c3=%23,  cyc 0
	mov r13d, dword ptr [rbp - 128]        #ld d3=%31, =%-1,  cyc -1
	mov r12d, r13d                         #add +=%108, d3=%31,  cyc 0
	mov r14d, dword ptr [rbp - 32]         #ld a3=%7, =%-1,  cyc -1
	add r10d, r14d                         #add +=%104, a3=%7,  cyc 1
	mov r15d, dword ptr [rbp - 64]         #ld b3=%15, =%-1,  cyc -1
	add r12d, r15d                         #add +=%108, b3=%15,  cyc 1
	#victim %23, r11d                      #victim c3=%23, =%-1,  cyc -1
	mov r11d, r10d                         #div /=%105, +=%104,  cyc 2
	#victim %104, r10d                     #victim check_dead_vr +=%104, =%-1,  cyc -1
	mov r10d, r12d                         #div /=%109, +=%108,  cyc 2
	#victim %108, r12d                     #victim check_dead_vr +=%108, =%-1,  cyc -1
	mov r12d, 2                            #li 2=%103, =%-1,  cyc 3
	#victim %31, r13d                      #victim d3=%31, =%-1,  cyc -1
	mov r13d, 2                            #li 2=%107, =%-1,  cyc 3
	mov eax, r11d 
	cdq 
	idiv r12d 
	mov r11d, eax                          #div /=%105, 2=%103,  cyc 4
	#victim %103, r12d                     #victim check_dead_vr 2=%103, =%-1,  cyc -1
	mov eax, r10d 
	cdq 
	idiv r13d 
	mov r10d, eax                          #div /=%109, 2=%107,  cyc 14
	#victim %107, r13d                     #victim check_dead_vr 2=%107, =%-1,  cyc -1
	mov r12d, r11d                         #add +=%106, /=%105,  cyc 14
	#victim %105, r11d                     #victim check_dead_vr /=%105, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 144]        #ld carry=%35, =%-1,  cyc -1
	add r12d, r11d                         #add +=%106, carry=%35,  cyc 15
	mov r13d, r12d                         #assign c3=%23, +=%106,  cyc 16
	#victim %106, r12d                     #victim check_dead_vr +=%106, =%-1,  cyc -1
	mov r12d, r10d                         #add +=%110, /=%109,  cyc 24
	#victim %109, r10d                     #victim check_dead_vr /=%109, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 224]        #ld p=%55, =%-1,  cyc -1
	add r12d, r10d                         #add +=%110, p=%55,  cyc 25
	#victim %7, r14d                       #victim a3=%7, =%-1,  cyc -1
	mov r14d, r12d                         #assign d3=%31, +=%110,  cyc 26
	#victim %110, r12d                     #victim check_dead_vr +=%110, =%-1,  cyc -1


.L_b42:
	#victim %55, r10d                      #victim p=%55, =%-1,  cyc -1
	#victim %35, r11d                      #victim carry=%35, =%-1,  cyc -1
	#victim %23, r13d                      #victim c3=%23, =%-1,  cyc -1
	mov dword ptr [rbp - 96], r13d         #spill c3=%23, =%-1,  cyc -1
	#victim %31, r14d                      #victim d3=%31, =%-1,  cyc -1
	mov dword ptr [rbp - 128], r14d        #spill d3=%31, =%-1,  cyc -1
	#victim %15, r15d                      #victim b3=%15, =%-1,  cyc -1
	jmp .L_b45_if39_tail 


.L_b43_if39_else:
	mov r11d, dword ptr [rbp - 88]         #ld c2=%21, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%112, c2=%21,  cyc 0
	mov r13d, dword ptr [rbp - 120]        #ld d2=%29, =%-1,  cyc -1
	mov r12d, r13d                         #add +=%116, d2=%29,  cyc 0
	mov r14d, dword ptr [rbp - 64]         #ld b3=%15, =%-1,  cyc -1
	add r10d, r14d                         #add +=%112, b3=%15,  cyc 1
	mov r15d, dword ptr [rbp - 32]         #ld a3=%7, =%-1,  cyc -1
	add r12d, r15d                         #add +=%116, a3=%7,  cyc 1
	#victim %21, r11d                      #victim c2=%21, =%-1,  cyc -1
	mov r11d, r10d                         #div /=%113, +=%112,  cyc 2
	#victim %112, r10d                     #victim check_dead_vr +=%112, =%-1,  cyc -1
	mov r10d, r12d                         #div /=%117, +=%116,  cyc 2
	#victim %116, r12d                     #victim check_dead_vr +=%116, =%-1,  cyc -1
	mov r12d, 2                            #li 2=%111, =%-1,  cyc 3
	#victim %29, r13d                      #victim d2=%29, =%-1,  cyc -1
	mov r13d, 2                            #li 2=%115, =%-1,  cyc 3
	mov eax, r11d 
	cdq 
	idiv r12d 
	mov r11d, eax                          #div /=%113, 2=%111,  cyc 4
	#victim %111, r12d                     #victim check_dead_vr 2=%111, =%-1,  cyc -1
	mov eax, r10d 
	cdq 
	idiv r13d 
	mov r10d, eax                          #div /=%117, 2=%115,  cyc 14
	#victim %115, r13d                     #victim check_dead_vr 2=%115, =%-1,  cyc -1
	mov r12d, r11d                         #add +=%114, /=%113,  cyc 14
	#victim %113, r11d                     #victim check_dead_vr /=%113, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 144]        #ld carry=%35, =%-1,  cyc -1
	add r12d, r11d                         #add +=%114, carry=%35,  cyc 15
	mov r13d, r12d                         #assign c2=%21, +=%114,  cyc 16
	#victim %114, r12d                     #victim check_dead_vr +=%114, =%-1,  cyc -1
	mov r12d, r10d                         #add +=%118, /=%117,  cyc 24
	#victim %117, r10d                     #victim check_dead_vr /=%117, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 224]        #ld p=%55, =%-1,  cyc -1
	add r12d, r10d                         #add +=%118, p=%55,  cyc 25
	#victim %15, r14d                      #victim b3=%15, =%-1,  cyc -1
	mov r14d, r12d                         #assign d2=%29, +=%118,  cyc 26
	#victim %118, r12d                     #victim check_dead_vr +=%118, =%-1,  cyc -1


.L_b44:
	#victim %55, r10d                      #victim p=%55, =%-1,  cyc -1
	#victim %35, r11d                      #victim carry=%35, =%-1,  cyc -1
	#victim %21, r13d                      #victim c2=%21, =%-1,  cyc -1
	mov dword ptr [rbp - 88], r13d         #spill c2=%21, =%-1,  cyc -1
	#victim %29, r14d                      #victim d2=%29, =%-1,  cyc -1
	mov dword ptr [rbp - 120], r14d        #spill d2=%29, =%-1,  cyc -1
	#victim %7, r15d                       #victim a3=%7, =%-1,  cyc -1
	jmp .L_b45_if39_tail 


.L_b45_if39_tail:


.L_b47:
	jmp .L_b49_if20_tail 


.L_b49_if20_tail:


.L_b50:
	mov r10d, 1                            #li 1=%119, =%-1,  cyc 0
	mov r12d, dword ptr [rbp - 144]        #ld carry=%35, =%-1,  cyc -1
	mov r11d, r12d                         #add +=%120, carry=%35,  cyc 0
	add r11d, r10d                         #add +=%120, 1=%119,  cyc 1
	#victim %119, r10d                     #victim check_dead_vr 1=%119, =%-1,  cyc -1
	mov r12d, r11d                         #assign carry=%35, +=%120,  cyc 2
	#victim %120, r11d                     #victim check_dead_vr +=%120, =%-1,  cyc -1


.L_b51_while17_body_tail:
	mov r10d, dword ptr [rbp - 8]          #while 4, ld  a0=%1, =%-1,  cyc -1
	mov r15d, dword ptr [rbp - 72]         #while 4, ld  c0=%17, =%-1,  cyc -1
	mov r11d, r12d                         #while 3, assign skip=%33, move %s35 =%-1, =%-1,  cyc -1
	mov r12d, dword ptr [rbp - 136]        #while 4, ld  skip=%33, =%-1,  cyc -1
	mov r13d, r11d                         #while 2, assign carry=%35 =%-1, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 224]        #while 4, ld  p=%55, =%-1,  cyc -1
	mov dword ptr [rbp - 144], r13d        #while spill st dirty  carry=%35, =%-1,  cyc -1
	jmp .L_b18_while17_cond 


.L_b53_while17_tail:


.L_b54:
	mov r10d, 5                            #li 5=%121, =%-1,  cyc 0
	mov r12d, r10d                         #assign s=%122, 5=%121,  cyc 1
	#victim %121, r10d                     #victim check_dead_vr 5=%121, =%-1,  cyc -1


.L_b55_while:


.L_b56_while55_cond:
	mov r10d, 1                            #li 1=%124, =%-1,  cyc 0
	mov r14d, 0                            #li 0=%123, =%-1,  cyc 0
	#victim %55, r11d                      #victim p=%55, =%-1,  cyc -1
	mov dword ptr [rbp - 224], r11d        #spill p=%55, =%-1,  cyc -1
	mov r11d, r12d                         #sub -=%125, s=%122,  cyc 1
	sub r11d, r10d                         #sub -=%125, 1=%124,  cyc 2
	#victim %124, r10d                     #victim check_dead_vr 1=%124, =%-1,  cyc -1
	mov r12d, r11d                         #assign s=%122, -=%125,  cyc 3
	#victim %125, r11d                     #victim check_dead_vr -=%125, =%-1,  cyc -1
	cmp r12d, r14d                         #cmple s=%122, 0=%123,  cyc 4
	setle r10b                             #cmple s=%122, 0=%123,  cyc 4
	movzx r10d, r10b                       #cmple s=%122, 0=%123,  cyc 4
	#victim %126, r10d                     #victim check_dead_vr >=%126, =%-1,  cyc -1
	#victim %123, r14d                     #victim check_dead_vr 0=%123, =%-1,  cyc -1
	jle .L_b68_while55_tail 


.L_b57_while55_body:


.L_b59_if58_cond:
	mov r10d, 2                            #li 2=%127, =%-1,  cyc 0
	cmp r12d, r10d                         #cmple s=%122, 2=%127,  cyc 1
	setle r11b                             #cmple s=%122, 2=%127,  cyc 1
	movzx r11d, r11b                       #cmple s=%122, 2=%127,  cyc 1
	#victim %128, r11d                     #victim check_dead_vr >=%128, =%-1,  cyc -1
	#victim %127, r10d                     #victim check_dead_vr 2=%127, =%-1,  cyc -1
	#victim %122, r12d                     #victim s=%122, =%-1,  cyc -1
	mov dword ptr [rbp - 492], r12d        #spill s=%122, =%-1,  cyc -1
	#victim %35, r13d                      #victim carry=%35, =%-1,  cyc -1
	#victim %17, r15d                      #victim c0=%17, =%-1,  cyc -1
	mov dword ptr [rbp - 72], r15d         #spill c0=%17, =%-1,  cyc -1
	jle .L_b62_if58_else 


.L_b60_if58_then:
	mov r11d, dword ptr [rbp - 104]        #ld d0=%25, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%130, d0=%25,  cyc 0
	mov r13d, dword ptr [rbp - 8]          #ld a0=%1, =%-1,  cyc -1
	mov r12d, r13d                         #add +=%134, a0=%1,  cyc 0
	mov r14d, dword ptr [rbp - 96]         #ld c3=%23, =%-1,  cyc -1
	add r10d, r14d                         #add +=%130, c3=%23,  cyc 1
	mov r15d, dword ptr [rbp - 64]         #ld b3=%15, =%-1,  cyc -1
	add r12d, r15d                         #add +=%134, b3=%15,  cyc 1
	#victim %25, r11d                      #victim d0=%25, =%-1,  cyc -1
	mov r11d, r10d                         #div /=%131, +=%130,  cyc 2
	#victim %130, r10d                     #victim check_dead_vr +=%130, =%-1,  cyc -1
	mov r10d, r12d                         #div /=%135, +=%134,  cyc 2
	#victim %134, r12d                     #victim check_dead_vr +=%134, =%-1,  cyc -1
	mov r12d, 2                            #li 2=%129, =%-1,  cyc 3
	#victim %1, r13d                       #victim a0=%1, =%-1,  cyc -1
	mov r13d, 2                            #li 2=%133, =%-1,  cyc 3
	mov eax, r11d 
	cdq 
	idiv r12d 
	mov r11d, eax                          #div /=%131, 2=%129,  cyc 4
	#victim %129, r12d                     #victim check_dead_vr 2=%129, =%-1,  cyc -1
	mov eax, r10d 
	cdq 
	idiv r13d 
	mov r10d, eax                          #div /=%135, 2=%133,  cyc 14
	#victim %133, r13d                     #victim check_dead_vr 2=%133, =%-1,  cyc -1
	mov r12d, r11d                         #add +=%132, /=%131,  cyc 14
	#victim %131, r11d                     #victim check_dead_vr /=%131, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 492]        #ld s=%122, =%-1,  cyc -1
	add r12d, r11d                         #add +=%132, s=%122,  cyc 15
	mov r13d, r12d                         #assign d0=%25, +=%132,  cyc 16
	#victim %132, r12d                     #victim check_dead_vr +=%132, =%-1,  cyc -1
	mov r12d, r10d                         #add +=%136, /=%135,  cyc 24
	#victim %135, r10d                     #victim check_dead_vr /=%135, =%-1,  cyc -1
	add r12d, r11d                         #add +=%136, s=%122,  cyc 25
	mov r10d, r12d                         #assign a0=%1, +=%136,  cyc 26
	#victim %136, r12d                     #victim check_dead_vr +=%136, =%-1,  cyc -1


.L_b61:
	#victim %1, r10d                       #victim a0=%1, =%-1,  cyc -1
	mov dword ptr [rbp - 8], r10d          #spill a0=%1, =%-1,  cyc -1
	#victim %122, r11d                     #victim s=%122, =%-1,  cyc -1
	#victim %25, r13d                      #victim d0=%25, =%-1,  cyc -1
	mov dword ptr [rbp - 104], r13d        #spill d0=%25, =%-1,  cyc -1
	#victim %23, r14d                      #victim c3=%23, =%-1,  cyc -1
	#victim %15, r15d                      #victim b3=%15, =%-1,  cyc -1
	jmp .L_b64_if58_tail 


.L_b62_if58_else:
	mov r11d, dword ptr [rbp - 112]        #ld d1=%27, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%138, d1=%27,  cyc 0
	mov r13d, dword ptr [rbp - 16]         #ld a1=%3, =%-1,  cyc -1
	mov r12d, r13d                         #add +=%142, a1=%3,  cyc 0
	mov r14d, dword ptr [rbp - 88]         #ld c2=%21, =%-1,  cyc -1
	add r10d, r14d                         #add +=%138, c2=%21,  cyc 1
	mov r15d, dword ptr [rbp - 56]         #ld b2=%13, =%-1,  cyc -1
	add r12d, r15d                         #add +=%142, b2=%13,  cyc 1
	#victim %27, r11d                      #victim d1=%27, =%-1,  cyc -1
	mov r11d, r10d                         #div /=%139, +=%138,  cyc 2
	#victim %138, r10d                     #victim check_dead_vr +=%138, =%-1,  cyc -1
	mov r10d, r12d                         #div /=%143, +=%142,  cyc 2
	#victim %142, r12d                     #victim check_dead_vr +=%142, =%-1,  cyc -1
	mov r12d, 2                            #li 2=%137, =%-1,  cyc 3
	#victim %3, r13d                       #victim a1=%3, =%-1,  cyc -1
	mov r13d, 2                            #li 2=%141, =%-1,  cyc 3
	mov eax, r11d 
	cdq 
	idiv r12d 
	mov r11d, eax                          #div /=%139, 2=%137,  cyc 4
	#victim %137, r12d                     #victim check_dead_vr 2=%137, =%-1,  cyc -1
	mov eax, r10d 
	cdq 
	idiv r13d 
	mov r10d, eax                          #div /=%143, 2=%141,  cyc 14
	#victim %141, r13d                     #victim check_dead_vr 2=%141, =%-1,  cyc -1
	mov r12d, r11d                         #add +=%140, /=%139,  cyc 14
	#victim %139, r11d                     #victim check_dead_vr /=%139, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 492]        #ld s=%122, =%-1,  cyc -1
	add r12d, r11d                         #add +=%140, s=%122,  cyc 15
	mov r13d, r12d                         #assign d1=%27, +=%140,  cyc 16
	#victim %140, r12d                     #victim check_dead_vr +=%140, =%-1,  cyc -1
	mov r12d, r10d                         #add +=%144, /=%143,  cyc 24
	#victim %143, r10d                     #victim check_dead_vr /=%143, =%-1,  cyc -1
	add r12d, r11d                         #add +=%144, s=%122,  cyc 25
	mov r10d, r12d                         #assign a1=%3, +=%144,  cyc 26
	#victim %144, r12d                     #victim check_dead_vr +=%144, =%-1,  cyc -1


.L_b63:
	#victim %3, r10d                       #victim a1=%3, =%-1,  cyc -1
	mov dword ptr [rbp - 16], r10d         #spill a1=%3, =%-1,  cyc -1
	#victim %122, r11d                     #victim s=%122, =%-1,  cyc -1
	#victim %27, r13d                      #victim d1=%27, =%-1,  cyc -1
	mov dword ptr [rbp - 112], r13d        #spill d1=%27, =%-1,  cyc -1
	#victim %21, r14d                      #victim c2=%21, =%-1,  cyc -1
	#victim %13, r15d                      #victim b2=%13, =%-1,  cyc -1
	jmp .L_b64_if58_tail 


.L_b64_if58_tail:


.L_b66_while55_body_tail:
	mov r15d, dword ptr [rbp - 72]         #while 4, ld  c0=%17, =%-1,  cyc -1
	mov r13d, dword ptr [rbp - 144]        #while 4, ld  carry=%35, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 224]        #while 4, ld  p=%55, =%-1,  cyc -1
	mov r12d, dword ptr [rbp - 492]        #while 4, ld  s=%122, =%-1,  cyc -1
	jmp .L_b56_while55_cond 


.L_b68_while55_tail:


.L_b69:
	mov r11d, dword ptr [rbp - 8]          #ld a0=%1, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%145, a0=%1,  cyc 0
	#victim %1, r11d                       #victim check_dead_vr a0=%1, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 16]         #ld a1=%3, =%-1,  cyc -1
	add r10d, r11d                         #add +=%145, a1=%3,  cyc 1
	#victim %3, r11d                       #victim check_dead_vr a1=%3, =%-1,  cyc -1
	mov r11d, r10d                         #add +=%146, +=%145,  cyc 2
	#victim %145, r10d                     #victim check_dead_vr +=%145, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 24]         #ld a2=%5, =%-1,  cyc -1
	add r11d, r10d                         #add +=%146, a2=%5,  cyc 3
	#victim %5, r10d                       #victim check_dead_vr a2=%5, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%147, +=%146,  cyc 4
	#victim %146, r11d                     #victim check_dead_vr +=%146, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 32]         #ld a3=%7, =%-1,  cyc -1
	add r10d, r11d                         #add +=%147, a3=%7,  cyc 5
	#victim %7, r11d                       #victim check_dead_vr a3=%7, =%-1,  cyc -1
	mov r11d, r10d                         #add +=%148, +=%147,  cyc 6
	#victim %147, r10d                     #victim check_dead_vr +=%147, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 40]         #ld b0=%9, =%-1,  cyc -1
	add r11d, r10d                         #add +=%148, b0=%9,  cyc 7
	#victim %9, r10d                       #victim check_dead_vr b0=%9, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%149, +=%148,  cyc 8
	#victim %148, r11d                     #victim check_dead_vr +=%148, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 48]         #ld b1=%11, =%-1,  cyc -1
	add r10d, r11d                         #add +=%149, b1=%11,  cyc 9
	#victim %11, r11d                      #victim check_dead_vr b1=%11, =%-1,  cyc -1
	mov r11d, r10d                         #add +=%150, +=%149,  cyc 10
	#victim %149, r10d                     #victim check_dead_vr +=%149, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 56]         #ld b2=%13, =%-1,  cyc -1
	add r11d, r10d                         #add +=%150, b2=%13,  cyc 11
	#victim %13, r10d                      #victim check_dead_vr b2=%13, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%151, +=%150,  cyc 12
	#victim %150, r11d                     #victim check_dead_vr +=%150, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 64]         #ld b3=%15, =%-1,  cyc -1
	add r10d, r11d                         #add +=%151, b3=%15,  cyc 13
	#victim %15, r11d                      #victim check_dead_vr b3=%15, =%-1,  cyc -1
	mov r11d, r10d                         #add +=%152, +=%151,  cyc 14
	#victim %151, r10d                     #victim check_dead_vr +=%151, =%-1,  cyc -1
	add r11d, r15d                         #add +=%152, c0=%17,  cyc 15
	#victim %17, r15d                      #victim check_dead_vr c0=%17, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%153, +=%152,  cyc 16
	#victim %152, r11d                     #victim check_dead_vr +=%152, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 80]         #ld c1=%19, =%-1,  cyc -1
	add r10d, r11d                         #add +=%153, c1=%19,  cyc 17
	#victim %19, r11d                      #victim check_dead_vr c1=%19, =%-1,  cyc -1
	mov r11d, r10d                         #add +=%154, +=%153,  cyc 18
	#victim %153, r10d                     #victim check_dead_vr +=%153, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 88]         #ld c2=%21, =%-1,  cyc -1
	add r11d, r10d                         #add +=%154, c2=%21,  cyc 19
	#victim %21, r10d                      #victim check_dead_vr c2=%21, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%155, +=%154,  cyc 20
	#victim %154, r11d                     #victim check_dead_vr +=%154, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 96]         #ld c3=%23, =%-1,  cyc -1
	add r10d, r11d                         #add +=%155, c3=%23,  cyc 21
	#victim %23, r11d                      #victim check_dead_vr c3=%23, =%-1,  cyc -1
	mov r11d, r10d                         #add +=%156, +=%155,  cyc 22
	#victim %155, r10d                     #victim check_dead_vr +=%155, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 104]        #ld d0=%25, =%-1,  cyc -1
	add r11d, r10d                         #add +=%156, d0=%25,  cyc 23
	#victim %25, r10d                      #victim check_dead_vr d0=%25, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%157, +=%156,  cyc 24
	#victim %156, r11d                     #victim check_dead_vr +=%156, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 112]        #ld d1=%27, =%-1,  cyc -1
	add r10d, r11d                         #add +=%157, d1=%27,  cyc 25
	#victim %27, r11d                      #victim check_dead_vr d1=%27, =%-1,  cyc -1
	mov r11d, r10d                         #add +=%158, +=%157,  cyc 26
	#victim %157, r10d                     #victim check_dead_vr +=%157, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 120]        #ld d2=%29, =%-1,  cyc -1
	add r11d, r10d                         #add +=%158, d2=%29,  cyc 27
	#victim %29, r10d                      #victim check_dead_vr d2=%29, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%159, +=%158,  cyc 28
	#victim %158, r11d                     #victim check_dead_vr +=%158, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 128]        #ld d3=%31, =%-1,  cyc -1
	add r10d, r11d                         #add +=%159, d3=%31,  cyc 29
	#victim %31, r11d                      #victim check_dead_vr d3=%31, =%-1,  cyc -1
	mov r11d, r10d                         #add +=%160, +=%159,  cyc 30
	#victim %159, r10d                     #victim check_dead_vr +=%159, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 136]        #ld skip=%33, =%-1,  cyc -1
	add r11d, r10d                         #add +=%160, skip=%33,  cyc 31
	#victim %33, r10d                      #victim check_dead_vr skip=%33, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%161, +=%160,  cyc 32
	#victim %160, r11d                     #victim check_dead_vr +=%160, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 224]        #ld p=%55, =%-1,  cyc -1
	add r10d, r11d                         #add +=%161, p=%55,  cyc 33
	#victim %55, r11d                      #victim check_dead_vr p=%55, =%-1,  cyc -1
	mov r11d, r10d                         #add +=%162, +=%161,  cyc 34
	#victim %161, r10d                     #victim check_dead_vr +=%161, =%-1,  cyc -1
	add r11d, r12d                         #add +=%162, s=%122,  cyc 35
	#victim %122, r12d                     #victim check_dead_vr s=%122, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%163, +=%162,  cyc 36
	#victim %162, r11d                     #victim check_dead_vr +=%162, =%-1,  cyc -1
	add r10d, r13d                         #add +=%163, carry=%35,  cyc 37
	#victim %35, r13d                      #victim check_dead_vr carry=%35, =%-1,  cyc -1
	#---------------- print ret ----------------# 
	mov esi, r10d 
	lea rdi, [rip + fmt] 
	mov eax, 0 
	call printf@PLT 
	#------------------------------------------# 
	mov eax, r10d 
	#victim %163, r10d                     #victim check_dead_vr +=%163, =%-1,  cyc -1


.L_b70_jmp_to_ret:
	jmp .L_return 


.L_return:
	mov rsp, rbp 
	pop rbp 
	ret 

.section .note.GNU-stack,"",@progbits




```





