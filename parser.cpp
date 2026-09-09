/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"

Ast* case_expr();

Ast* new_ast_node(Token t)
{
	Ast *p = new Ast (t);

	p->this_scp = scope;
	return p;
}
bool is_op(Sem_type op)
{
	return op >= op_assign && op <= op_div;
}
enum Var_type get_var_type(enum Token_type ty)
{
	switch(ty)
	{
	case tk_int:
		return INT;
	case tk_float:
		return FLOAT;
	default:
		return INVALID_TYPE;
	}
}

Ast* case_tk_left_brace()
{
	if(scope->is_virtual_scope)
		scope = scope->parent;

	tokens.get();
	scope = scope->new_cld();
	scope->is_virtual_scope = false;

	scope->id = scope_id++;
	scope->name = "b" + std::to_string(scope->id);

	printf("case_tk_left_curly_bracket \n");
	return 0;
}
Ast* case_tk_right_brace()
{
	tokens.get();
	if(scope->is_virtual_scope)
			scope = scope->parent;

	scope = scope->parent;		// go to parent scope
	scope = scope->new_cld();	// create a virual scope
	scope->is_virtual_scope = true;
	scope->id = scope_id++;
	scope->name = "b" + std::to_string(scope->id);

	// use parent's Symbol_var and Symbol_func
	scope->var_table = scope->parent->var_table;
	scope->func_table = scope->parent->func_table;

	printf("case_tk_right_curly_bracket \n");
	return 0;
}
int case_tk_func()
{
	Token tk = tokens.get();
	Var_type return_type = get_var_type(tk.type);

	tk = tokens.get();
	string func_name = tk.src_name;

	// assume no argument
	tk = tokens.get();
	assert(tk.type == tk_lparen);

	tk = tokens.get();
	assert(tk.type == tk_rparen);

	tk = tokens.get();
	assert(tk.type == tk_lbrace);

	Scope *parent = scope;
	scope = scope->new_cld();
	scope->is_virtual_scope = false;
	scope->sem = sem_func;
	scope->name = func_name;
	scope->id = scope_id++;
//	scope->name = "b" + std::to_string(scope->id);
//	parent->clds.push_back(scope);

	Symbol_func *sym = new Symbol_func;
	sym->return_type = return_type;
	sym->name = func_name;
	sym->argc = 0;
	sym->func_scope = scope;

	if(parent->func_table->find(func_name) != parent->func_table->end()){
		assert(0);
	}

    if(parent->func_table->find(func_name) != parent->func_table->end())
        ERR("function %s already declared", func_name.c_str());
	parent->func_table->insert({func_name, sym});

//	int left_curly = 1;
//	int right_curly = 0;
//	while(1)
//	{
//		tk = tokens.get();
//		if(tk.type == tk_left_curly_bracket)
//			left_curly++;
//		if(tk.type == tk_right_curly_bracket)
//			right_curly++;
//		if(left_curly == right_curly)
//			break;
//		parser();
//	}

	_parser();
	return 0;
}

Ast* case_tk_declare()
{
	LOG();
	Token tk_variable_type = tokens.get();
	printf("%s \n", tk_variable_type.src_name.c_str());

	Token tk_variable = tokens.get();

	if(tk_variable.type == tk_var && tokens.peek().type == tk_lparen){
		tokens.unget();
		tokens.unget();

		case_tk_func();		//todo define here?
		return 0;
	}

	// declare a variable
	Ast *p = new_ast_node(tk_variable);
	assert(p->semty == sem_var);
	p->semty = sem_declare;		// update default semty which have been set to sem_var
	p->var_type = get_var_type(tk_variable_type.type);

	if(tokens.peek().type == tk_semicolon){
		tokens.get();
	}else{
		tokens.unget();
	}

	return p;
}

