/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "parser.h"

#include "basic_block.h"
#include "frontend.h"

Scope file_scp;
Scope *current_scope_pointer;
int scope_id = 0;
bool in_func_define = 0;


static Ast* parse_expr(TokenStamp end_tk_stamp);
static void parse_file__gen_ast();

static Scope* new_cld_scp(bool is_virtual)
{
	PARSER_LOG("new %s scope %d", is_virtual ? "virtual" : "real", scope_id);

	if (current_scope_pointer->is_virtual)
		exit_current_scope();
	assert(current_scope_pointer->is_virtual == false);

	current_scope_pointer = current_scope_pointer->new_cld();
	current_scope_pointer->is_virtual = is_virtual;
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
//	Token tk = tokens.peek();
	return new_cld_scp(true);
}
void exit_current_scope()
{
	PARSER_LOG(" -> parent scope-%s-%d",
			current_scope_pointer->parent->stamp.c_str(),
			current_scope_pointer->parent->id);

	current_scope_pointer = current_scope_pointer->parent;
	assert(current_scope_pointer && current_scope_pointer->is_virtual == false);
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
enum Type get_declare_type(enum TokenStamp ty)
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
	Type return_type = get_declare_type(tk.stamp);

	tk = tokens.get();
	string func_name = tk.src;

	// assume no argument
	tk = tokens.get();
	assert(tk.stamp == TK_PAREN_L);

	tk = tokens.get();
	assert(tk.stamp == TK_PAREN_R);

	tk = tokens.peek();
	assert(tk.stamp == TK_BRACE_L);

	Scope *cur_file_scope = current_scope_pointer;
	assert(cur_file_scope->sem_stamp == SEM_FILE_SCOPE);

	if (cur_file_scope->var_table->find(func_name) != cur_file_scope->var_table->end())
		ERR("%s already declared", func_name.c_str());

	if (cur_file_scope->func_table->find(func_name) != cur_file_scope->func_table->end())
		ERR("%s already declared", func_name.c_str());

	Scope *func = new_scope_and_drop_in();
	func->sem_stamp = SEM_FUNC_DEFINE;
	func->name = func_name;
	func->return_type = return_type;
	func->return_label = new Scope;
	func->return_label->id = -1; //scope_id++;
	func->return_label->name = ".L_return";
	func->return_label->parent = func;

	SymbolFunc *sym = new SymbolFunc;
	sym->return_type = return_type;
	sym->name = func_name;
	sym->argc = 0;
	sym->func_scope = current_scope_pointer;

	cur_file_scope->func_table->insert( {func_name, sym});

	in_func_define = 1;
	parse_scope();
	in_func_define = 0;
	return 0;
}
void case_tk_return()
{
	Token tk = tokens.get();
	if (!in_func_define)
		ERR("%s not in func", tk.src.c_str());

	PARSER_LOG("%s", tk.src.c_str());

	Scope *func = current_scope_pointer;
	while (func && func->sem_stamp != SEM_FUNC_DEFINE)
		func = func->parent;

	if (!func)
		ERR();

	Ast *expr = parse_expr(TK_SEMICOLON);
	if(!expr)
		ERR("only support return int");
//	current_scope_pointer->asts.push_back(expr);
	Ast *p = new_ast_node(tk);
//	p->home_scp = func;		// Ast(return) not create a vr. let home_scp = func;
	p->left = expr;
	current_scope_pointer->asts.push_back(p);

	Scope *jmp = new_scope_and_drop_in();
	jmp->sem_stamp = SEM_JMP;
	jmp->name = "jmp_to_ret";
	jmp->jmp_out = func->return_label;
	exit_current_scope();

	if (enable_bb_terminate)
		make_bb_terminate();

	return;
}
static Ast* case_tk_declare()
{
	Token ttk = tokens.get();
	PARSER_LOG("%s", stamp.src.c_str());

	Token vtk = tokens.get();

	if (vtk.stamp == TK_VAR && tokens.peek().stamp == TK_PAREN_L)
	{
		tokens.unget();
		tokens.unget();

		case_tk_func();		//todo define here?
		return 0;
	}

	// declare a variable
	Ast *ty = new_ast_node(ttk);
	Ast *var = new_ast_node(vtk);
	ty->left = var;
//	assert(var->semty == SEM_VAR);
//	var->semty = SEM_VAR_DECLARE;		// update default semty which have been set to sem_var
//	var->var_type = get_var_type(type.type);

	Token tk = tokens.get();
	if (tk.stamp == TK_SEMICOLON)
		return ty;

	tokens.unget();
	tokens.unget();
	return ty;

//	if (tk.stamp != TK_ASSIGN)
//		ERR("%s %s %s", ttk.src.c_str(), vtk.src.c_str(), tk.src.c_str());
//
//	tokens.unget();
//	var->right = parse_expr(TK_SEMICOLON);
//	return ty;
}

