/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

// i am learning compiler these days , after i read
/*
 * Module
 └── Function
      └── BasicBlock
           └── Instruction
*/
/*
 *target: use vslc(very simple language compiler) to compile vsl(very simple language), run on x86
 * type: int
 * op: '=', '+', '-', '*', '/', '(', ')', '{', '}', "return", "void"
 * other: ';',
 * name: '_ | a-z | A-Z' + '_ | a-z | A-Z | 0-9'
 *just one main function: void main(... return;), so not care about ABI
 *	only one function: main
	no function calls
	no parameters
	no ABI
 *
 *
 *asm:  instruction  assume_latency
 *asm:  ADD 1
 *asm:  SUB 1
 *asm:  MUL 2
 *asm:  DIV 10
 *asm:  MOVE 1
 *asm:  LOAD 3
 *asm:  STORE 3
 *
 *
 *lexer
 *parser
 *
 *ast
 *llvm ir
 *dag critical_path_length
 *
 *inst select
 *inst Schedule   priority = critical_path_length
 *reg alloc
 *
 */

//////////////////////////////////////////////////////////

/*
 * target: use vslc (very simple language compiler)
 *         to compile vsl (very simple language)
 *         and run on x86
 *
 * type:
 *   int
 *
 * operators:
 *   =
 *   +
 *   -
 *   *
 *   /
 *   (
 *   )
 *   {
 *   }
 *
 * keywords:
 *   return
 *   void
 *
 * other:
 *   ;
 *
 * identifier:
 *   _ | a-z | A-Z
 *   followed by:
 *   _ | a-z | A-Z | 0-9
 *
 * language restriction:
 *   only one function: void main()
 *   no function calls
 *   no parameters
 *   no ABI
 *
 *
 * target instructions:
 *
 *   ADD    latency = 1
 *   SUB    latency = 1
 *   MUL    latency = 2
 *   DIV    latency = 10
 *   MOVE   latency = 1
 *   LOAD   latency = 3
 *   STORE  latency = 3
 *
 *
 * compiler pipeline:
 *
 *   Lexer
 *      ↓
 *   Parser
 *      ↓
 *   AST
 *      ↓
 *   LLVM IR
 *      ↓
 *   Instruction Selection
 *      ↓
 *   DAG
 *      ↓
 *   Instruction Scheduling
 *      ↓
 *   Register Allocation
 *      ↓
 *   x86
 *
 * scheduling:
 *   priority = critical_path_length
 *
 *
 *--dump-token
--dump-ast
--dump-llvm
--dump-dag
--dump-schedule
--dump-regalloc
--emit-asm
 */

