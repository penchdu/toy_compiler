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
		return "int";
	case FLOAT:
			return "FLOAT";
	case VOID:
			return "VOID";
	case INVALID_TYPE:
	default:
		return "INVALID_TYPE";
	}
}

void print_blank(int n)
{
	n = std::max(n, 0);
	for(int i = 0; i < n; i++)
		printf("    ");
}
void dump_ast_node(Ast *p, int depth)
{
	if(p == nullptr)
		return;

	print_blank(depth);
//	printf("%s", p->src_name.c_str());

	if(p->semty == sem_declare)
		printf(" [%s %s]\n", get_var_type_name(p->var_type).c_str(), p->src_name.c_str());
	else if(p->semty < op_all){
		printf("%s [%s]\n", p->sem_name.c_str(), p->vr_name.c_str());
		print_blank(depth);
		printf("\n");
	}
	else if(p->semty == sem_var){
		if(!p->var_symb)
			printf("%s [ ]\n", p->src_name.c_str());
		else
			printf("%s [%s]\n", p->var_symb->unique_name.c_str(), p->var_symb->vr_name.c_str());
	}
	else if(p->semty == sem_const_num)
		printf("%d [num]\n", p->const_value);


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

void dump_scope(Scope *s, int depth)
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

static string trace_ast(Ast *p)
{
	if(!p)
		return "";

	string a = trace_ast(p->left);
	string b = trace_ast(p->right);
	string c;
	Symbol_var *symb = 0;

	switch(p->semty)
	{
	case sem_declare:
		ERR();
		break;

	case sem_var:
		symb = p->var_symb;
		assert(symb);
		symb->vr_name = "%" + std::to_string(symb->vr);
		return symb->vr_name;
		return symb->unique_name;

	case sem_const_num:
		return std::to_string(p->const_value);

	case op_assign:
		c = a + " " + p->src_name + " " + b;
		ir.push_back(c);
		return a;

	case op_add:
		case op_sub:
		case op_mul:
		case op_div:
			p->vr = vrid++;
			p->vr_name = "%" + std::to_string(p->vr);
			c = p->vr_name + " = " + a + " " + p->src_name + " " + b;
			ir.push_back(c);
			return p->vr_name;

	case sem_return:
		c = p->src_name + " " + a;
		ir.push_back(c);
		return "";

	default:
		ERR("%d \n", p->semty);
	}

	return 0;
}
void _dump_ir(Scope *scp)
{
	LOG("scp %s \n", scp->name.c_str());

	for(auto it = scp->asts.begin(); it != scp->asts.end();) {
		printf("%s, scp %s, ast %ld\n", __FUNCTION__, scp->name.c_str(), it - scp->asts.begin());
		Ast *p = *it;

		trace_ast(p);
		it++;
	}

	for(Scope *p : scp->clds) {
		_dump_ir(p);
	}
	return;
}
void dump_ir()
{
	_dump_ir(&file_scope);

	printf("\n========== ir ==========\n");
	for(auto &r : ir) {
		printf("%s\n", r.c_str());
	}
}


