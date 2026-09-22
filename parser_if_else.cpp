/*
 * parser_if_else.cpp
 *
 *  Created on: 2026年9月19日
 *      Author: x
 */

#include "parser.h"
#include "scope.h"

Scope* case_tk_if()
{
	PARSER_LOG();
	Token tk = tokens.get();
	if (!in_func_define)
	ERR("%s not in func", tk.src.c_str());

	tk = tokens.peek();
	if (tk.stamp != TK_PAREN_L)
	ERR("unexpect token %s", tk.src.c_str());

	Scope *cond = new_scope_and_drop_in();
	current_scope_pointer->sem = SEM_IF;
	current_scope_pointer->name = "if";

	/////////////////////////////////// if-cond SEM_IF only have one condition express
	Ast *p = parse_expr_with_paren();
	if (!p) ERR();		// todo, must have a available expr
	current_scope_pointer->asts.push_back(p);
	exit_current_scope();

	///////////////////////////////// then branch
	Scope *then_branch = 0;
	tk = tokens.peek();
	if (tk.stamp == TK_BRACE_L)		// {...}
	{
		then_branch = new_scope_and_drop_in();
		current_scope_pointer->name = "then{}";
		parse_scope();
	}
	else // single stmt end with ';'
	{
		// only one express, create a real-scope to save it
		// must use real-scope because the express could be "int a = 0;"
		then_branch = new_scope_and_drop_in();
		current_scope_pointer->name = "then";
		Ast *p = parse_stmt();
		if (p)
		    current_scope_pointer->asts.push_back(p);

		// simulate behavior of parse_scope()
		// if not have this, code hehand will not have virtual-scp to store
		case_tk_right_brace(false);
	}
	cond->jmp_then = then_branch;
	then_branch->jmp_in.push_back(cond);

	/////////////////////////////////////////// else branch
	Scope *else_branch = 0;
	tk = tokens.peek();

	if (tk.stamp == TK_ELSE)
	{
		tokens.get();
		tk = tokens.peek();

//		if (tk.type == TK_IF)		// SEM_ELIF
//		{
//			else_branch = new_scope_and_drop_in();
////			else_branch = case_tk_if();
//			else_branch->sem = SEM_ELIF;
//			else_branch->name = "elif";
//			parse_scope(0);
//		} else
		if (tk.stamp == TK_BRACE_L)
		{
			else_branch = new_scope_and_drop_in();
			// else_branch->sem = SEM_ELSE;
			// else_branch->name = "else{}";
			parse_scope();
		}
		else
		{
			// only one express, create a real-scope to save it
			// must use real-scope because the express could be "int a = 0;"
			else_branch = new_scope_and_drop_in();
			else_branch->sem = SEM_ELSE;
			else_branch->name = "else";
			if (tk.stamp == TK_IF)
				else_branch->sem = SEM_ELSE;

			Ast *p = parse_stmt();
			if (p)
			    current_scope_pointer->asts.push_back(p);
			case_tk_right_brace(false);
		}
	}

	/////////////////////////////////////////////////// end work
	assert((!current_scope_pointer->is_virtual && current_scope_pointer == cond->parent)
	        || (current_scope_pointer->is_virtual && current_scope_pointer->parent == cond->parent));

	Scope *tail = current_scope_pointer;
	if (tail->is_virtual == false || tail->sem != SEM_JMP_UNIT)
	{
		tail = new_virtual_scp_and_drop_in();
		tail->sem = SEM_JMP_UNIT;
		tail->name = "if_jmp_tail";
	}
//	tail->sem = SEM_JMP_TARGET;
//	tail->name = "jmp_tail";

	if (else_branch)
		cond->jmp_else = else_branch;
	else
		cond->jmp_else = tail;

	//////////////////////////////////
	cond->jmp_out = then_branch->jmp_out = tail;
	tail->jmp_in.push_back(cond);
	tail->jmp_in.push_back(then_branch);

	// for elif, else_branch maybe a mix of else-br and if-br
	// else-br -> tail, and if-br -> tail, they are the same else_branch,
	// so tail may have 2 duplicate jmp-in
	if (else_branch)
	{
		assert(else_branch != tail);

		else_branch->jmp_in.push_back(cond);
		else_branch->jmp_out = tail;
		tail->jmp_in.push_back(else_branch);

//		if (else_branch->sem != SEM_JMP_TARGET)
		{
			assert(else_branch->clds.size());
			Scope *inner_tail = else_branch->clds.back();
			assert(inner_tail && inner_tail->is_virtual);
			assert(inner_tail->sem == SEM_JMP_UNIT);

			inner_tail->jmp_out = tail;
			tail->jmp_in.push_back(inner_tail);
		}
	}
	Scope *inner_tail;
	assert(then_branch->clds.size());
	inner_tail = then_branch->clds.back();	// then_branch.cld.back()
	assert(inner_tail && inner_tail->is_virtual);
	assert(inner_tail->sem == SEM_JMP_UNIT);

	inner_tail->jmp_out = tail;
	tail->jmp_in.push_back(inner_tail);

	auto &v = tail->jmp_in;
	std::sort(v.begin(), v.end());
	v.erase(std::unique(v.begin(), v.end()), v.end());

	return cond;
}

