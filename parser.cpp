/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"

extern Tokens tokens;

Scope file_scp;
Scope *cur_scp;
int scope_id = 0;
int in_func_define = 0;

#define STR_ITEM(name) STR(name),
const char *tk_ty_names[TK_EOF + 1] = {
    TOKEN_LIST(STR_ITEM)
};

const char *sem_ty_names[SEM_INVALID + 1] = {
    SEMANTIC_LIST(STR_ITEM)
};
#undef STR_ITEM

static Ast* parse_stmt();
static Ast* parse_expr(TokenType end_tk_ty);
static void parse_file__gen_ast();
static Scope* new_scope(bool is_virtual);
static Ast* case_tk_left_brace();
static Ast* case_tk_right_brace();

static Ast* new_ast_node(Token t)
{
	Ast *p = new Ast(t);

	p->this_scp = cur_scp;
	return p;
}
static enum VarType get_var_type(enum TokenType ty)
{
	switch (ty)
	{
	case TK_INT:
		return INT;
	case TK_FLOAT:
		return FLOAT;
	default:
		return INVALID_TYPE;
	}
}

static int case_tk_func()
{
	LOG();
	Token tk = tokens.get();
	VarType return_type = get_var_type(tk.type);

	tk = tokens.get();
	string func_name = tk.src;

	// assume no argument
	tk = tokens.get();
	assert(tk.type == TK_PAREN_L);

	tk = tokens.get();
	assert(tk.type == TK_PAREN_R);

	tk = tokens.get();
	assert(tk.type == TK_BRACE_L);

	assert(cur_scp->sem == SEM_FILE_SCOPE);

	if (cur_scp->var_table->find(func_name) != cur_scp->var_table->end())
		ERR("%s already declared", func_name.c_str());

	if (cur_scp->func_table->find(func_name) != cur_scp->func_table->end())
		ERR("%s already declared", func_name.c_str());

	Scope *file_scp = cur_scp;
	Scope *func_scp = new_scope(false);

//	func_scp->is_virtual_scope = false;
	func_scp->sem = SEM_FUNC_DEFINE;
	func_scp->name = func_name;
//	func_scp->id = scope_id++;
	func_scp->return_type = return_type;
//	new_scp->name = "b" + std::to_string(new_scp->id);

	SymbolFunc *sym = new SymbolFunc;
	sym->return_type = return_type;
	sym->name = func_name;
	sym->argc = 0;
	sym->func_scope = func_scp;

	file_scp->func_table->insert(
	    { func_name, sym });

//	cur_scp = func_scp;

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
	in_func_define = 1;
	case_tk_left_brace();
	in_func_define = 0;
	return 0;
}

static Ast* case_tk_declare()
{
	Token tk_variable_type = tokens.get();
	LOG("%s \n", tk_variable_type.src.c_str());

	Token tk_variable = tokens.get();

	if (tk_variable.type == TK_VAR && tokens.peek().type == TK_PAREN_L)
	{
		tokens.unget();
		tokens.unget();

		case_tk_func();		//todo define here?
		return 0;
	}

	// declare a variable
	Ast *p = new_ast_node(tk_variable);
	assert(p->semty == SEM_VAR);
	p->semty = SEM_VAR_DECLARE;		// update default semty which have been set to sem_var
	p->var_type = get_var_type(tk_variable_type.type);

	if (tokens.peek().type == TK_SEMICOLON)
	{
		tokens.get();
	}
	else
	{
		tokens.unget();
	}

	return p;
}

