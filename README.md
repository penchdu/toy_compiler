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



### 2026.10.11

**Random Testing Tool Written by ChatGPT**

GPT wrote a very useful and awesome Python-based random testing tool to stress-test VSC.

It uses random seeds to generate programs with configurable variable counts, nesting depths, and AST statement budgets. It supports deeply nested `if/else` and `while` constructs, and can selectively enable or disable language features through a JSON configuration file. The tool compiles and runs each generated program with both GCC and VSC, then compares their results to automatically detect miscompilation.

I ran it several times, testing 1,000 programs per run, with up to 140 variables, a maximum nesting depth of 30, and an AST statement budget of 320 per program. All tests passed.

The normal back edge from the while-body to the while-condition is implemented without `spill all` on this edge. This was not an easy task.

**`break` and `continue` register-state handling implemented.**

The allocator saves the register state at each `break` and `continue` point, then reconciles it with the target state after the while-loop allocation is completed. `continue` transfers control back to the while-condition, while `break` transfers control to the loop exit.

The random testing tool now generates `break` and `continue` as well, including statements after them to exercise dead-code handling. I ran multiple rounds of 1,000-program differential tests with both constructs enabled, including a run with `if/else` disabled to specifically test loop control flow without relying on the `spill all` behavior at branch boundaries. All tests passed. Honestly, I’m surprised.

Still need to reason more carefully about the correctness of `break` and `continue` register-state reconciliation, even though extensive random testing has not revealed any problems yet.




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
- [x] x64 -O1 reg-alloc local register state reconciliation on control-flow edges
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

- Scope Tree Architecture: 

	Each `Scope` maintains a parent pointer and child references, forming a hierarchical scope tree. Variables are declared within the symbol table of their enclosing scope.
	
- Lexical Resolution & Variable Shadowing: 

	Name lookup starts from the innermost active scope and walks outward through parent scopes. An identifier declared in an inner scope shadows an identifier with the same name in an outer scope.
- Control-Flow Scoping & Basic Blocks: 
	
	`if`/`else` constructs create dedicated scopes for conditions, branches, and control-flow join points. These scopes are later used to organize basic blocks, jump targets, and control-flow edges.
	
- Name Disambiguation: 

	During semantic analysis, variables are assigned unique internal names (for example, `global_a` and `block2_a`) so that shadowed identifiers can be distinguished in the flat intermediate representation.



### DAG Construction
...

### Scope and Semantic Analysis

Semantic analysis performs:

- variable declaration checking
- symbol table construction
- scope resolution
- variable renaming


### Three Address Code

- `bb_terminate` 

	Originally, it was an optional optimization method used to eliminate some dead code and dead jump labels. Later, I realized that it is actually necessary for register allocation. It prevents TAC generation from producing instructions for unreachable code, so the register allocator does not allocate physical registers for these dead instructions. Therefore, the `vr_state` remains correct.


### Instruction Select
...

### Instruction Schedule


Schedule priority:

- Score‑Driven Instruction Scheduler: Refined scheduling based on critical‑path latency, functional‑unit availability(ALU/IMUL/DIV), and remaining‑usage hints from scope‑level transient‑variable consumption; no explicit register‑pressure modelling.



### Register Allocation

- `pre_cond_swap_pr` & `after_cond_swap_pr`: 

	A temporary physical register used to break register move cycles during this state transition. It does not preserve program state; it only provides temporary storage for register swapping.


- `mc_victim`

	A debugging marker used to track potential dirty-value loss across control-flow edges. It records physical registers containing virtual registers that become dirty after a loop body, while they were clean before entering the body. When such a register is spilled later, the allocator must generate a store instruction. Otherwise, the updated value may be lost because the new value only exists in the physical register. `mc_victim` helps detect this kind of incorrect spill behavior.


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

## Random Test

```c

$make
$cd test/python_random_test
$./python_random_test_time_seed.sh  100

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
	int i = 5;
	int a = 0;
	int b = 1;
	int c = 2;

	while ((i = i - 1) > 0)
	{
		a = a + i;

		if (i == 3)
		{
			b = b + a;
			continue;
			a = 100;
		}
		else
		{
			c = c + i;
		}

		if (i == 2)
		{
			b = b + 10;
			break;
			c = 200;
		}
		else
		{
			a = a + c;
		}

		c = c + 1;
	}

	return a + b + c + i;
}



```


