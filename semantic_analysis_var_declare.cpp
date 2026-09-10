/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"


bool is_math_op(Semantic_type t)
{
	return t == op_add
	        || t == op_sub
	        || t == op_mul
	        || t == op_div;
}
int case_sem_var(Ast *p)
{
	Symbol_var *symb;
	Scope *ps = p->this_scp;

	while(ps && ps->var_table)
	{
		auto r = ps->var_table->find(p->tk.src);
		if(r != ps->var_table->end()) {
			p->symb_var = r->second;
			break;
		}
		ps = ps->parent;
	}
	if(!p->symb_var) {
		ERR("error: %s is undeclared\n", p->tk.src.c_str());
	}

	return 0;
}
int case_sem_assign(Ast *p)
{
	assert(p);
	assert(p->left);
	assert(p->right);

	Ast *left = p->left;
	if(left->semty != sem_var)
		ERR("semty %d", left->semty);


	Semantic_type ty = p->right->semty;

	if(!(ty == sem_var
	        || ty == sem_const_num
	        || ty == op_assign
	        || is_math_op(ty)
	        || ty == sem_func_call)) {

		ERR("semty %d, %s %s %s ", ty,
		    p->tk.src.c_str(),
		    left->tk.src.c_str(), p->right->tk.src.c_str());
	}

//	assert(p->left->var_type == p->right->var_type);

	return 0;
}
int case_sem_op(Ast *p)
{
	assert(p);
	assert(p->left);
	Semantic_type ty = p->left->semty;
	assert(ty == sem_var
	        || ty == sem_const_num
	        || is_math_op(ty)
	        || ty == sem_func_call);

	assert(p->right);
	ty = p->right->semty;
	assert(ty == sem_var
	        || ty == sem_const_num
	        || is_math_op(ty)
	        || ty == sem_func_call);

//	p->vr = vrid++;
//	p->vr_name = "%" + std::to_string(p->vr);
//	assert(p->left->var_type == p->right->var_type);
	return 0;
}
int case_sem_return(Ast *p)
{
	assert(p);
	assert(p->left);
	assert(!p->right);

	Semantic_type ty = p->left->semty;
	if(!(ty == sem_var
	        || ty == sem_const_num
	        || is_math_op(ty)
	        || ty == sem_func_call)) {
		ERR("semty %d, %s %s ", ty,
		    p->tk.src.c_str(),
		    p->left->tk.src.c_str());
	}

	Scope *scp = p->this_scp;
	while(scp && scp->is_virtual_scope){
		scp = scp->parent;
	}

	if(!scp)
		ERR();
	p->sem_home_scp = scp;

	if(p->sem_home_scp->sem != sem_func_declare)
		ERR("return not in a func");
	//		assert(p->this_scp->is_virtual_scope == false);
	return 0;
}

int case_sem_variable_declare(Ast *p)
{
	// sem_declare is root node, no child
	Scope *scp = p->this_scp;
	auto tbl = scp->var_table;
	assert(p->var_type == INT);

	if(tbl->find(p->tk.src) != tbl->end()) {
		ERR("%s is already declared\n", p->tk.src.c_str());
	}

	Symbol_var *symb = new Symbol_var;
	// p->semty has been set to sem_declare, but actually it is sem_var
//	symb->semty = sem_var;
	symb->var_type = p->var_type;
	symb->src = &p->tk.src;
//	symb->vr = get_vr(p);

	// get a unique name
	if(global_unique_src_name_tbl.find(p->tk.src) == global_unique_src_name_tbl.end()) {
		symb->unique_name = p->tk.src;
		global_unique_src_name_tbl[p->tk.src] = symb;
	}
	else {
		// a scope declared p->src_name before this point
		symb->unique_name = "b" + std::to_string(scp->id) + "_" + p->tk.src;
	}

	symb->explicit_unique_name = "b" + std::to_string(scp->id) + "_" + p->tk.src;
	p->symb_var = symb;
	tbl->insert({p->tk.src, symb});
	return 0;
}

static int trace_ast_up_down_do_var_declare(Ast *p)
{
	if(!p)
		return 0;

	switch(p->semty)
	{
	case sem_var_declare:
		case_sem_variable_declare(p);
		break;

	case op_assign:
		case_sem_assign(p);
		break;

	case sem_var:
		case_sem_var(p);
		break;

	case sem_const_num:
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

	trace_ast_up_down_do_var_declare(p->left);
	trace_ast_up_down_do_var_declare(p->right);

	return 0;
}
int sem_analysis_var_declare(Scope *scp)
{
	LOG();
	LOG("%s \n", scp->name.c_str());

	for(auto it : scp->asts) {
		LOG("ast %ld\n", it - scp->asts.begin());
		trace_ast_up_down_do_var_declare(it);
	}

//	for(auto it = scp->asts.begin(); it != scp->asts.end();) {
//		Ast *p = *it;
//
//		// sem_declare is root node, no child
//		if(p->semty == sem_declare) {
//			delete p;
//			it = scp->asts.erase(it);
//			continue;
//		}
//		it++;
//	}

	for(Scope *p : scp->clds) {
		sem_analysis_var_declare(p);
	}

	return 0;
}

