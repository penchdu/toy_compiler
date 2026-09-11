/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"

string get_var_type_name(Var_type ty)
{
	switch(ty)
	{
	case INT:
		return "INT";
	case FLOAT:
			return "FLOAT";
	case VOID:
			return "VOID";
	case INVALID_TYPE:
	default:
		return "INVALID_TYPE";
	}
}

static void print_blank(int n)
{
	n = std::max(n, 0);
	for(int i = 0; i < n; i++)
		printf("    ");
}
static void dump_ast_node(Ast *p, int depth)
{
	if(p == nullptr)
		return;

	print_blank(depth);
//	printf("%s", p->src_name.c_str());

	if(p->semty == sem_var_declare){
		printf(" [%s %s]\n", get_var_type_name(p->var_type).c_str(), p->tk.src.c_str());
	}
	else if(p->semty < op_all){
		printf("%s [%%%d]\n", p->tk.src.c_str(), p->vr);
		print_blank(depth);
		printf("\n");
	}
	else if(p->semty == sem_var){
		if(!p->symb_var)
			printf("%s [ ]\n", p->tk.src.c_str());
		else
			printf("%s [%%%d]\n", p->symb_var->unique_name.c_str(), p->symb_var->vr);
	}
	else if(p->semty == sem_const_num){
		printf("%d [num]\n", p->const_value);
	}
	else if(p->semty == sem_func_call){
			printf("func [%s]\n", p->tk.src.c_str());
	}
	else if(p->semty == sem_return){
		printf("%s \n", p->tk.src.c_str());
	}
	else{
		ERR();
	}

	if(p->left)
	{
		print_blank(depth);
		printf("L: \n");
		dump_ast_node(p->left, depth + 1);
	}

	if(p->right)
	{
		print_blank(depth);
		printf("R: \n");
		dump_ast_node(p->right, depth + 1);
	}
}

static void dump_scope(Scope *s, int depth)
{
	if(!s)
		return;

	if(!s->is_virtual_scope)
		print_blank(depth);
	else
		print_blank(depth - 1);

	printf("===%s===", s->name.c_str());
	if(s->is_virtual_scope)
		printf(" virtual");
	printf("\n");

	for(auto &it : *(s->var_table))
	{
		print_blank(depth);
		printf("symbol: %s\n", it.second->unique_name.c_str());
	}

	for(auto ast : s->asts)
	{
		print_blank(depth);
		printf("AST:\n");
		dump_ast_node(ast, depth + 1);
	}
	printf("\n");

	for(auto child : s->clds) {
		dump_scope(child, depth + 1);
	}
}
void dump_ast()
{
    printf("========== AST ==========\n");
    dump_scope(&file_scope,0);
    printf("=========================\n");
}



