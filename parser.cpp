/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"
#include "parser.h"

Scope file_scp;
Scope *current_scope_pointer;
int scope_id = 0;
bool in_func_define = 0;

#define STR_ITEM(name) STR(name),
const char *tk_ty_names[TK_EOF + 1] = {
    TOKEN_LIST(STR_ITEM)
};

const char *sem_ty_names[SEM_INVALID + 1] = {
    SEMANTIC_LIST(STR_ITEM)
};
#undef STR_ITEM

static Ast* parse_expr(TokenType end_tk_ty);
static void parse_file__gen_ast();

static Scope* new_cld_scp(bool is_virtual)
{
	PARSER_LOG("new %s scope %d", is_virtual ? "virtual" : "real", scope_id);

	if (current_scope_pointer->is_virtual_scope)
		exit_current_scope();
	assert(current_scope_pointer->is_virtual_scope == false);

	current_scope_pointer = current_scope_pointer->new_cld();
	current_scope_pointer->is_virtual_scope = is_virtual;
	current_scope_pointer->id = scope_id++;
	current_scope_pointer->name = "b" + std::to_string(current_scope_pointer->id);

	return current_scope_pointer;
}
Scope* new_scope_and_drop_in()
{
	PARSER_LOG();
	return new_cld_scp(false);
}
Scope* new_virtual_scp_and_drop_in()
{
	PARSER_LOG();
	Token tk = tokens.peek();
//	return new_cld_scp(true);

	if (
//		tk.type == TK_EOF
//	    || tk.type == TK_INVALID
//	    || tk.type == TK_IF
//	    || tk.type == TK_ELSE
//	    || tk.type == TK_WHILE
	    tk.type == TK_BRACE_L
	    || tk.type == TK_BRACE_R)
	{
		return 0;
	}
	return new_cld_scp(true);
}
void exit_current_scope()
{
	PARSER_LOG(" -> parent scope-%s-%d",
		current_scope_pointer->parent->name.c_str(),
		current_scope_pointer->parent->id);

	current_scope_pointer = current_scope_pointer->parent;
	assert(current_scope_pointer && current_scope_pointer->is_virtual_scope == false);
}
//Scope* new_brother_scp(bool is_virtual)
//{
//	Scope *parent = cur_scp->parent;
//	assert(parent);
//	assert(parent->is_virtual_scope == false);
//
//	Scope *p = new Scope;
//	parent->clds.push_back(p);
//	p->parent = parent;
//	p->is_virtual_scope = is_virtual;
//	p->id = scope_id++;
//	p->name = "b" + std::to_string(p->id);
//
//	cur_scp = p;
//	return p;
//}

static Ast* new_ast_node(Token t)
{
	Ast *p = new Ast(t);

	p->this_scp = current_scope_pointer;
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
	PARSER_LOG();
	Token tk = tokens.get();
	VarType return_type = get_var_type(tk.type);

	tk = tokens.get();
	string func_name = tk.src;

	// assume no argument
	tk = tokens.get();
	assert(tk.type == TK_PAREN_L);

	tk = tokens.get();
	assert(tk.type == TK_PAREN_R);

	tk = tokens.peek();
	assert(tk.type == TK_BRACE_L);

	assert(current_scope_pointer->sem == SEM_FILE_SCOPE);

	if (current_scope_pointer->var_table->find(func_name) != current_scope_pointer->var_table->end())
		ERR("%s already declared", func_name.c_str());

	if (current_scope_pointer->func_table->find(func_name) != current_scope_pointer->func_table->end())
		ERR("%s already declared", func_name.c_str());

	Scope *fscp = current_scope_pointer;

	new_scope_and_drop_in();

	current_scope_pointer->sem = SEM_FUNC_DEFINE;
	current_scope_pointer->name = func_name;
	current_scope_pointer->return_type = return_type;

	SymbolFunc *sym = new SymbolFunc;
	sym->return_type = return_type;
	sym->name = func_name;
	sym->argc = 0;
	sym->func_scope = current_scope_pointer;

	fscp->func_table->insert( {func_name, sym});

	in_func_define = 1;
	parse_scope();
	in_func_define = 0;
	return 0;
}

static Ast* case_tk_declare()
{
	Token tk_variable_type = tokens.get();
	PARSER_LOG("%s", tk_variable_type.src.c_str());

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
		tokens.get();
	else
		tokens.unget();

	return p;
}

