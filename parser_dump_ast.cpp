/*
 * dump_ast.cpp
 *
 *  Created on: 2026年9月17日
 *      Author: x
 */

#include "frontend.h"
#include "parser.h"
#include "scope.h"

static string get_var_type_name(Type ty)
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

static string indent_str(int n)
{
	n = std::max(n, 0);
	return string(n * 6, ' ');
}
static void print_blank(int n)
{
	n = std::max(n, 0);

	string s = indent_str(n);
	printf("%s", s.c_str());
}


// 树形打印 AST 节点
static void dump_ast_node(Ast *p, const string &prefix, bool is_last)
{
	if (p == nullptr)
		return;

	string line_prefix = prefix + (is_last ? "└── " : "├── ");

	printf("%s", line_prefix.c_str());
	if (p->sem == SEM_VAR_DECLARE)
		printf("[del %s]\n", Type_string[p->type]);
	else if (p->sem == SEM_OPERATOR)	// OP_ALL
		printf("[op %s %%%d]\n", p->tk.src.c_str(), p->vr_id);
	else if (p->sem == SEM_VAR)
	{
		if (!p->symb_var)
			printf("[var %s]\n", p->tk.src.c_str());
		else
			printf("[var %s %%%d]\n", p->symb_var->unique_name.c_str(), p->symb_var->vr);
	}
	else if (p->sem == SEM_CONST_NUM)
		printf("[num %d]\n", p->const_value);
	else if (p->sem == SEM_FUNC_CALL)
		printf("[func_call %s]\n", p->tk.src.c_str());
	else if (p->sem == SEM_RETURN)
		printf("[ret %s]\n", p->tk.src.c_str());
	else if (p->sem == SEM_NONE)
		printf("[none %s]\n", p->tk.src.c_str());
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
	printf("======%s=%d  parent=%d",
	    s->name.c_str(),
	    s->id,
	    s->parent ? s->parent->id : 0);

	if(s->sem == SEM_IF_COND)
	printf("  then=%d  else=%d",
		    s->jmp_then ? s->jmp_then->id : 0,
		    s->jmp_else ? s->jmp_else->id : 0);

	printf("====== %s out=%d",
		    s->sem == SEM_INVALID ? "" : Semantic_string[s->sem],
		    s->jmp_out ? s->jmp_out->id : 0);

	if (s->jmp_in.size())
		printf("  in=");
	for (auto p : s->jmp_in)
		printf("%d ", p->id);

	if (s->is_virtual)
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
		printf("ast:\n");
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
	printf("========== ast ==========\n");
	dump_scope(&file_scp, 0);
	printf("=========================\n");
}
