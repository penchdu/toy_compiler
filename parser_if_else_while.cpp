/*
 * parser_if_else.cpp
 *
 *  Created on: 2026年9月19日
 *      Author: x
 */

#include "basic_block.h"

Operator cmp_op_to_negative[] = {
    [OP_CMP_E] = OP_CMP_NE,
    [OP_CMP_NE] = OP_CMP_E,
    [OP_CMP_L] = OP_CMP_GE,
    [OP_CMP_LE] = OP_CMP_G,
    [OP_CMP_G] = OP_CMP_LE,
    [OP_CMP_GE] = OP_CMP_L,
};
static Scope* cond_to_negative()
{
	Token tk = tokens.peek();
	if (tk.stamp != TK_PAREN_L)
		ERR("unexpect token %s", tk.src.c_str());

	Scope *cond = new_scope_and_drop_in();
	cond->sem_stamp = SEM_COND_JMP;
	cond->name = ".L_" + cond->name;

	// if-cond SEM_IF only have one condition express
	Ast *p = parse_expr_with_paren();
	if (!p)
		ERR();

	// assume just one op, no "&&", "||"
	if (p->op >= OP_CMP_E && p->op <= OP_CMP_GE)
	{
		p->op = cmp_op_to_negative[p->op];
	}
	else
	{
		Ast *eq = new Ast({"==", TK_CMP_E});
		Ast *zero = new Ast({"0", TK_CONST_NUM});
		eq->left = zero;
		eq->right = p;
		p = eq;
	}

	current_scope_pointer->asts.push_back(p);
	exit_current_scope();
	return cond;
}
static Scope* body_of_if__while(enum Semantic sem_stamp = SEM_LABEL)
{
	Scope *branch = 0;
	Token tk = tokens.peek();
	if (tk.stamp == TK_BRACE_L)		// {...}
	{
		branch = new_scope_and_drop_in();
		branch->sem_stamp = sem_stamp;
		parse_scope();
	}
	else // single stmt end with ';'
	{
		// only one express, create a real-scope to save it
		// must use real-scope because the express could be "int a = 0;"
		branch = new_scope_and_drop_in();
		branch->sem_stamp = sem_stamp;
		Ast *p = parse_stmt();
		if (p)
			current_scope_pointer->asts.push_back(p);

		// simulate behavior of parse_scope()
		// if not have this, code hehand will not have virtual-scp to store
		case_tk_right_brace(false);
	}
	branch->name = ".L_" + branch->name;
	return branch;
}
static Scope* new_jmp_tail_for_cond_imp(Scope *cond)
{
//	Scope *tail = current_scope_pointer;
//	assert((!tail->is_virtual && tail == cond->parent)
//	        || (tail->is_virtual && tail->parent == cond->parent));

	Scope *tail = new_virtual_scope_and_drop_in();
	tail->sem_stamp = SEM_LABEL;
	tail->name = ".L_" + tail->name + "_jmp_tail";

	/*
	 * exit_current_scope();
	 * new_virtual_scp_and_drop_in();
	 *
	 */

//	if (tail->is_virtual == false)	// || tail->sem != SEM_JMP)
//	{
//		tail = new_virtual_scp_and_drop_in();
//		tail->sem_stamp = SEM_LABEL;
//		tail->name = ".L_" + tail->name + "_jmp_tail";
//	}
	return tail;
}
static Scope* get_inner_tail_of_scope(Scope *scope)
{
	assert(scope->clds.size());
	Scope *inner_tail = scope->clds.back();	// then_branch.cld.back()
	assert(inner_tail->is_virtual);
	assert(inner_tail->sem_stamp == SEM_JMP);
	assert(inner_tail->asts.size() == 0);
	return inner_tail;
}
static void rm_duplicit_jmp_in(Scope *tail)
{
	auto &v = tail->jmp_in;
	std::sort(v.begin(), v.end());
	v.erase(std::unique(v.begin(), v.end()), v.end());
}
void case_tk_if()
{
	PARSER_LOG();
	Token tk = tokens.get();
	if (!in_func_define)
		ERR("%s not in func", tk.src.c_str());

	// cond
	Scope *cond = cond_to_negative();
	cond->name += "_if";

	// then branch
	Scope *then_branch = body_of_if__while();
//	then_branch->sem_stamp = SEM_LABEL;
	then_branch->name += "_then";	// + to_string(cond->id);

	// else branch
	Scope *else_branch = 0;
	tk = tokens.peek();

	if (tk.stamp == TK_ELSE)
	{
		tokens.get();
		else_branch = body_of_if__while();
		else_branch->name += "_else";
	}

	// end work
	Scope *tail = new_jmp_tail_for_cond_imp(cond);
	tail->name += "_of_if";

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

	Scope *then_branch_inner_tail = get_inner_tail_of_scope(then_branch);
	then_branch_inner_tail->jmp_out = tail;
	tail->jmp_in.push_back(then_branch_inner_tail);

	rm_duplicit_jmp_in(tail);
	return;
}

void case_tk_while()
{
	PARSER_LOG();
	Token tk = tokens.get();
	if (!in_func_define)
		ERR("%s not in func", tk.src.c_str());

	// cond
	// while only have one condition express
	Scope *while_cond = cond_to_negative();
	while_cond->name += "_while";

	// body
	Scope *while_body = body_of_if__while(SEM_WHILE_BODY);
	while_body->name += "_while_body";

	// end work
	Scope *tail = new_jmp_tail_for_cond_imp(while_cond);
	tail->name += "_of_while";

	while_cond->jmp_out = tail;
	tail->jmp_in.push_back(while_cond);

	Scope *while_body_inner_tail = get_inner_tail_of_scope(while_body);
	while_body_inner_tail->jmp_out = while_cond;
	while_cond->jmp_in.push_back(while_body_inner_tail);

	//
	for (Scope *p : while_body->continue__break)
	{
		if (p->name == "continue")
			p->jmp_out = while_cond;
		else if (p->name == "break")
			p->jmp_out = tail;
		else
			ERR();
		p->sem_stamp = SEM_JMP;
	}

	rm_duplicit_jmp_in(tail);
	return;
}
void make_bb_terminate()
{
	// bb_terminate can eliminate some dead code, and dead jmp-label
	if (!enable_bb_terminate)
		return;

//	Scope *parent = current_scope_pointer;
	Scope *scp_terminate = new_virtual_scope_and_drop_in();
	scp_terminate->sem_stamp = SEM_bb_terminate;
	scp_terminate->name += "_SEM_bb_terminate";
	exit_current_scope();
}
static Scope* case_tk_continue_break()
{
	Token tk = tokens.get();
	PARSER_LOG("%s", tk.src.c_str());

	Scope *while_body = current_scope_pointer;
	while (while_body && while_body->sem_stamp != SEM_WHILE_BODY)
		while_body = while_body->parent;

	assert(while_body);

	Scope *scp = new_virtual_scope_and_drop_in();
	while_body->continue__break.push_back(scp);
	exit_current_scope();

	make_bb_terminate();

	new_virtual_scope_and_drop_in();
	return scp;
}
void case_tk_continue()
{
	Scope *scp = case_tk_continue_break();
	scp->name = "continue";
}
void case_tk_break()
{
	Scope *scp = case_tk_continue_break();
	scp->name = "break";
}