Ast* parse_expr_with_paren()
{
	Token tk = tokens.get();
	PARSER_LOG("%s", tk.src.c_str());

	tk = tokens.peek();
	if (tk.type == TK_PAREN_R)
		ERR("unexpect token %s\n", tk.src.c_str());

	return parse_expr(TK_PAREN_R);
}
static Ast* _parse_expr(TokenType end_tk_ty)
{
	deque<Ast*> op_queue;
	deque<Ast*> operand_queue;

	while (!tokens.empty())
	{
		Token tk = tokens.get();
		PARSER_LOG("%s", tk.src.c_str());

		if (tk.type == end_tk_ty)
		{
			PARSER_LOG("end_tk %s", tk.src.c_str());
			break;
		}
		if (tk.type == TK_SEMICOLON)
		{
			if (end_tk_ty != TK_SEMICOLON)
				ERR("tk.type TK_SEMICOLON, end_tk_ty %s", tk_ty_names[end_tk_ty]);
		}

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
				PARSER_LOG("%s %s %s", left->tk.src.c_str(), pa->tk.src.c_str(), right->tk.src.c_str());

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
			Ast *t = parse_expr_with_paren();
			operand_queue.push_back(t);
		}
		else
		{
			ERR("invalid tk %s, end_tk_ty %s \n", tk.src.c_str(), tk_ty_names[end_tk_ty]);
		}
	}

	Token prev = tokens.prev();
	if (prev.type != end_tk_ty)
		ERR("expect tk %s\n", tk_ty_names[end_tk_ty]);

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
//	PARSER_LOG("%s \n", assign->tk.src.c_str());
//
//	assign->left = left;
//	assign->right = parse_expr(TK_SEMICOLON);
//	return assign;
//}
//static Ast* case_tk_variable()
//{
//	Token tk = tokens.get();
//	PARSER_LOG("%s \n", tk.src.c_str());
//
////	if(symble_table.find(tk.str) == symble_table.end()){
////		PARSER_LOG("%s not declared\n", tk.str.c_str());
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
static Ast* case_tk_semicolon()
{
	Token tk = tokens.get();
	return new Ast(tk);
}
void case_tk_eof()
{
	tokens.get();
	Scope *p = current_scope_pointer;
	if(current_scope_pointer->is_virtual_scope)
		p = current_scope_pointer->parent;

	if(p->sem != SEM_FILE_SCOPE)
		ERR("EOF in scope %s-%d\n", p->name.c_str(), p->id);
}
static Ast* case_tk_return()
{
	Token tk = tokens.get();
	if (!in_func_define)
		ERR("%s not in func", tk.src.c_str());

	PARSER_LOG("%s", tk.src.c_str());

	Ast *pa = new_ast_node(tk);
	Ast *left = parse_expr(TK_SEMICOLON);
	pa->left = left;

	assert(left);
	return pa;
}
Ast* parse_stmt()
{
	Token tk = tokens.peek();
	PARSER_LOG("%s", tk.src.c_str());

	if (tk.type < TK_OP_ALL)
		ERR();

	switch (tk.type)
	{
	case TK_INT:
		case TK_FLOAT:
		return case_tk_declare();		// todo declare -> express

	case TK_VAR:
		case TK_CONST_NUM:
		return parse_expr(TK_SEMICOLON);

	case TK_PAREN_L:
		return parse_expr_with_paren();

	case TK_SEMICOLON:
		return case_tk_semicolon();
		break;

	case TK_IF:
		case_tk_if();
		break;

	case TK_WHILE:
		ERR();
		break;

	case TK_BRACE_L:
		new_scope_and_drop_in();
		parse_scope();
		break;

	case TK_BRACE_R:
		ERR();
		case_tk_right_brace();
		break;

	case TK_RETURN:
		return case_tk_return();

	case TK_EOF:
		case_tk_eof();
		return 0;

	case TK_ELSE:
		case TK_ASSIGN:
		case TK_PAREN_R:
		case TK_INVALID:
		ERR("unexpect token %s", tk.src.c_str());
		break;

	default:
		ERR();
	}

	return 0;
}

Ast* parse_scope(bool eat)
{
	/*
	 * 1. eat '{', call new_scope(false);
	 * 2. set the cur_scp attribute
	 * 3. call parse_scope()
	 */

	if(eat)
		tokens.get();

	PARSER_LOG();
	if (!in_func_define)
		ERR("tk_left_brace not in func");
	assert(current_scope_pointer->is_virtual_scope == false);

	while (!tokens.empty())
	{
		Token tk = tokens.peek();
		if (tk.type == TK_BRACE_R)
		{
			case_tk_right_brace();
			break;
		}

		Ast *p = parse_stmt();
		if (p)
			current_scope_pointer->asts.push_back(p);

//		if (!current_scope_pointer->is_virtual_scope){
//			if(tk.type == TK_IF)
//				new_virtual_scp_and_drop_in();
//		}
	}

//	Token tk = tokens.prev();		// }
//	if (tk.type != TK_BRACE_R)
//		ERR("tk: %s, except }", tk.src.c_str());
	return 0;
}
Ast* case_tk_right_brace(bool eat)
{
	PARSER_LOG();
	if (current_scope_pointer->is_virtual_scope)
		exit_current_scope();

	if(eat)
		tokens.get();

	exit_current_scope();

	new_virtual_scp_and_drop_in();
	return 0;
}

static void parse_file__gen_ast()
{
	while (!tokens.empty())
	{
		Ast *p = parse_stmt();
		if (p)
			current_scope_pointer->asts.push_back(p);
	}
}
/*
 */
void parser()
{
	current_scope_pointer = &file_scp;
	current_scope_pointer->sem = SEM_FILE_SCOPE;
	current_scope_pointer->id = scope_id++;
	current_scope_pointer->name = "b" + std::to_string(current_scope_pointer->id);
	current_scope_pointer->is_virtual_scope = false;
	current_scope_pointer->parent = 0;

	parse_file__gen_ast();
	dump_ast();

}

