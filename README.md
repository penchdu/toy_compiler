# VSC - Very Simple Compiler

after i learned the book *Engineering a Compiler 3E*, i found the compiler is much more simpler than what i suppose **if you do not want to build a 
real-world compiler**, so i want to make one.

before i start coding i thought it will be very simple, when i am coding i found it is not that simple even it is a toy compiler, i think i can finish it with the help of AI.

i design and write the code, and sometimes i show the code to AI and ask if it discovered some bug, i ask AI to write some not-interesting code, for example, read a word from file, print the ast, i also ask AI for compiler questions.

i name the language vsl (very simple language)
so the compiler's name is vsc

The goal is use vsc to compile vsl source code to x86 or risc-v assembly, or directly to binary, and let it run on cpu.



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
=
+
-
*
/
(
)
{
}
int
return
void
;
```

### Language Restrictions
- only one function: `main` with no parameters
- no function calls
- no ABI handling
- only integer operations
- identifiers follow the same naming rules as in the C language

## Compiler Components
### 1. Lexer

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

### 2. Parser

Example:
Source:

```c
a + b * 2;
```

AST:

```
+
├── a
└── *
    ├── b
    └── 2
```


### 3. Scope and Semantic Analysis

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


### 4. Intermediate Representation


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

---


### 5. Instruction Selection

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


### 6. DAG Construction


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



### 7. Instruction Scheduling

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



---


### 8. Register Allocation

Current target:

```
-O0
```
- correctness
- simple allocation
- load/store based strategy


---


### 9. Debug Options

```
-dump-token

-dump-ast

-dump-llvm

-dump-dag

-dump-schedule

-dump-regalloc

-emit-asm
```


### 10. Optimization Level

Currently the compiler targets:

```
-O0
```


#### -O0 goals:

- instruction selection
- basic scheduling
- simple register allocation
- assembly generation


#### -o1 goals:

- constant folding
- dead code elimination
- SSA form
- better register allocation
- peephole optimization


## Project Status
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

### Example
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







