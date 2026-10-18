# VSC - Very Simple Compiler

After studying the book *Engineering a Compiler 3E*, I found that compiler theory was simpler than I had supposed. Before that, I thought it was very difficult, or even impossible, to understand. So I decided to write a toy compiler to kill some time and have fun.

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

Reg-state merge for the back edge of while-body is implemented, without `spill all`. I think it is not an easy task.


**`continue` and `break` register-state handling implemented.**

The allocator saves the register state at each `continue` and `break` point, then reconciles it with the target state after the while-loop allocation is completed. `continue` transfers control back to the while-condition, while `break` transfers control to the loop exit.

The random testing tool now generates `continue` and `break` as well, including statements after them to exercise dead-code handling. I ran multiple rounds of 1,000-program differential tests with both constructs enabled, including a run with `if/else` disabled to specifically test loop control flow without relying on the `spill all` behavior at branch boundaries. All tests passed. 

I use the method designed for while-body to implement `continue` and `break`, it works, honestly, I’m quite surprised, I thought these two token are much more complex than while-body.

Still need to reason more carefully about the correctness of `continue` and `break` register-state reconciliation, even though extensive random testing has not revealed any problems yet.



### 2026.1012

**if/else branch merge without spill-all is implemented**

Previously, spill-all at the end of `then/else` branches. Now, merge the register state of `then/else` branches at the end of if-scope. 

Just use the same way of `while` to implemente `then/else`, works.

Things seems easier from now, the complex control-flow problem of `while` and `if` have been work out.

Current merge algorithm is too simple. an improve is: consider future VR use after if-scope, reduce reg-spill.

20k+ random case passed.


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
- [ ] basic optimize
- [x] basic Semantic analysis
- [x] Three-address IR
- [x] x64 Inst-select
- [x] x64 Inst-schedule (BasicBlock-level, use score)
- [ ] x64 Inst-schedule (global, use score)
- [x] x64 -O0 reg-alloc	(spill/reload)
- [x] x64 -O1 reg-alloc (BasicBlock-level Wavefront)
- [x] x64 -O1 reg-alloc (local register state reconciliation on control-flow edges)
- [ ] x64 -O1 reg-alloc (global Wavefront)
- [x] x64 asm


## Supported Keywords


- `=`  `*`  `/`  `(`  `)`  `{`  `}`  `;`  `<`  `<=`  `>`  `>=`  `==`  `!=`
- `+` (binary add only)
- `-` (binary minus only)
- `int` `return` `if` `else` `while` `continue` `break`


## Language Restrictions
- only one function: `main` with no parameters
- no function calls, no ABI
- type only `int`
- identifiers follow the same naming rules as in the C language



## Compiler components detail


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


### Scope-based Compiler Architecture

`Scope` is central internal representation throughout the compiler pipeline.

In many traditional compilers, lexical scope, AST structure, control-flow graph, and basic blocks are represented by separate data structures. VSC takes a simpler approach: these structures are unified through a hierarchical `Scope` tree.

A `Scope` represents not only a lexical block, but also a control-flow region.

A Scope can represent:

- normal block scope `{ ... }`
- `if/else` branches
- `while` loops
- loop control targets (`break` / `continue`)
- basic block boundaries

The same Scope tree is used by:

- semantic analysis
- TAC generation
- basic block construction
- instruction scheduling
- register allocation

The compiler pipeline can be viewed as:

```
source code
    |
    v
 Scope tree
    |
    +---- TAC generation
    |
    +---- Basic blocks
    |
    +---- Instruction scheduling
    |
    +---- Register allocation
    |
    v
assembly
```

### Control Flow Through Scope

Control flow is also represented using Scope relationships.

For example, a `while` Scope contains:

```
while Scope
    |
    +-- condition Scope
    |
    +-- body Scope
```

The loop Scope owns the targets of `break` and `continue`.

- `continue` transfers control back to the loop condition.
- `break` transfers control to the code after the loop.

Nested loops naturally work because each loop Scope keeps its own control-flow targets.

```
outer while Scope
    |
    +-- inner while Scope
            |
            +-- continue -> inner condition
            +-- break    -> after inner while
```

### Scope and Reachability

`Scope` also defines reachable code boundaries.

When a terminating statement such as `break`, `continue`, or `return` appears, VSC inserts a `bb_terminate` marker.

The following statements remain in the source Scope tree, but they are unreachable and do not generate TAC.

This is important for register allocation:

- TAC generation does not create instructions for dead code.
- Register allocation does not allocate physical registers for unreachable
  instructions.
- The virtual-register state is therefore not polluted by dead code.

