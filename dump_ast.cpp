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
static void print_blank(int n)
{
	n = std::max(n, 0);
	for (int i = 0; i < n; i++)
		printf("    ");
}
static string indent_str(int n)
{
	n = std::max(n, 0);
	return string(n * 4, ' ');
}

// 树形打印 AST 节点
static void dump_ast_node(Ast *p, const string &prefix, bool is_last)
{
	if (p == nullptr)
		return;

	string line_prefix = prefix + (is_last ? "└── " : "├── ");

	printf("%s", line_prefix.c_str());
	if (p->semty == SEM_VAR_DECLARE)
		printf("[VAR_DECL %s %s]\n", get_var_type_name(p->var_type).c_str(), p->tk.src.c_str());
	else if (p->semty < OP_ALL)
		printf("[OP %s %% %d]\n", p->tk.src.c_str(), p->vr_id);
	else if (p->semty == SEM_VAR)
	{
		if (!p->symb_var)
			printf("[VAR %s]\n", p->tk.src.c_str());
		else
			printf("[VAR %s %% %d]\n", p->symb_var->unique_name.c_str(), p->symb_var->vr);
	}
	else if (p->semty == SEM_CONST_NUM)
		printf("[NUM %d]\n", p->const_value);
	else if (p->semty == SEM_FUNC_CALL)
		printf("[FUNC_CALL %s]\n", p->tk.src.c_str());
	else if (p->semty == SEM_RETURN)
		printf("[RETURN %s]\n", p->tk.src.c_str());
	else
		ERR();

	string child_prefix = prefix + (is_last ? "    " : "│   ");

	if (p->left && p->right)
	{
		dump_ast_node(p->left, child_prefix, false);
		dump_ast_node(p->right, child_prefix, true);
	}
	else if (p->left)
	{
		dump_ast_node(p->left, child_prefix, true);
	}
	else if (p->right)
	{
		dump_ast_node(p->right, child_prefix, true);
	}
}
static void dump_scope(Scope *s, int depth)
{
	if (!s)
		return;

	print_blank(depth);
	printf("======%s, %d %d====== %s", s->name.c_str(), s->id,
		s->parent ? s->parent->id : 0,
		sem_ty_names[s->sem]);

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
		dump_ast_node(ast, indent_str(depth), true);
	}
	printf("\n");

	for (auto child : s->clds)
	{
		int d = depth + 1;
//		int d = s->is_virtual_scope ? depth : depth + 1;
		dump_scope(child, d);
	}
}
void dump_ast()
{
	printf("========== AST ==========\n");
	dump_scope(&file_scp, 0);
	printf("=========================\n");
}