Ast* parse_expr_with_paren()
{
	Token tk = tokens.get();
	PARSER_LOG("%s", tk.src.c_str());

	tk = tokens.peek();
	if (tk.stamp == TK_PAREN_R)
		ERR("unexpect token %s\n", tk.src.c_str());

	return parse_expr(TK_PAREN_R);
}
static Ast* _parse_expr(TokenStamp end_tk_ty)
{
	deque<Ast*> op_queue;
	deque<Ast*> operand_queue;

	while (!tokens.empty())
	{
		Token tk = tokens.get();
		PARSER_LOG("%s", tk.src.c_str());

		if (tk.stamp == end_tk_ty)
		{
			PARSER_LOG("end_tk %s", tk.src.c_str());
			break;
		}
		if (tk.stamp == TK_SEMICOLON)
		{
			if (end_tk_ty != TK_SEMICOLON)
				ERR("tk.type TK_SEMICOLON, end_tk_ty %s", TokenStamp_string[end_tk_ty]);
		}

		if (tk.stamp < TK_OP_ALL)
		{
			Token t = tokens.peek();
			if (t.stamp == TK_EOF ||
			        !(t.stamp == TK_CONST_NUM
			                || t.stamp == TK_VAR
			                || t.stamp == TK_PAREN_L))
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
		else if (tk.stamp == TK_VAR || tk.stamp == TK_CONST_NUM)
		{
			Token t = tokens.peek();
			if (t.stamp == TK_EOF)
				ERR("unexpect EOF after %s\n", tokens.prev().src.c_str());

			if (!(t.stamp < TK_OP_ALL
			        || t.stamp == TK_SEMICOLON
			        || t.stamp == TK_PAREN_R
			))
				ERR("unexpect token %s \n", t.src.c_str());

			Ast *p = new_ast_node(tk);
			operand_queue.push_back(p);
		}
		else if (tk.stamp == TK_PAREN_L)
		{
			tokens.unget();
			Ast *t = parse_expr_with_paren();
			operand_queue.push_back(t);
		}
		else
		{
			ERR("invalid tk %s, end_tk_ty %s \n", tk.src.c_str(), TokenStamp_string[end_tk_ty]);
		}
	}

	Token prev = tokens.prev();
	if (prev.stamp != end_tk_ty)
		ERR("expect tk %s\n", TokenStamp_string[end_tk_ty]);

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
static Ast* parse_expr(TokenStamp end_tk_stamp)
{
	Token tk = tokens.peek();

	if(tk.stamp == TK_SEMICOLON){
		if(end_tk_stamp != TK_SEMICOLON)
			ERR();

		tokens.get();
		return 0;
	}

	if (tk.stamp < TK_OP_ALL)
		ERR("unexpect tk %s", TokenStamp_string[tk.stamp]);

	if (!(tk.stamp == TK_CONST_NUM
	        || tk.stamp == TK_VAR
	        || tk.stamp == TK_PAREN_L))
		ERR("unexpect tk %s", TokenStamp_string[tk.stamp]);

	return _parse_expr(end_tk_stamp);
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
	// maybe "if(...);" need semicolon to make a empty ast
	Token tk = tokens.get();
	return 0;
	return new Ast(tk);
}
void case_tk_eof()
{
	tokens.get();
	Scope *p = current_scope_pointer;
	if (current_scope_pointer->is_virtual)
		p = current_scope_pointer->parent;

	if (p->sem_stamp != SEM_FILE_SCOPE)
		ERR("EOF in scope %s-%d\n", p->name.c_str(), p->id);
}

void parse_scope(bool eat)
{
	/*
	 * 1. eat '{', call new_scope(false);
	 * 2. set the cur_scp attribute
	 * 3. call parse_scope()
	 */

	if (eat)
	{
		Token tk = tokens.get();
		if (tk.stamp != TK_BRACE_L)
			ERR();
	}

	PARSER_LOG();
	if (!in_func_define)
		ERR("tk_left_brace not in func");
	assert(current_scope_pointer->is_virtual == false);

	while (!tokens.empty())
	{
		Token tk = tokens.peek();
		if (tk.stamp == TK_BRACE_R)
		{
			case_tk_right_brace();
			break;
		}

		Ast *p = parse_stmt();
		if (p)
			current_scope_pointer->asts.push_back(p);
	}
}
void case_tk_right_brace(bool eat)
{
	PARSER_LOG();

	Scope *tail = new_virtual_scp_and_drop_in();
	tail->sem_stamp = SEM_JMP;
	tail->name += "_scp_tail";
	exit_current_scope();

//	if (current_scope_pointer->is_virtual)
//		exit_current_scope();
	assert(current_scope_pointer->is_virtual == false);
	Scope *scp = current_scope_pointer;

	if(scp->sem_stamp == SEM_FUNC_DEFINE)
		scp->return_label->id = scope_id++;

	if (eat)
		tokens.get();	// '}'
	exit_current_scope();

	Token tk = tokens.peek();
	if (!(tk.stamp == TK_BRACE_L || tk.stamp == TK_BRACE_R
	        || tk.stamp == TK_EOF
	        //	    || tk.type == TK_INVALID
	        || tk.stamp == TK_IF || tk.stamp == TK_ELSE
	        || tk.stamp == TK_WHILE
	))
	{
		new_virtual_scp_and_drop_in();
	}

	PARSER_LOG();
	return;
}
Ast* parse_stmt()
{
	Token tk = tokens.peek();
	PARSER_LOG("%s", tk.src.c_str());

	if (tk.stamp < TK_OP_ALL)
		ERR();

	switch (tk.stamp)
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
		case_tk_while();
		break;

	case TK_CONTINUE:
		case_tk_continue();
		break;

	case TK_BREAK:
		case_tk_break();
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
		case_tk_return();
		break;

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
static void sweep_dead_virtual_scope(Scope *scp)
{
	for (auto c : scp->clds)
		sweep_dead_virtual_scope(c);

	int i = 0;
	while (i < (int) scp->clds.size())
	{
		Scope *c = scp->clds[i];
		if (c->is_virtual
		        && c->asts.empty()
		        && c->clds.empty()
		        && c->jmp_in.empty()
		        && c->jmp_out == 0
		        && c->sem_stamp != SEM_bb_terminate)
		{
			printf("sweep_dead_virtual_scope: id=%d name=%s\n", c->id, c->name.c_str());
			scp->clds.erase(scp->clds.begin() + i);
			delete c;
			continue;
		}
		i++;
	}
}
void parser()
{
	current_scope_pointer = &file_scp;
	current_scope_pointer->sem_stamp = SEM_FILE_SCOPE;
	current_scope_pointer->id = scope_id++;
	current_scope_pointer->name = "b" + std::to_string(current_scope_pointer->id);
	current_scope_pointer->is_virtual = false;
	current_scope_pointer->parent = 0;

	parse_file__gen_ast();

	sweep_dead_virtual_scope(&file_scp);
//	dump_ast();

}

