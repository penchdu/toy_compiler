# VSLC - Very Simple Language Compiler

after i learned the book *Engineering a Compiler 3E*, i found the compiler is much more simpler than what i suppose **if you do not want to build a 
real-world compiler**, so i want to make one.

before i start coding i thought it is simple, when i am coding i found it is not that simple even it is a toy compiler, i think i can finish it with the help of AI.

i design and write the code, and sometimes i show the code to AI and ask if it discovered some bug, i ask AI to write some simple and no-interesting code, for example, read a word from file, print the ast, i also ask AI for compiler questions.

i name the language vsl (very simple language)
so the compiler's name is vslc

The goal is use vslc to compile vsl source code to x86 or riscv assembly, or direct to binary, and let it run on cpu.


## the plan is:
## Compiler Pipeline

```
Source Code
    |
    v
Lexer
    |
    v
Parser
    |
    v
AST
    |
    v
Three-address IR
    |
    v
Instruction Selection
    |
    v
DAG Construction
    |
    v
Instruction Scheduling
    |
    v
Register Allocation
    |
    v
x86 Assembly
```

## Project Goal

The purpose of this project is learning compiler construction.

The project focuses on:

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


## vsl Language

## Supported Types

Currently only:

```
int
```


## Supported Keywords

```
=
+
-
*
/
(
)
{
}
return
void
;
```


## Identifier Rules
same as C

## Current Language Restrictions
- only one function: `main` with no parameters
- no function calls
- no ABI handling
- only integer operations

# Compiler Components
## 1. Lexer

The lexer converts source code into tokens.
Example:
Input:
```c
int a = 10;
```

Output:
```
tk_int
tk_var(a)
tk_assign
tk_const_num(10)
tk_semicolon
```

## 2. Parser
The parser converts tokens into an Abstract Syntax Tree (AST).

Example:

Source:

```c
a + b * 2;
```

AST:

```
        +
       / \
      a   *
         / \
        b   2
```


The parser handles:

- expressions
- statements
- variable declarations
- blocks
- scopes


---


## 3. Scope and Semantic Analysis

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


This avoids name conflicts between different scopes.


---


## 4. Intermediate Representation

The compiler generates a simple three-address code IR.

Example:

Source:

```c
c = a + b * 2;
```


IR:

```
%0 = b * 2
%1 = a + %0
c = %1
```


Temporary values use virtual registers:

```
%0
%1
%2
...
```


The IR is designed to make backend development easier.


---


# Backend


## Instruction Selection

The IR is translated into target instructions.

Example:

IR:

```
%1 = a + b
```

Possible machine instructions:

```
LOAD
ADD
STORE
```


---


## DAG Construction

Expressions are converted into dependency graphs.

Example:

```
a + b * c
```

DAG:

```
        +
       / \
      a   *
         / \
        b   c
```


The DAG represents instruction dependencies.


---


## Instruction Scheduling

Each instruction has an estimated latency.

| Instruction | Latency |
|-------------|---------|
| ADD | 1 |
| SUB | 1 |
| MUL | 2 |
| DIV | 10 |
| MOVE | 1 |
| LOAD | 3 |
| STORE | 3 |


Scheduling priority:

```
priority = critical_path_length
```


The goal is to reduce pipeline stalls.


---


## Register Allocation

The IR uses unlimited virtual registers:

```
%0
%1
%2
...
```


Register allocation maps virtual registers to physical registers.


Current target:

```
-O0
```

Focus:

- correctness
- simple allocation
- load/store based strategy


---


# Debug Options

The compiler provides several debugging stages:

```
-dump-token

-dump-ast

-dump-llvm

-dump-dag

-dump-schedule

-dump-regalloc

-emit-asm
```


Example:

```
vslc test.vsl --dump-ast
```


---


# Optimization Level

Currently the compiler targets:

```
-O0
```


-O0 goals:

- instruction selection
- basic scheduling
- simple register allocation
- assembly generation


-o1  goals:

- constant folding
- dead code elimination
- SSA form
- better register allocation
- peephole optimization


---


# Build

Example:

```bash
make
```


---


# Example


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


Generated IR:

```
%0 = 0
%1 = 1
%3 = 0
%4 = %3 + 2
%2 = %4
%1 = 1
%2 = 2
%5 = %1 + %2
return %5
```


---


# Project Status


Implemented:

- [x] Lexer
- [x] Parser
- [x] AST
- [x] Scope management
- [x] Symbol table
- [x] Semantic analysis
- [x] Three-address IR


In progress:

- [ ] Instruction selection
- [ ] Instruction scheduler
- [ ] Register allocation
- [ ] x86 backend


---




