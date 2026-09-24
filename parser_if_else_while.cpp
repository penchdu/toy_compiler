/*
 * parser_if_else.cpp
 *
 *  Created on: 2026年9月19日
 *      Author: x
 */

#include "basic_block.h"
#include "parser.h"

SemOperator op_2_negative_cmp[] = {
    [OP_CMP_E] = OP_CMP_NE,
    [OP_CMP_NE] = OP_CMP_E,
    [OP_CMP_L] = OP_CMP_GE,
    [OP_CMP_LE] = OP_CMP_G,
    [OP_CMP_G] = OP_CMP_LE,
    [OP_CMP_GE] = OP_CMP_L,
};

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
	current_scope_pointer->sem = SEM_COND_JMP;
	current_scope_pointer->name = ".L_if" + std::to_string(current_scope_pointer->id);

	/////////////////////////////////// if-cond SEM_IF only have one condition express
	Ast *p = parse_expr_with_paren();
	if (!p)
		ERR();		// todo, must have a available expr

	// assume just one op, no "&&", "||"
	p->op = op_2_negative_cmp[p->op];

	current_scope_pointer->asts.push_back(p);
	exit_current_scope();

	///////////////////////////////// then branch
	Scope *then_branch = 0;
	tk = tokens.peek();
	if (tk.stamp == TK_BRACE_L)		// {...}
	{
		then_branch = new_scope_and_drop_in();
		current_scope_pointer->name = ".L_then" + std::to_string(current_scope_pointer->id);
		parse_scope();
	}
	else // single stmt end with ';'
	{
		// only one express, create a real-scope to save it
		// must use real-scope because the express could be "int a = 0;"
		then_branch = new_scope_and_drop_in();
		current_scope_pointer->name = ".L_then" + std::to_string(current_scope_pointer->id);
		Ast *p = parse_stmt();
		if (p)
			current_scope_pointer->asts.push_back(p);

		// simulate behavior of parse_scope()
		// if not have this, code hehand will not have virtual-scp to store
		case_tk_right_brace(false);
	}
//	cond->jmp_then = then_branch;
//	then_branch->jmp_in.push_back(cond);

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
			else_branch->name = ".L_else" + std::to_string(else_branch->id);
			parse_scope();
		}
		else
		{
			// only one express, create a real-scope to save it
			// must use real-scope because the express could be "int a = 0;"
			else_branch = new_scope_and_drop_in();
			else_branch->sem = SEM_ELSE;
			else_branch->name = ".L_else" + std::to_string(current_scope_pointer->id);
			if (tk.stamp == TK_IF)
				else_branch->sem = SEM_ELSE;

			Ast *p = parse_stmt();
			if (p)
				current_scope_pointer->asts.push_back(p);
			case_tk_right_brace(false);
		}
	}

	/////////////////////////////////////////////////// end work
	assert(
	    (!current_scope_pointer->is_virtual
	        && current_scope_pointer == cond->parent)
	        || (current_scope_pointer->is_virtual
	            && current_scope_pointer->parent == cond->parent));

	Scope *tail = current_scope_pointer;
	if (tail->is_virtual == false)	// || tail->sem != SEM_JMP)
	{
		tail = new_virtual_scp_and_drop_in();
//		tail->sem = SEM_JMP;
		tail->name = ".L_if_jmp_tail" + std::to_string(current_scope_pointer->id);
	}

	if (else_branch)
	{
		cond->jmp_out = else_branch;
		else_branch->jmp_in.push_back(cond);
	}
	else
	{
		cond->jmp_out = tail;
		tail->jmp_in.push_back(cond);
	}

	//////////////////////////////////
//	then_branch->jmp_out = tail;
//	tail->jmp_in.push_back(then_branch);

	// for elif, else_branch maybe a mix of else-br and if-br
	// else-br -> tail, and if-br -> tail, they are the same else_branch,
	// so tail may have 2 duplicate jmp-in
//	if (else_branch)
//	{
//		assert(else_branch->clds.size());
//		Scope *inner_tail = else_branch->clds.back();
//		assert(inner_tail->is_virtual);
//		assert(inner_tail->sem == SEM_JMP);
//
//		inner_tail->jmp_out = tail;
//		tail->jmp_in.push_back(inner_tail);
//	}
	Scope *then_branch_inner_tail;
	assert(then_branch->clds.size());
	then_branch_inner_tail = then_branch->clds.back();	// then_branch.cld.back()
	assert(then_branch_inner_tail->is_virtual);
	assert(then_branch_inner_tail->sem == SEM_JMP);

	then_branch_inner_tail->jmp_out = tail;
	tail->jmp_in.push_back(then_branch_inner_tail);

	auto &v = tail->jmp_in;
	std::sort(v.begin(), v.end());
	v.erase(std::unique(v.begin(), v.end()), v.end());

	return cond;
}