the `vsc` compile the .cpp into two assembly: `a0.s` and `a1.s`


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
	sub rsp, 128 

	mov r10d, 5                            #li 5=%0, =%-1,  cyc 0
	mov r11d, 0                            #li 0=%2, =%-1,  cyc 0
	mov r12d, r10d                         #assign i=%1, 5=%0,  cyc 1
	                                       #victim r10d check_dead_vr 5=%0, =%-1,  cyc -1
	mov r10d, r11d                         #assign a=%3, 0=%2,  cyc 1
	                                       #victim r11d check_dead_vr 0=%2, =%-1,  cyc -1
	mov r11d, 1                            #li 1=%4, =%-1,  cyc 2
	mov r13d, 2                            #li 2=%6, =%-1,  cyc 2
	mov r14d, r11d                         #assign b=%5, 1=%4,  cyc 3
	                                       #victim r11d check_dead_vr 1=%4, =%-1,  cyc -1
	mov r11d, r13d                         #assign c=%7, 2=%6,  cyc 3
	                                       #victim r13d check_dead_vr 2=%6, =%-1,  cyc -1


.L_b2_while:


.L_b3_while2_cond:
	mov r13d, 1                            #li 1=%9, =%-1,  cyc 0
	mov r15d, 0                            #li 0=%8, =%-1,  cyc 0
	                                       #victim r10d a=%3, =%-1,  cyc -1
	mov dword ptr [rbp - 16], r10d         #spill a=%3, =%-1,  cyc -1
	mov r10d, r12d                         #sub -=%10, i=%1,  cyc 1
	sub r10d, r13d                         #sub -=%10, 1=%9,  cyc 2
	                                       #victim r13d check_dead_vr 1=%9, =%-1,  cyc -1
	mov r12d, r10d                         #assign i=%1, -=%10,  cyc 3
	                                       #victim r10d check_dead_vr -=%10, =%-1,  cyc -1
	cmp r12d, r15d                         #cmple i=%1, 0=%8,  cyc 4
	setle r10b                             #cmple i=%1, 0=%8,  cyc 4
	movzx r10d, r10b                       #cmple i=%1, 0=%8,  cyc 4
	                                       #victim r10d check_dead_vr >=%11, =%-1,  cyc -1
	                                       #victim r15d check_dead_vr 0=%8, =%-1,  cyc -1
	jle .L_b28_while2_tail 


.L_b4_while2_body:
	mov r13d, dword ptr [rbp - 16]         #ld a=%3, =%-1,  cyc -1
	mov r10d, r13d                         #add +=%12, a=%3,  cyc 0
	add r10d, r12d                         #add +=%12, i=%1,  cyc 1
	mov r13d, r10d                         #assign a=%3, +=%12,  cyc 2
	                                       #victim r10d check_dead_vr +=%12, =%-1,  cyc -1


.L_b6_if5_cond:
	mov r10d, 3                            #li 3=%13, =%-1,  cyc 0
	cmp r12d, r10d                         #cmpne i=%1, 3=%13,  cyc 1
	setne r15b                             #cmpne i=%1, 3=%13,  cyc 1
	movzx r15d, r15b                       #cmpne i=%1, 3=%13,  cyc 1
	                                       #victim r15d check_dead_vr ===%14, =%-1,  cyc -1
	                                       #victim r10d check_dead_vr 3=%13, =%-1,  cyc -1
	                                       #victim r11d c=%7, =%-1,  cyc -1
	mov dword ptr [rbp - 32], r11d         #spill c=%7, =%-1,  cyc -1
	                                       #victim r12d i=%1, =%-1,  cyc -1
	mov dword ptr [rbp - 8], r12d          #spill i=%1, =%-1,  cyc -1
	                                       #victim r13d a=%3, =%-1,  cyc -1
	mov dword ptr [rbp - 16], r13d         #spill a=%3, =%-1,  cyc -1
	                                       #victim r14d b=%5, =%-1,  cyc -1
	mov dword ptr [rbp - 24], r14d         #spill b=%5, =%-1,  cyc -1
	jne .L_b11_if5_else 


