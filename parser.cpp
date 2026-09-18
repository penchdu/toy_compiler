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
bool in_func_define = 0;

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
static Scope* new_cld_scp(bool is_virtual);
static Ast* parse_scope();
static Ast* case_tk_right_brace();
static Scope* case_tk_if();
static Scope* case_tk_else();

static Scope* new_cld_scp(bool is_virtual)
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
void new_virtual_scp()
{
	Token tk = tokens.peek();

	if (tk.type == TK_EOF
	    || tk.type == TK_INVALID
	    || tk.type == TK_IF
	    || tk.type == TK_ELSE
	    || tk.type == TK_WHILE
	    || tk.type == TK_BRACE_L
	    || tk.type == TK_BRACE_R)
	{
		return;
	}
	new_cld_scp(true);
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

	Scope *fscp = cur_scp;
	new_cld_scp(false);
	cur_scp->sem = SEM_FUNC_DEFINE;
	cur_scp->name = func_name;
	cur_scp->return_type = return_type;

	SymbolFunc *sym = new SymbolFunc;
	sym->return_type = return_type;
	sym->name = func_name;
	sym->argc = 0;
	sym->func_scope = cur_scp;

	fscp->func_table->insert( {func_name, sym});

	in_func_define = 1;
	parse_scope();
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
		tokens.get();
	else
		tokens.unget();

	return p;
}

static Ast* parse_expr_with_paren()
{
	tokens.get();

	Token tk = tokens.peek();
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
		LOG("%s \n", tk.src.c_str());

		if (tk.type == end_tk_ty)
			break;

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
static Ast* case_tk_semicolon()
{
	Token tk = tokens.get();
	return new Ast(tk);
}
static Scope* case_tk_else()
{
	LOG();
	Token tk = tokens.get();
	if (!in_func_define)
		ERR("%s not in func", tk.src.c_str());

	Scope *else_scp;
//	new_scope(false);
//	cur_scp->sem = SEM_ELSE;

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
		else_scp = new_cld_scp(false);
		else_scp->sem = SEM_ELSE;
		else_scp->name = "iff";
		parse_scope();
	}
	else
	{
		// only one express, create a real-scope to save it
		// must use real-scope because the express could be "int a = 0;"
		else_scp = new_cld_scp(false);
		else_scp->sem = SEM_ELSE;
		else_scp->name = "iff";
		Ast *p = parse_stmt();
		cur_scp->asts.push_back(p);

		cur_scp = cur_scp->parent;
		new_cld_scp(true);
	}

	return else_scp;
}
static Scope* get_tail_scope_of_ifelse(Scope *parent)
{
    vector<Scope*> &v = parent->clds;

    if (!v.empty() && v.back()->is_virtual_scope)
        return v.back();

    Scope *s = new Scope;
    s->parent = parent;
    s->is_virtual_scope = true;
    s->id = scope_id++;
    s->name = "jmp_target";
    s->sem = SEM_JMP_TARGET;

    v.push_back(s);
    return s;
}
static Scope* case_tk_if()
{
	LOG();
	Token tk = tokens.get();
	if (!in_func_define)
		ERR("%s not in func", tk.src.c_str());

	tk = tokens.peek();
	if (tk.type != TK_PAREN_L)
		ERR("unexpect token %s", tk.src.c_str());

	new_cld_scp(false);
	cur_scp->sem = SEM_IF;
	cur_scp->name = "ifc";

	// SEM_IF only have one condition express
	Ast *p = parse_expr_with_paren();
	cur_scp->asts.push_back(p);

	Scope *ifc = cur_scp;
	Scope *jmp_ift = 0;
	Scope *jmp_iff = 0;
	cur_scp = cur_scp->parent;

	// next scope is IF true body
	tk = tokens.peek();
	if (tk.type == TK_BRACE_L)
	{
		tokens.get();
		jmp_ift = new_cld_scp(false);
		jmp_ift->name = "ift";
		parse_scope();
	}
	else // tk.type != TK_ELSE
	{
		// only one express, create a real-scope to save it
		// must use real-scope because the express could be "int a = 0;"
		jmp_ift = new_cld_scp(false);
		jmp_ift->name = "ift";
		Ast *p = parse_stmt();
		cur_scp->asts.push_back(p);

		cur_scp = cur_scp->parent;
		new_virtual_scp();				// simulate behavior of parse_scope()
	}
	ifc->jmp_if_true = jmp_ift;

	// else
	tk = tokens.peek();
//	LOG("%s", tk.src.c_str());

	if (tk.type == TK_ELSE)
		jmp_iff = case_tk_else();
	else
		jmp_iff = cur_scp;		// case_tk_else() should call new_scope(true);

	ifc->jmp_if_false = jmp_iff;

	//
	jmp_ift->jmp_in.push_back(ifc);
	jmp_iff->jmp_in.push_back(ifc);

	Scope *tail = get_tail_scope_of_ifelse(jmp_ift->parent);
	jmp_ift->jmp_out = tail;
	jmp_iff->jmp_out = tail;
	tail->jmp_in.push_back(jmp_ift);
	tail->jmp_in.push_back(jmp_iff);

	return ifc;
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
		return case_tk_declare();		// todo declare -> express

	case TK_VAR:
		case TK_CONST_NUM:
		return parse_expr(TK_SEMICOLON);

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
		tokens.get();
		new_cld_scp(false);
		parse_scope();
		break;

	case TK_BRACE_R:
		ERR();
		case_tk_right_brace();
		break;

	case TK_RETURN:
		return case_tk_return();

	case TK_EOF:
		return 0;

	case TK_ELSE:
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

void _parse_scope(TokenType end_tk_ty)
{
	while (!tokens.empty())
	{
		Token tk = tokens.peek();
		if (tk.type == TK_BRACE_R)
		{
			case_tk_right_brace();
			return;
		}

		Ast *p = parse_stmt();
		if (p)
			cur_scp->asts.push_back(p);
	}
}
static Ast* parse_scope()
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

	_parse_scope(TK_BRACE_R);

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
	assert(cur_scp);
//	Token tk = tokens.peek();
//	if (tk.type == TK_INVALID)
//		ERR("%s is invalis token", tk.src.c_str());
//	if (tk.type == TK_EOF)
//		return 0;

	new_virtual_scp();
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