void case_tk_while()
{
	PARSER_LOG();
	Token tk = tokens.get();
	if (!in_func_define)
		ERR("%s not in func", tk.src.c_str());

	tk = tokens.peek();
	if (tk.stamp != TK_PAREN_L)
		ERR("unexpect token %s", tk.src.c_str());

	Scope *while_cond = new_scope_and_drop_in();
	current_scope_pointer->sem = SEM_COND_JMP;
	current_scope_pointer->name = ".L_while" + std::to_string(current_scope_pointer->id);

	/////////////////////////////////// while only have one condition express
	Ast *p = parse_expr_with_paren();
	if (!p)
		ERR();		// todo, must have a available expr

	// assume just one op, no "&&", "||"
	p->op = op_2_negative_cmp[p->op];

	current_scope_pointer->asts.push_back(p);
	exit_current_scope();

	///////////////////////////////// then branch
	Scope *while_body = 0;
	tk = tokens.peek();
	if (tk.stamp == TK_BRACE_L)		// {...}
	{
		while_body = new_scope_and_drop_in();
		current_scope_pointer->sem = SEM_WHILE_BODY;
		current_scope_pointer->name = ".L_while_body" + std::to_string(current_scope_pointer->id);
		parse_scope();
	}
	else // single stmt end with ';'
	{
		// only one express, create a real-scope to save it
		// must use real-scope because the express could be "int a = 0;"
		while_body = new_scope_and_drop_in();
		current_scope_pointer->sem = SEM_WHILE_BODY;
		current_scope_pointer->name = ".L_while_body" + std::to_string(current_scope_pointer->id);
		Ast *p = parse_stmt();
		if (p)
			current_scope_pointer->asts.push_back(p);

		// simulate behavior of parse_scope()
		// if not have this, code hehand will not have virtual-scp to store
		case_tk_right_brace(false);
	}

	/////////////////////////////////////////////////// end work
	assert((!current_scope_pointer->is_virtual && current_scope_pointer == while_cond->parent)
	    || (current_scope_pointer->is_virtual && current_scope_pointer->parent == while_cond->parent));

	Scope *tail = current_scope_pointer;
	if (tail->is_virtual == false)	// || tail->sem != SEM_JMP)
	{
		tail = new_virtual_scp_and_drop_in();
		//		tail->sem = SEM_JMP;
		tail->name = ".L_while_tail" + std::to_string(current_scope_pointer->id);
	}

	while_cond->jmp_out = tail;
	tail->jmp_in.push_back(while_cond);

	//////////////////////////////////
	//	then_branch->jmp_out = tail;
	//	tail->jmp_in.push_back(then_branch);

	// for elif, else_branch maybe a mix of else-br and if-br
	// else-br -> tail, and if-br -> tail, they are the same else_branch,
	// so tail may have 2 duplicate jmp-in
	//	if (else_branch)
	//	{
	//		assert(else_branch->clds.size());
	//		Scope *inner_tail = else_branch->clds.back();
	//		assert(inner_tail->is_virtual);
	//		assert(inner_tail->sem == SEM_JMP);
	//
	//		inner_tail->jmp_out = tail;
	//		tail->jmp_in.push_back(inner_tail);
	//	}
	Scope *while_body_inner_tail;
	assert(while_body->clds.size());
	while_body_inner_tail = while_body->clds.back();
	assert(while_body_inner_tail->is_virtual);
	assert(while_body_inner_tail->sem == SEM_JMP);

	while_body_inner_tail->jmp_out = while_cond;
	while_cond->jmp_in.push_back(while_body_inner_tail);

	/////////////////////////////////
	for (Scope *p : while_body->_continue_break)
	{
		if (p->sem == SEM_CONTINUE)
			p->jmp_out = while_cond;

		else if (p->sem == SEM_BREAK)
			p->jmp_out = tail;

		else
			ERR();

		p->sem = SEM_JMP;
	}

	auto &v = tail->jmp_in;
	std::sort(v.begin(), v.end());
	v.erase(std::unique(v.begin(), v.end()), v.end());

	return;
}
static Scope* case_tk_continue_break()
{
	Token tk = tokens.get();
	PARSER_LOG("%s", tk.src);

	Scope *while_body = current_scope_pointer;
	while (while_body && while_body->sem != SEM_WHILE_BODY)
	{
		while_body = while_body->parent;
	}

	if (!while_body)
		ERR();

	Scope *scp = new_scope_and_drop_in();
	while_body->_continue_break.push_back(current_scope_pointer);

	exit_current_scope();
	new_virtual_scp_and_drop_in();
	return scp;
}
void case_tk_continue()
{
	Scope *scp = case_tk_continue_break();
	scp->sem = SEM_CONTINUE;
	scp->name = "continue";
}
void case_tk_break()
{
	Scope *scp = case_tk_continue_break();
	scp->sem = SEM_BREAK;
	scp->name = "break";
}

