/*
 * dump_ast.cpp
 *
 *  Created on: 2026年9月17日
 *      Author: x
 */

#include "h.h"

//extern Tokens tokens;
extern const char *tk_ty_names[];
extern const char *sem_ty_names[];

extern Scope file_scp;
//extern Scope *cur_scp;
//extern int scope_id;

static void print_blank(int n)
{
	n = std::max(n, 0);
	for (int i = 0; i < n; i++)
		printf("    ");
}
static string get_var_type_name(VarType ty)
{
	switch (ty)
	{
	case INT:
		return "int";
	case FLOAT:
		return "float";
	case VOID:
		return "void";
	case INVALID_TYPE:
		default:
		return "INVALID_TYPE";
	}
}
static void dump_ast_node(Ast *p, int depth)
{
	if (p == nullptr)
		return;

	print_blank(depth);
//	printf("%s", p->src_name.c_str());

	if (p->semty == SEM_VAR_DECLARE)
	{
		printf(" [%s %s]\n", get_var_type_name(p->var_type).c_str(), p->tk.src.c_str());
	}
	else if (p->semty < OP_ALL)
	{
		printf("%s [%%%d]\n", p->tk.src.c_str(), p->vr_id);
		print_blank(depth);
		printf("\n");
	}
	else if (p->semty == SEM_VAR)
	{
		if (!p->symb_var)
			printf("%s [ ]\n", p->tk.src.c_str());
		else
			printf("%s [%%%d]\n", p->symb_var->unique_name.c_str(), p->symb_var->vr);
	}
	else if (p->semty == SEM_CONST_NUM)
	{
		printf("%d [num]\n", p->const_value);
	}
	else if (p->semty == SEM_FUNC_CALL)
	{
		printf("func [%s]\n", p->tk.src.c_str());
	}
	else if (p->semty == SEM_RETURN)
	{
		printf("%s \n", p->tk.src.c_str());
	}
	else
	{
		ERR();
	}

	if (p->left)
	{
		print_blank(depth);
		printf("L: \n");
		dump_ast_node(p->left, depth + 1);
	}

	if (p->right)
	{
		print_blank(depth);
		printf("R: \n");
		dump_ast_node(p->right, depth + 1);
	}
}
static void dump_scope(Scope *s, int depth)
{
	if (!s)
		return;

	if (!s->is_virtual_scope)
		print_blank(depth);
	else
		print_blank(depth - 1);

	printf("===%s, %d=== %s", s->name.c_str(), s->id, sem_ty_names[s->sem]);
	if (s->is_virtual_scope)
		printf(" virtual");
	printf("\n");

	for (auto &it : *(s->var_table))
	{
		print_blank(depth);
		printf("symbol: %s\n", it.second->unique_name.c_str());
	}

	for (auto ast : s->asts)
	{
		print_blank(depth);
		printf("AST:\n");
		dump_ast_node(ast, depth + 1);
	}
	printf("\n");

	for (auto child : s->clds)
	{
		dump_scope(child, depth + 1);
	}
}
void dump_ast()
{
	printf("========== AST ==========\n");
	dump_scope(&file_scp, 0);
	printf("=========================\n");
}