This simple mechanism makes control-flow handling and register-state
tracking much more reliable.



### DAG Construction
...



### Three Address Code

- `bb_terminate` 

	Ab optional optimization method used to eliminate some dead code and dead jump labels. 


### Instruction Select
...

### Instruction Schedule

- The main target is to reduce spill under limited physical registers, not parallelism.

- Schedule priority:

	Score‑Driven Instruction Scheduler: Refined scheduling based on critical‑path latency, functional‑unit availability(ALU/IMUL/DIV), and remaining‑usage hints from scope‑level transient‑variable consumption; no explicit register‑pressure modelling.



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



## Basic Test

```c

$make
$./test/basic_test.sh

```

## Random Test

```c

$make
$./test/random_test/random_test.sh 2000

```

output:

```c

Random seed: 1791997507512941761
Cases: 2000
Config: vsc/test/python_random_test/vsl_fuzz_config.json
Random seed: 1791997507512941761
Cases: 2000; variables: 2..100; max depth: 30; budget: 320
Enabled tokens: if, else, while, break, continue
Force statements after break/continue: True
Fully random control-placement probability: 25% per program

...
...

===== Differential test summary =====
total: 2000
generated break statements: 39010
generated continue statements: 38787
direct loop-body break statements: 12006
direct loop-body continue statements: 11858
statements forced after break/continue: 76588
programs using fully random control placement: 495/2000
block-start controls: break=10327, continue=10280
pass: 2000
No differences or compile failures found.

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


### AST

```

========== ast ==========
======b0=0  parent=0====== SEM_FILE_SCOPE

    ======test=1  parent=0====== SEM_FUNC_DEFINE
    symbol: i 1
    symbol: a 3
    symbol: b 5
    symbol: c 7
    symbol: %0 0
    symbol: %2 2
    symbol: %4 4
    symbol: %6 6
    symbol: %26 26
    symbol: %27 27
    symbol: %28 28
    ast:
    └── [del INT]
        └── [var i %1]
    ast:
    └── [OP_ASSIGN %1]
        ├── [var i %1]
        └── [num 5 %0]
    ast:
    └── [del INT]
        └── [var a %3]
    ast:
    └── [OP_ASSIGN %3]
        ├── [var a %3]
        └── [num 0 %2]
    ast:
    └── [del INT]
        └── [var b %5]
    ast:
    └── [OP_ASSIGN %5]
        ├── [var b %5]
        └── [num 1 %4]
    ast:
    └── [del INT]
        └── [var c %7]
    ast:
    └── [OP_ASSIGN %7]
        ├── [var c %7]
        └── [num 2 %6]

        ======.L_b2_while=2  parent=1====== SEM_WHILE

            ======.L_b3_while2_cond=3  parent=2====== SEM_WHILE_COND out=28  in=26 
            symbol: %8 8
            symbol: %9 9
            symbol: %10 10
            symbol: %11 11
            ast:
            └── [OP_CMP_LE %11]
                ├── [OP_ASSIGN %1]
                │   ├── [var i %1]
                │   └── [OP_SUB %10]
                │       ├── [var i %1]
                │       └── [num 1 %9]
                └── [num 0 %8]

            ======.L_b4_while2_body=4  parent=2====== SEM_WHILE_BODY
            symbol: %12 12
            symbol: %24 24
            symbol: %25 25
            ast:
            └── [OP_ASSIGN %3]
                ├── [var a %3]
                └── [OP_ADD %12]
                    ├── [var a %3]
                    └── [var i %1]

                ======.L_b5_if5=5  parent=4====== SEM_IF

                    ======.L_b6_if5_cond=6  parent=5====== SEM_IF_cond out=11
                    symbol: %13 13
                    symbol: %14 14
                    ast:
                    └── [OP_CMP_NE %14]
                        ├── [var i %1]
                        └── [num 3 %13]

                    ======.L_b7_if5_then=7  parent=5====== SEM_IF_then
                    symbol: %15 15
                    symbol: %16 16
                    ast:
                    └── [OP_ASSIGN %5]
                        ├── [var b %5]
                        └── [OP_ADD %15]
                            ├── [var b %5]
                            └── [var a %3]

                        ======.L_b8_while2_continue=8  parent=7====== SEM_CONTINUE out=3 virtual

                        ======.L_b9=9  parent=7====== SEM_LABEL virtual
                        ast:
                        └── [OP_ASSIGN %3]
                            ├── [var a %3]
                            └── [num 100 %16]

                        ======.L_b10=10  parent=7====== SEM_LABEL out=13 virtual

                    ======.L_b11_if5_else=11  parent=5====== SEM_IF_else  in=6 
                    symbol: %17 17
                    ast:
                    └── [OP_ASSIGN %7]
                        ├── [var c %7]
                        └── [OP_ADD %17]
                            ├── [var c %7]
                            └── [var i %1]

                        ======.L_b12=12  parent=11====== SEM_LABEL out=13 virtual

                    ======.L_b13_if5_tail=13  parent=5====== SEM_LABEL  in=10 12  virtual

                ======.L_b15_if15=15  parent=4====== SEM_IF

                    ======.L_b16_if15_cond=16  parent=15====== SEM_IF_cond out=21
                    symbol: %18 18
                    symbol: %19 19
                    ast:
                    └── [OP_CMP_NE %19]
                        ├── [var i %1]
                        └── [num 2 %18]

                    ======.L_b17_if15_then=17  parent=15====== SEM_IF_then
                    symbol: %20 20
                    symbol: %21 21
                    symbol: %22 22
                    ast:
                    └── [OP_ASSIGN %5]
                        ├── [var b %5]
                        └── [OP_ADD %21]
                            ├── [var b %5]
                            └── [num 10 %20]

                        ======.L_b18_while2_break=18  parent=17====== SEM_BREAK out=28 virtual

                        ======.L_b19=19  parent=17====== SEM_LABEL virtual
                        ast:
                        └── [OP_ASSIGN %7]
                            ├── [var c %7]
                            └── [num 200 %22]

                        ======.L_b20=20  parent=17====== SEM_LABEL out=24 virtual

                    ======.L_b21_if15_else=21  parent=15====== SEM_IF_else  in=16 
                    symbol: %23 23
                    ast:
                    └── [OP_ASSIGN %3]
                        ├── [var a %3]
                        └── [OP_ADD %23]
                            ├── [var a %3]
                            └── [var c %7]

                        ======.L_b22=22  parent=21====== SEM_LABEL out=24 virtual

                    ======.L_b24_if15_tail=24  parent=15====== SEM_LABEL  in=20 22  virtual

                ======.L_b25=25  parent=4====== SEM_LABEL virtual
                ast:
                └── [OP_ASSIGN %7]
                    ├── [var c %7]
                    └── [OP_ADD %25]
                        ├── [var c %7]
                        └── [num 1 %24]

                ======.L_b26_while2_body_tail=26  parent=4====== SEM_LABEL out=3 virtual

            ======.L_b28_while2_tail=28  parent=2====== SEM_LABEL  in=3  virtual

        ======.L_b29=29  parent=1====== SEM_LABEL virtual
        ast:
        └── [save-ret(return) %28]
            └── [OP_ADD %28]
                ├── [OP_ADD %27]
                │   ├── [OP_ADD %26]
                │   │   ├── [var a %3]
                │   │   └── [var b %5]
                │   └── [var c %7]
                └── [var i %1]

        ======.L_b30_jmp_to_ret=30  parent=1====== SEM_LABEL out=32