.L_b7_if5_then:
	mov r11d, dword ptr [rbp - 24]         #ld b=%5, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%15, b=%5,  cyc 0
	mov r12d, dword ptr [rbp - 16]         #ld a=%3, =%-1,  cyc -1
	add r10d, r12d                         #add +=%15, a=%3,  cyc 1
	mov r11d, r10d                         #assign b=%5, +=%15,  cyc 2
	                                       #victim r10d check_dead_vr +=%15, =%-1,  cyc -1


.L_b8_while2_continue:
	mov r10d, r12d                         #while 3, assign i=%1, move %s3 =%-1, =%-1,  cyc -1
	mov r12d, dword ptr [rbp - 8]          #while 4, ld  i=%1, =%-1,  cyc -1
	mov r14d, r11d                         #while 2, assign b=%5 =%-1, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 32]         #while 4, ld  c=%7, =%-1,  cyc -1
	jmp .L_b3_while2_cond 


.L_b9:
	mov r10d, 100                          #li 100=%16, =%-1,  cyc 0
	mov r12d, r10d                         #assign a=%3, 100=%16,  cyc 1
	                                       #victim r10d check_dead_vr 100=%16, =%-1,  cyc -1


.L_b10:
	                                       #victim r11d b=%5, =%-1,  cyc -1
	mov dword ptr [rbp - 24], r11d         #spill b=%5, =%-1,  cyc -1
	                                       #victim r12d a=%3, =%-1,  cyc -1
	mov dword ptr [rbp - 16], r12d         #spill a=%3, =%-1,  cyc -1
	jmp .L_b13_if5_tail 


.L_b11_if5_else:
	mov r11d, dword ptr [rbp - 32]         #ld c=%7, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%17, c=%7,  cyc 0
	mov r12d, dword ptr [rbp - 8]          #ld i=%1, =%-1,  cyc -1
	add r10d, r12d                         #add +=%17, i=%1,  cyc 1
	mov r11d, r10d                         #assign c=%7, +=%17,  cyc 2
	                                       #victim r10d check_dead_vr +=%17, =%-1,  cyc -1


.L_b12:
	                                       #victim r11d c=%7, =%-1,  cyc -1
	mov dword ptr [rbp - 32], r11d         #spill c=%7, =%-1,  cyc -1
	                                       #victim r12d i=%1, =%-1,  cyc -1
	jmp .L_b13_if5_tail 


.L_b13_if5_tail:


.L_b16_if15_cond:
	mov r10d, 2                            #li 2=%18, =%-1,  cyc 0
	mov r12d, dword ptr [rbp - 8]          #ld i=%1, =%-1,  cyc -1
	cmp r12d, r10d                         #cmpne i=%1, 2=%18,  cyc 1
	setne r11b                             #cmpne i=%1, 2=%18,  cyc 1
	movzx r11d, r11b                       #cmpne i=%1, 2=%18,  cyc 1
	                                       #victim r11d check_dead_vr ===%19, =%-1,  cyc -1
	                                       #victim r10d check_dead_vr 2=%18, =%-1,  cyc -1
	                                       #victim r12d i=%1, =%-1,  cyc -1
	jne .L_b21_if15_else 


.L_b17_if15_then:
	mov r10d, 10                           #li 10=%20, =%-1,  cyc 0
	mov r12d, dword ptr [rbp - 24]         #ld b=%5, =%-1,  cyc -1
	mov r11d, r12d                         #add +=%21, b=%5,  cyc 0
	add r11d, r10d                         #add +=%21, 10=%20,  cyc 1
	                                       #victim r10d check_dead_vr 10=%20, =%-1,  cyc -1
	mov r12d, r11d                         #assign b=%5, +=%21,  cyc 2
	                                       #victim r11d check_dead_vr +=%21, =%-1,  cyc -1


.L_b18_while2_break:
	mov r11d, r12d                         #while 3, assign i=%1, move %s5 =%-1, =%-1,  cyc -1
	mov r12d, dword ptr [rbp - 8]          #while 4, ld  i=%1, =%-1,  cyc -1
	mov r14d, r11d                         #while 2, assign b=%5 =%-1, =%-1,  cyc -1
	mov r11d, dword ptr [rbp - 32]         #while 4, ld  c=%7, =%-1,  cyc -1
	jmp .L_b28_while2_tail 


.L_b19:
	mov r10d, 200                          #li 200=%22, =%-1,  cyc 0
	mov r11d, r10d                         #assign c=%7, 200=%22,  cyc 1
	                                       #victim r10d check_dead_vr 200=%22, =%-1,  cyc -1


