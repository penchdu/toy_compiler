/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"

Ast* do_declare_and_sem_name_in_ast(Ast *p)
{

	return 0;
}
bool is_math_op(Sem_type t)
{
	return t == op_add
	        || t == op_sub
	        || t == op_mul
	        || t == op_div;
}
int case_sem_assign(Ast *p)
{
	assert(p);
	assert(p->left);

	Ast *l = p->left;
	if(l->semty != sem_var)
		ERR("semty %d", l->semty);

	assert(p->right);
	Sem_type ty = p->right->semty;

	assert(ty == sem_var
	        || ty == sem_const_num
	        || is_math_op(ty)
	        || ty == sem_func);

	if(!(ty == sem_var
	        || ty == sem_const_num
	        || is_math_op(ty)
	        || ty == sem_func)) {

		ERR("semty %d, %s %s %s ", ty,
		    p->src_name.c_str(),
		    l->src_name.c_str(), p->right->src_name.c_str());
	}

//	assert(p->left->var_type == p->right->var_type);

	return 0;
}
int case_sem_op(Ast *p)
{
	assert(p);
	assert(p->left);
	Sem_type ty = p->left->semty;
	assert(ty == sem_var
	        || ty == sem_const_num
	        || is_math_op(ty)
	        || ty == sem_func);

	assert(p->right);
	ty = p->right->semty;
	assert(ty == sem_var
	        || ty == sem_const_num
	        || is_math_op(ty)
	        || ty == sem_func);

//	p->vr = vrid++;
//	p->vr_name = "%" + std::to_string(p->vr);


//	assert(p->left->var_type == p->right->var_type);
	return 0;
}
int case_sem_var(Ast *p)
{
	Symbol_var *symb;
	Scope *ps = p->this_scp;

	while(ps && ps->var_table)
	{
		auto r = ps->var_table->find(p->src_name);
		if(r != ps->var_table->end()) {
			p->var_symb = r->second;
			break;
		}
		ps = ps->parent;
	}
	if(!p->var_symb) {
		ERR("error: %s is undeclared\n", p->src_name.c_str());
	}

	return 0;
}
int case_sem_return(Ast *p)
{
	assert(p);
	assert(p->left);
	assert(!p->right);

	Sem_type ty = p->left->semty;
	if(!(ty == sem_var
	        || ty == sem_const_num
	        || is_math_op(ty)
	        || ty == sem_func)) {
		ERR("semty %d, %s %s ", ty,
		    p->src_name.c_str(),
		    p->left->src_name.c_str());
	}
	return 0;
}
static int trace_ast(Ast *p)
{
	if(!p)
		return 0;

	switch(p->semty)
	{
	case sem_declare:
		ERR();
		break;

	case sem_var:
		case_sem_var(p);
		break;

	case sem_const_num:
		break;

	case op_assign:
		case_sem_assign(p);
		break;

	case op_add:
		case op_sub:
		case op_mul:
		case op_div:
		case_sem_op(p);
		break;

	case sem_return:
		case_sem_return(p);
		break;
	default:
		ERR("%d \n", p->semty);
	}

	trace_ast(p->left);
	trace_ast(p->right);

	return 0;
}
int case_sem_declare(Scope *scp, Ast *p)
{
	// sem_declare is root node
	auto tbl = scp->var_table;
	assert(p->var_type == INT);

	if(tbl->find(p->src_name) != tbl->end()) {
		ERR("%s is already declared\n", p->src_name.c_str());
	}

	Symbol_var *symb = new Symbol_var;
	// p->semty has been set to sem_declare, but actually it is sem_var
	symb->semty = sem_var;
	symb->var_type = p->var_type;
	symb->src_name = p->src_name;
	symb->vr = vrid++;

	// get a unique name
	if(global_unique_src_name_table.find(p->src_name) == global_unique_src_name_table.end()) {
		symb->unique_name = p->src_name;
		global_unique_src_name_table[p->src_name] = symb;
	}
	else {
		// a scope declared p->src_name before this point
		symb->unique_name = "b" + std::to_string(scp->id) + "_" + p->src_name;
	}

	symb->explicit_unique_name = "b" + std::to_string(scp->id) + "_" + p->src_name;
	tbl->insert({p->src_name, symb});

	return 0;
}
int semantic_analysis(Scope *scp)
{
	LOG();
	printf("%s \n", scp->name.c_str());

	for(auto it = scp->asts.begin(); it != scp->asts.end();) {
		printf("ast %ld\n", it - scp->asts.begin());
		Ast *p = *it;

		// sem_declare is root node
		if(p->semty == sem_declare) {
			case_sem_declare(scp, p);
			delete p;
			it = scp->asts.erase(it);
			continue;
		}

		trace_ast(p);
		it++;
	}

	for(Scope *p : scp->clds) {
		semantic_analysis(p);
	}

	return 0;
}