=========================



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

	mov r10d, 2                            #li, 2 %8  cyc -1
	cmp r10d, 3                            #cmpg, > %9, 2 %8, 3 %7  cyc 0
	setg r11b                              #cmpg, > %9, 2 %8, 3 %7  cyc 0
	movzx r11d, r11b                       #cmpg, > %9, 2 %8, 3 %7  cyc 0
	                                       #victim r10d check_dead_vr, 2 %8  cyc -1
	mov r10d, 0                            #assign, a %3, 0 %2  cyc 0
	cmp r10d, 5                            #cmpg, > %5, a %3, 5 %4  cyc 1
	setg r12b                              #cmpg, > %5, a %3, 5 %4  cyc 1
	movzx r12d, r12b                       #cmpg, > %5, a %3, 5 %4  cyc 1
	mov r13d, 5                            #assign, i %1, 5 %0  cyc 1
	mov r14d, r11d                         #assign, c %10, > %9  cyc 2
	                                       #victim r11d check_dead_vr, > %9  cyc -1
	mov r11d, r12d                         #assign, b %6, > %5  cyc 2
	                                       #victim r12d check_dead_vr, > %5  cyc -1


.L_b3_while2_cond:
	mov r12d, r13d 
	sub r12d, 1                            #sub, - %13, i %1, 1 %12  cyc 0
	mov r13d, r12d                         #assign, i %1, - %13  cyc 1
	                                       #victim r12d check_dead_vr, - %13  cyc -1
	cmp r13d, 0                            #cmple, > %14, i %1, 0 %11  cyc 2
	setle r12b                             #cmple, > %14, i %1, 0 %11  cyc 2
	movzx r12d, r12b                       #cmple, > %14, i %1, 0 %11  cyc 2
	                                       #victim r12d check_dead_vr, > %14  cyc -1
	jle .L_b30_while2_tail 


