/*
 * parser_if_else.cpp
 *
 *  Created on: 2026年9月19日
 *      Author: x
 */

#include "parser.h"

static Scope* get_jmp_tail_scope_of_ifelse(Scope *parent)
{
	PARSER_LOG();
	vector<Scope*> &v = parent->clds;

	if (!v.empty() && v.back()->is_virtual_scope){
		v.back()->sem = SEM_JMP_TARGET;
		v.back()->name = "jmp_target";
		return v.back();
	}
//	printf("parent %s %d, cur virtual=%d  %zu \n",
//		parent->name.c_str(), parent->id, cur_scp->is_virtual_scope,
//		v.size());
//
//	printf("parent %s %d, cur virtual=%d, cld size=%d, %s %d \n",
//		parent->name.c_str(), parent->id, cur_scp->is_virtual_scope,
//		v.size(), v.back()->name.c_str(), v.back()->is_virtual_scope);

//	ERR("s %s", cur_scp->parent->name.c_str());

	Scope *s = new Scope;
	s->parent = parent;
	s->is_virtual_scope = true;
	s->id = scope_id++;
	s->name = "jmp_target";
	s->sem = SEM_JMP_TARGET;

	v.push_back(s);
	return s;
}

Scope* case_tk_else()
{
	LOG();
	Token tk = tokens.get();
	if (!in_func_define)
		ERR("%s not in func", tk.src.c_str());

	Scope *else_scp;
	tk = tokens.peek();

	if (tk.type == TK_IF)		// SEM_ELIF
	{
		else_scp = case_tk_if();
		else_scp->sem = SEM_ELIF;
		else_scp->name = "elif";
		return else_scp;
	}

	if (tk.type == TK_BRACE_L)
	{
		tokens.get();
		else_scp = new_scope_and_drop_in();
		else_scp->sem = SEM_ELSE;
		else_scp->name = "cond_false";
		parse_scope(false);
	}
	else
	{
		// only one express, create a real-scope to save it
		// must use real-scope because the express could be "int a = 0;"
		else_scp = new_scope_and_drop_in();
		else_scp->sem = SEM_ELSE;
		else_scp->name = "cond_false";

		Ast *p = parse_stmt();
		current_scope_pointer->asts.push_back(p);

		case_tk_right_brace(false);
	}

	return else_scp;
}

Scope* case_tk_if()
{
	LOG();
	Token tk = tokens.get();
	if (!in_func_define)
		ERR("%s not in func", tk.src.c_str());

	tk = tokens.peek();
	if (tk.type != TK_PAREN_L)
		ERR("unexpect token %s", tk.src.c_str());

	Scope *cond = new_scope_and_drop_in();
	current_scope_pointer->sem = SEM_IF;
	current_scope_pointer->name = "if_cond";

	// SEM_IF only have one condition express
	Ast *p = parse_expr_with_paren();
	current_scope_pointer->asts.push_back(p);
	exit_current_scope();

	Scope *true_branch = 0;
	Scope *false_branch = 0;

	// next scope is true branch
	tk = tokens.peek();
	if (tk.type == TK_BRACE_L)		// {...}
	{
		tokens.get();
		true_branch = new_scope_and_drop_in();
		current_scope_pointer->name = "cond_true";
		parse_scope(false);
	}
	else // single stmt end with ';', tk.type != TK_ELSE
	{
		// only one express, create a real-scope to save it
		// must use real-scope because the express could be "int a = 0;"
		true_branch = new_scope_and_drop_in();
		current_scope_pointer->name = "cond_true";
		Ast *p = parse_stmt();
		current_scope_pointer->asts.push_back(p);

		// simulate behavior of parse_scope()
		// if not have this, code hehand will not have virtual-scp to store
		case_tk_right_brace(false);
	}
	cond->jmp_to_if_true = true_branch;
	true_branch->jmp_in.push_back(cond);

	// else
	tk = tokens.peek();
//	LOG("%s", tk.src.c_str());

	if (tk.type == TK_ELSE)
		false_branch = case_tk_else();

//	assert(current_scope_pointer->is_virtual_scope);
////	 case_tk_else() should call new_scope(true);
	Scope *tail = get_jmp_tail_scope_of_ifelse(cond->parent);


	if (!false_branch)
		false_branch = tail;

	cond->jmp_to_if_false = false_branch;

	//
	cond->jmp_out = true_branch->jmp_out = tail;
	tail->jmp_in.push_back(cond);
	tail->jmp_in.push_back(true_branch);

	if (false_branch != tail)
	{
		false_branch->jmp_in.push_back(cond);

		false_branch->jmp_out = tail;
		tail->jmp_in.push_back(false_branch);
	}

	return cond;
}