static Ast* case_tk_lparen()
{
	tokens.get();

	return parse_expr(TK_PAREN_R);
}
static Ast* _parse_expr(TokenType end_tk_ty)
{
	deque<Ast*> op_queue;
	deque<Ast*> operand_queue;

	while (!tokens.empty())
	{
		Token tk = tokens.get();
		LOG("%s \n", tk.src.c_str());

		if (tk.type == TK_SEMICOLON)
		{
			if (end_tk_ty != TK_SEMICOLON)
				ERR("tk.type TK_SEMICOLON, end_tk_ty %s", tk_ty_names[end_tk_ty]);
		}

		if (tk.type == end_tk_ty)
			break;

		if (tk.type < TK_OP_ALL)
		{
			Token t = tokens.peek();
			if (t.type == TK_EOF ||
			    !(t.type == TK_CONST_NUM
			        || t.type == TK_VAR
			        || t.type == TK_PAREN_L))
				ERR("unexpect token %s\n", t.src.c_str());

			Ast *p = new_ast_node(tk);
			while (op_queue.size() > 0 && p->op_prio <= op_queue.back()->op_prio)
			{
				Ast *right = operand_queue.back();
				operand_queue.pop_back();

				Ast *left = operand_queue.back();
				operand_queue.pop_back();

				Ast *pa = op_queue.back();
				op_queue.pop_back();

				assert(pa && left && right);
				pa->left = left;
				pa->right = right;

				operand_queue.push_back(pa);
				LOG("%s %s %s\n", left->tk.src.c_str(), pa->tk.src.c_str(), right->tk.src.c_str());

			}

			op_queue.push_back(p);
		}
		else if (tk.type == TK_VAR || tk.type == TK_CONST_NUM)
		{
			Token t = tokens.peek();
			if (t.type == TK_EOF)
				ERR("unexpect EOF after %s\n", tokens.prev().src.c_str());

			if (!(t.type < TK_OP_ALL
			    || t.type == TK_SEMICOLON
			    || t.type == TK_PAREN_R
			))
				ERR("unexpect token %s \n", t.src.c_str());

			Ast *p = new_ast_node(tk);
			operand_queue.push_back(p);
		}
		else if (tk.type == TK_PAREN_L)
		{
			tokens.unget();
			Ast *t = case_tk_lparen();
			operand_queue.push_back(t);
		}
		else
		{
			ERR("invalid tk %s, end_tk_ty %s \n", tk.src.c_str(), tk_ty_names[end_tk_ty]);
		}
	}

	Token prev = tokens.prev();
	if (!(prev.type == TK_SEMICOLON || prev.type == TK_PAREN_R))
		ERR("expect semicolon or tk_rparen\n");

	while (!op_queue.empty())
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
static Ast* parse_expr(TokenType end_tk_ty)
{
	Token tk = tokens.peek();
	if (tk.type < TK_OP_ALL)
		ERR("unexpect tk %s", tk_ty_names[tk.type]);

	return _parse_expr(end_tk_ty);
}
//static Ast* case_tk_assign()
//{
//	Ast *left = new_ast_node(tokens.prev());
//	Ast *assign = new_ast_node(tokens.get());
//	LOG("%s \n", assign->tk.src.c_str());
//
//	assign->left = left;
//	assign->right = parse_expr(TK_SEMICOLON);
//	return assign;
//}
//static Ast* case_tk_variable()
//{
//	Token tk = tokens.get();
//	LOG("%s \n", tk.src.c_str());
//
////	if(symble_table.find(tk.str) == symble_table.end()){
////		LOG("%s not declared\n", tk.str.c_str());
////		assert(0);
////	}
//
//	tk = tokens.peek();
////	if(tk.type == tk_semicolon)
////		return 0;
//
//	if (tk.type == TK_ASSIGN)
//		return case_tk_assign();
//
//	tokens.unget();
//	return parse_expr(TK_SEMICOLON);
//}
//static Ast* case_tk_const_num()
//{
//	return parse_expr(TK_SEMICOLON);
//}
static Ast* case_tk_else()
{
	Token tk = tokens.get();
	if (!in_func_define)
		ERR("%s not in func", tk.src.c_str());

	new_scope(false);
	cur_scp->sem = SEM_ELSE;

	tk = tokens.peek();
	if (tk.type == TK_IF)
	{

	}

	//	Scope *if_scp = cur_scp;
	cur_scp = cur_scp->parent;

	tk = tokens.peek();
	if (tk.type == TK_BRACE_L)
	{

	}
	else
	{
		// only one express, create a real-scope to save it
		// must use real-scope because the express could be "int a = 0;"
		new_scope(false);
		Ast *p = parse_stmt();
		cur_scp->asts.push_back(p);
		cur_scp = cur_scp->parent;
		new_scope(true);
	}

	return 0;
}
static Ast* case_tk_if()
{
	Token tk = tokens.get();
	if (!in_func_define)
		ERR("%s not in func", tk.src.c_str());

	new_scope(false);
	cur_scp->sem = SEM_IF;

	tk = tokens.peek();
	if (tk.type != TK_PAREN_L)
		ERR("unexpect token %s", tk.src.c_str());

	// SEM_IF only have one condition express
	Ast *p = case_tk_lparen();
	cur_scp->asts.push_back(p);

//	Scope *if_scp = cur_scp;
	cur_scp = cur_scp->parent;

	tk = tokens.peek();
	if (tk.type == TK_BRACE_L)
	{

	}
	else
	{
		// only one express, create a real-scope to save it
		// must use real-scope because the express could be "int a = 0;"
		new_scope(false);
		Ast *p = parse_stmt();
		cur_scp->asts.push_back(p);
		cur_scp = cur_scp->parent;
		new_scope(true);
	}

	tk = tokens.peek();
	if (tk.type == TK_ELSE)
	{
		case_tk_else();
	}

	return 0;
}
static Ast* case_tk_return()
{
	Token tk = tokens.get();
	if (!in_func_define)
		ERR("%s not in func", tk.src.c_str());

	LOG("%s \n", tk.src.c_str());

	Ast *pa = new_ast_node(tk);
	Ast *left = parse_expr(TK_SEMICOLON);
	pa->left = left;

	assert(left);
	return pa;
}
static Ast* parse_stmt()
{
	LOG("");
	Token tk = tokens.peek();
	if (tk.type < TK_OP_ALL)
		ERR();

	switch (tk.type)
	{
	case TK_INT:
		case TK_FLOAT:
		return case_tk_declare();

	case TK_VAR:
		return parse_expr(TK_SEMICOLON);
//			return case_tk_variable();

	case TK_CONST_NUM:
		return parse_expr(TK_SEMICOLON);
//			return case_tk_const_num();

	case TK_SEMICOLON:
		tokens.get();		// todo
		break;

	case TK_IF:
		case_tk_if();
		break;

	case TK_WHILE:
		ERR();
		break;

	case TK_BRACE_L:
		tokens.get();
		new_scope(false);
		case_tk_left_brace();
		break;

	case TK_BRACE_R:
		ERR();
		case_tk_right_brace();
		break;

	case TK_RETURN:
		return case_tk_return();

	case TK_EOF:
		return 0;

	case TK_ASSIGN:
		case TK_PAREN_L:
		case TK_PAREN_R:
		case TK_INVALID:
		ERR("unexpect token %s", tk.src.c_str());
		break;

	default:
		ERR();
	}

	return 0;
}

static Scope* new_scope(bool is_virtual)
{
	LOG("scope %s: new %s", cur_scp->name.c_str(), is_virtual ? "virtual scope" : "real scope");
//	assert(cur_scp->is_virtual_scope);

//	if (cur_scp->is_virtual_scope)
//		cur_scp = cur_scp->parent;
//	assert(cur_scp->is_virtual_scope == false);

	cur_scp = cur_scp->new_cld();
	cur_scp->is_virtual_scope = is_virtual;

	cur_scp->id = scope_id++;
	cur_scp->name = "b" + std::to_string(cur_scp->id);

	return cur_scp;
}

void parse_scope(TokenType end_tk_ty)
{
	while (!tokens.empty())
	{
		Token tk = tokens.peek();
		if (tk.type == TK_BRACE_R)
		{
//			tokens.get();
			case_tk_right_brace();
			return;
		}

		Ast *p = parse_stmt();
		if (p)
			cur_scp->asts.push_back(p);
	}
}
static Ast* case_tk_left_brace()
{
	/*
	 * 1. call new_scope(false);
	 * 2. set the cur_scp attribute
	 * 3. call case_tk_left_brace()
	 */

	LOG();
	if (!in_func_define)
		ERR("tk_left_brace not in func");

	assert(cur_scp->is_virtual_scope == false);

	parse_scope(TK_BRACE_R);

	Token tk = tokens.prev();		// }
	if (tk.type != TK_BRACE_R)
		ERR("tk: %s, except }", tk.src.c_str());

	return 0;
}
static Ast* case_tk_right_brace()
{
	LOG();
	if (cur_scp->is_virtual_scope)
		cur_scp = cur_scp->parent;
	assert(cur_scp->is_virtual_scope == false);

	tokens.get();
	cur_scp = cur_scp->parent;

	Token tk = tokens.peek();
	if (tk.type == TK_INVALID)
		ERR("%s is invalis token", tk.src.c_str());
	if (tk.type == TK_EOF)
		return 0;

	if (!(tk.type == TK_BRACE_L || tk.type == TK_BRACE_R || tk.type == TK_EOF))
		new_scope(true);

	return 0;
//
//	// go on leave parent scope
//	tokens.get();
//	cur_scp = cur_scp->parent;		// go to parent scope
//	assert(cur_scp->is_virtual_scope == false);
//
//	// now in grandpa scope
//	// if next tk is "}", return, if not, some expression is here, new a virtual scope
//	Token tk = tokens.peek();
//	if (tk.type != TK_BRACE_R)
//		new_scope(true);
//
//	return 0;
}
static void parse_file__gen_ast()
{
	while (!tokens.empty())
	{
		Ast *p = parse_stmt();
		if (p)
			cur_scp->asts.push_back(p);
	}
}

void parser()
{
	cur_scp = &file_scp;
	cur_scp->sem = SEM_FILE_SCOPE;
	cur_scp->id = scope_id++;
	cur_scp->name = "b" + std::to_string(cur_scp->id);
	cur_scp->is_virtual_scope = false;
	cur_scp->parent = 0;

	parse_file__gen_ast();
	dump_ast();

}