.L_b4_while2_body:
	mov r12d, r10d 
	add r12d, r13d                         #add, + %15, a %3, i %1  cyc 0
	mov r10d, r12d                         #assign, a %3, + %15  cyc 1
	                                       #victim r12d check_dead_vr, + %15  cyc -1


.L_b6_if5_cond:
	cmp r13d, 3                            #cmpne, == %17, i %1, 3 %16  cyc 0
	setne r12b                             #cmpne, == %17, i %1, 3 %16  cyc 0
	movzx r12d, r12b                       #cmpne, == %17, i %1, 3 %16  cyc 0
	                                       #victim r12d check_dead_vr, == %17  cyc -1
	jne .L_b12_if5_else 


.L_b7_if5_then:
	mov r12d, r11d 
	add r12d, r10d                         #add, + %18, b %6, a %3  cyc 0
	mov r11d, r12d                         #assign, b %6, + %18  cyc 1
	                                       #victim r12d check_dead_vr, + %18  cyc -1


.L_b8_while2_continue:
	jmp .L_b3_while2_cond 


.L_b10:
	jmp .L_b14_if5_tail 


.L_b12_if5_else:
	mov r12d, r14d 
	add r12d, r13d                         #add, + %20, c %10, i %1  cyc 0
	mov r14d, r12d                         #assign, c %10, + %20  cyc 1
	                                       #victim r12d check_dead_vr, + %20  cyc -1
	jmp .L_b14_if5_tail 


.L_b14_if5_tail:


.L_b17_if16_cond:
	cmp r13d, 2                            #cmpne, == %22, i %1, 2 %21  cyc 0
	setne r12b                             #cmpne, == %22, i %1, 2 %21  cyc 0
	movzx r12d, r12b                       #cmpne, == %22, i %1, 2 %21  cyc 0
	                                       #victim r12d check_dead_vr, == %22  cyc -1
	jne .L_b23_if16_else 


.L_b18_if16_then:
	mov r12d, r11d 
	add r12d, 10                           #add, + %24, b %6, 10 %23  cyc 0
	mov r11d, r12d                         #assign, b %6, + %24  cyc 1
	                                       #victim r12d check_dead_vr, + %24  cyc -1


.L_b19_while2_break:
	jmp .L_b30_while2_tail 


.L_b21:
	jmp .L_b26_if16_tail 


.L_b23_if16_else:
	mov r12d, r10d 
	add r12d, r14d                         #add, + %26, a %3, c %10  cyc 0
	mov r10d, r12d                         #assign, a %3, + %26  cyc 1
	                                       #victim r12d check_dead_vr, + %26  cyc -1
	jmp .L_b26_if16_tail 


.L_b26_if16_tail:


.L_b27:
	mov r12d, r14d 
	add r12d, 1                            #add, + %28, c %10, 1 %27  cyc 0
	mov r14d, r12d                         #assign, c %10, + %28  cyc 1
	                                       #victim r12d check_dead_vr, + %28  cyc -1
	jmp .L_b3_while2_cond 


.L_b30_while2_tail:


.L_b31:
	mov r12d, r10d 
	add r12d, r11d                         #add, + %29, a %3, b %6  cyc 0
	                                       #victim r10d check_dead_vr, a %3  cyc -1
	                                       #victim r11d check_dead_vr, b %6  cyc -1
	mov r10d, r12d 
	add r10d, r14d                         #add, + %30, + %29, c %10  cyc 1
	                                       #victim r12d check_dead_vr, + %29  cyc -1
	                                       #victim r14d check_dead_vr, c %10  cyc -1
	mov r11d, r10d 
	add r11d, r13d                         #add, + %31, + %30, i %1  cyc 2
	                                       #victim r10d check_dead_vr, + %30  cyc -1
	                                       #victim r13d check_dead_vr, i %1  cyc -1
	#---------------- print ret ----------------# 
	mov esi, r11d 
	lea rdi, [rip + fmt] 
	mov eax, 0 
	call printf@PLT 
	#------------------------------------------# 
	mov eax, r11d 
	                                       #victim r11d check_dead_vr, + %31  cyc -1
	jmp .L_return 


.L_return:
	mov rsp, rbp 
	pop rbp 
	ret 

.section .note.GNU-stack,"",@progbits




```