Ast* case_tk_lparen()
{
	tokens.get();

	return case_expr();
}
Ast* case_expr()
{
	deque<Ast*> op_queue;
	deque<Ast*> operand_queue;

	while(!tokens.empty())
	{
		Token tk = tokens.get();
		printf("%s \n", tk.src_name.c_str());

		if(tk.type == tk_semicolon || tk.type == tk_rparen)
			break;

		if(tk.type < tk_op_all) {
			Token t = tokens.peek();
			if(t.type == tk_EOF ||
					!(t.type == tk_const_num
					|| t.type == tk_var
					|| t.type == tk_lparen))
				ERR("unexpect token %s\n", t.src_name.c_str());

			Ast *p = new_ast_node(tk);
			while(op_queue.size() > 0 && p->op_prio <= op_queue.back()->op_prio)
			{
				Ast *r = operand_queue.back();
				operand_queue.pop_back();

				Ast *l = operand_queue.back();
				operand_queue.pop_back();

				Ast *pa = op_queue.back();
				op_queue.pop_back();

				assert(pa && l && r);
				pa->left = l;
				pa->right = r;

				operand_queue.push_back(pa);
				printf("%s %s %s\n", l->src_name.c_str(), pa->src_name.c_str(), r->src_name.c_str());

			}

			op_queue.push_back(p);
		}
		else if(tk.type == tk_var || tk.type == tk_const_num) {
			Token t = tokens.peek();
			if(t.type == tk_EOF)
				ERR("unexpect EOF after %s\n", tokens.prew().src_name.c_str());

			if(!(t.type < tk_op_all
					|| t.type == tk_semicolon
					|| t.type == tk_rparen))
				ERR("unexpect token %s \n", t.src_name.c_str());

			Ast *p = new_ast_node(tk);
			operand_queue.push_back(p);
		}
		else if(tk.type == tk_lparen) {
			tokens.unget();
			Ast *t = case_tk_lparen();
			operand_queue.push_back(t);
		}
		else {
			assert(0);
		}
	}

	Token prew = tokens.prew();
	if(!(prew.type == tk_semicolon || prew.type == tk_rparen))
		ERR("expect semicolon or tk_rparen\n");

	while(!op_queue.empty())
	{
		Ast *r = operand_queue.back();
		operand_queue.pop_back();

		Ast *l = operand_queue.back();
		operand_queue.pop_back();

		Ast *parent = op_queue.back();
		op_queue.pop_back();

		assert(parent && l && r);

		parent->left = l;
		parent->right = r;

		operand_queue.push_back(parent);
	}
	assert(operand_queue.size() == 1);
	return operand_queue.back();
}
Ast* case_tk_assign()
{
	Ast *l = new_ast_node(tokens.prew());
	Ast *assign = new_ast_node(tokens.get());
	printf("%s \n", assign->src_name.c_str());

	assign->left = l;
	assign->right = case_expr();
	return assign;
}
Ast* case_tk_variable()
{
	Token tk = tokens.get();
	printf("%s \n", tk.src_name.c_str());

//	if(symble_table.find(tk.str) == symble_table.end()){
//		printf("%s not declared\n", tk.str.c_str());
//		assert(0);
//	}

	tk = tokens.peek();
//	if(tk.type == tk_semicolon)
//		return 0;

	if(tk.type == tk_assign)
		return case_tk_assign();

	tokens.unget();
	return case_expr();
}
Ast* case_tk_const_num()
{
	Token tk = tokens.get();
	Ast *p = new_ast_node(tk);
	return p;
}
Ast* case_tk_return()
{
	Token tk = tokens.get();
	printf("%s \n", tk.src_name.c_str());

	Ast *pa = new_ast_node(tk);
	Ast *l = case_expr();
	pa->left = l;

	assert(l);
	return pa;
}
Ast* parse_stmt()
{
	LOG();
	while(!tokens.empty())
	{
		Token token = tokens.peek();
		switch(token.type)
		{
		case tk_int:
		case tk_float:
			return case_tk_declare();

		case tk_var:
			return case_tk_variable();
			break;

		case tk_assign:
			assert(0);
			break;

		case tk_const_num:
			case_tk_const_num();
			break;

		case tk_semicolon:
			tokens.get();
			break;

		case tk_lbrace:
			case_tk_left_brace();
			break;

		case tk_rbrace:
			case_tk_right_brace();
			break;

		case tk_return:
			return case_tk_return();

		case tk_EOF:
			return 0;

		case tk_lparen:
		case tk_rparen:
		case tk_invalid:
			assert(0);
			break;

		default:
			assert(0);
		}
	}


	return 0;
}

void _parser()
{
	while(!tokens.empty()){
		Ast *p = parse_stmt();
		if(p)
			scope->asts.push_back(p);
	}

//	for(Ast * p : scope->asts){
//		check_semantic(p);
//	}
}
void parser()
{
	_parser();
	dump_ast();

	semantic_analysis(&file_scope);
	dump_ast();

	dump_ir();
}