.L_b20:
	                                       #victim r11d c=%7, =%-1,  cyc -1
	mov dword ptr [rbp - 32], r11d         #spill c=%7, =%-1,  cyc -1
	                                       #victim r12d b=%5, =%-1,  cyc -1
	mov dword ptr [rbp - 24], r12d         #spill b=%5, =%-1,  cyc -1
	jmp .L_b24_if15_tail 


.L_b21_if15_else:
	mov r11d, dword ptr [rbp - 16]         #ld a=%3, =%-1,  cyc -1
	mov r10d, r11d                         #add +=%23, a=%3,  cyc 0
	mov r12d, dword ptr [rbp - 32]         #ld c=%7, =%-1,  cyc -1
	add r10d, r12d                         #add +=%23, c=%7,  cyc 1
	mov r11d, r10d                         #assign a=%3, +=%23,  cyc 2
	                                       #victim r10d check_dead_vr +=%23, =%-1,  cyc -1


.L_b22:
	                                       #victim r11d a=%3, =%-1,  cyc -1
	mov dword ptr [rbp - 16], r11d         #spill a=%3, =%-1,  cyc -1
	                                       #victim r12d c=%7, =%-1,  cyc -1
	jmp .L_b24_if15_tail 


.L_b24_if15_tail:


.L_b25:
	mov r10d, 1                            #li 1=%24, =%-1,  cyc 0
	mov r12d, dword ptr [rbp - 32]         #ld c=%7, =%-1,  cyc -1
	mov r11d, r12d                         #add +=%25, c=%7,  cyc 0
	add r11d, r10d                         #add +=%25, 1=%24,  cyc 1
	                                       #victim r10d check_dead_vr 1=%24, =%-1,  cyc -1
	mov r12d, r11d                         #assign c=%7, +=%25,  cyc 2
	                                       #victim r11d check_dead_vr +=%25, =%-1,  cyc -1


.L_b26_while2_body_tail:
	mov r10d, r12d                         #while 3, assign i=%1, move %s7 =%-1, =%-1,  cyc -1
	mov r12d, dword ptr [rbp - 8]          #while 4, ld  i=%1, =%-1,  cyc -1
	mov r11d, r10d                         #while 3, assign a=%3, move %s7 =%-1, =%-1,  cyc -1
	mov r10d, dword ptr [rbp - 16]         #while 4, ld  a=%3, =%-1,  cyc -1
	mov r14d, dword ptr [rbp - 24]         #while 4, ld  b=%5, =%-1,  cyc -1
	jmp .L_b3_while2_cond 


.L_b28_while2_tail:


.L_b29:
	mov r13d, dword ptr [rbp - 16]         #ld a=%3, =%-1,  cyc -1
	mov r10d, r13d                         #add +=%26, a=%3,  cyc 0
	                                       #victim r13d check_dead_vr a=%3, =%-1,  cyc -1
	add r10d, r14d                         #add +=%26, b=%5,  cyc 1
	                                       #victim r14d check_dead_vr b=%5, =%-1,  cyc -1
	mov r13d, r10d                         #add +=%27, +=%26,  cyc 2
	                                       #victim r10d check_dead_vr +=%26, =%-1,  cyc -1
	add r13d, r11d                         #add +=%27, c=%7,  cyc 3
	                                       #victim r11d check_dead_vr c=%7, =%-1,  cyc -1
	mov r10d, r13d                         #add +=%28, +=%27,  cyc 4
	                                       #victim r13d check_dead_vr +=%27, =%-1,  cyc -1
	add r10d, r12d                         #add +=%28, i=%1,  cyc 5
	                                       #victim r12d check_dead_vr i=%1, =%-1,  cyc -1
	#---------------- print ret ----------------# 
	mov esi, r10d 
	lea rdi, [rip + fmt] 
	mov eax, 0 
	call printf@PLT 
	#------------------------------------------# 
	mov eax, r10d 
	                                       #victim r10d check_dead_vr +=%28, =%-1,  cyc -1


.L_b30_jmp_to_ret:
	jmp .L_return 


.L_return:
	mov rsp, rbp 
	pop rbp 
	ret 

.section .note.GNU-stack,"",@progbits



```





