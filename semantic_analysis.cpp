/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"

extern Scope file_scp;
map<string, SymbolVar*> global_unique_src_name_tbl;

static int case_sem_assign(Ast *p)
{
	assert(p);
	assert(p->left);
	assert(p->right);

	Ast *left = p->left;
	if(left->semty != SEM_VAR)
		ERR("semty %d", left->semty);


	SemanticType ty = p->right->semty;

	if(!(ty == SEM_VAR
	        || ty == SEM_CONST_NUM
	        || ty < OP_ALL
	        || ty == SEM_FUNC_CALL)) {

		ERR("semty %d, %s %s %s ", ty,
		    p->tk.src.c_str(),
		    left->tk.src.c_str(), p->right->tk.src.c_str());
	}

//	assert(p->left->var_type == p->right->var_type);

	return 0;
}
static int case_sem_op(Ast *p)
{
	assert(p);
	assert(p->left);
	SemanticType ty = p->left->semty;
	assert(ty == SEM_VAR
	        || ty == SEM_CONST_NUM
	        || ty < OP_ALL
	        || ty == SEM_FUNC_CALL);

	assert(p->right);
	ty = p->right->semty;
	assert(ty == SEM_VAR
	        || ty == SEM_CONST_NUM
	        || ty < OP_ALL
	        || ty == SEM_FUNC_CALL);

//	p->vr = vrid++;
//	p->vr_name = "%" + std::to_string(p->vr);
//	assert(p->left->var_type == p->right->var_type);
	return 0;
}
static int case_sem_return(Ast *p)
{
	assert(p);
	assert(p->left);
	assert(!p->right);

	SemanticType ty = p->left->semty;
	if(!(ty == SEM_VAR
	        || ty == SEM_CONST_NUM
	        || ty < OP_ALL
	        || ty == SEM_FUNC_CALL)) {
		ERR("semty %d, %s %s ", ty,
		    p->tk.src.c_str(),
		    p->left->tk.src.c_str());
	}

	Scope *scp = p->this_scp;
	while(scp && scp->is_virtual){
		scp = scp->parent;
	}

	if(!scp)
		ERR();
	p->sem_home_scp = scp;

	if(p->sem_home_scp->sem != SEM_FUNC_DEFINE)
		ERR("return not in a func");
	//		assert(p->this_scp->is_virtual_scope == false);
	return 0;
}
static int case_sem_var(Ast *p)
{
	SymbolVar *symb;
	Scope *scp = p->this_scp;

	while(scp && scp->var_table)
	{
		auto r = scp->var_table->find(p->tk.src);
		if(r != scp->var_table->end()) {
			p->symb_var = r->second;
			break;
		}
		scp = scp->parent;
	}
	if(!p->symb_var) {
		ERR("error: %s is undeclared\n", p->tk.src.c_str());
	}

	return 0;
}
static int case_sem_variable_declare(Ast *p)
{
	// sem_declare is root node, no child
	Scope *scp = p->this_scp;
	auto tbl = scp->var_table;
	assert(p->var_type == INT);
	assert(p->left == 0 && p->right == 0);

	if(tbl->find(p->tk.src) != tbl->end()) {
		ERR("%s is already declared\n", p->tk.src.c_str());
	}

	SymbolVar *symb = new SymbolVar;
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
static int trace_ast_up_down__named_variable_declare(Ast *p)
{
	if(!p)
		return 0;

	switch(p->semty)
	{
	case SEM_VAR_DECLARE:
		case_sem_variable_declare(p);
		break;

	case OP_ASSIGN:
		case_sem_assign(p);
		break;

	case SEM_VAR:
		case_sem_var(p);
		break;

	case SEM_CONST_NUM:
		break;

	case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
		case_sem_op(p);
		break;

	case SEM_RETURN:
		case_sem_return(p);
		break;
	default:
		ERR("%d \n", p->semty);
	}

	trace_ast_up_down__named_variable_declare(p->left);
	trace_ast_up_down__named_variable_declare(p->right);

	return 0;
}
static int sem_analysis_named_var(Scope *scp)
{
	LOG();
	LOG("%s \n", scp->name.c_str());

	for(int i = 0; i < scp->asts.size(); i++) {
		LOG("ast %d\n", i);
		trace_ast_up_down__named_variable_declare(scp->asts[i]);
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
		sem_analysis_named_var(p);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////

static int trace_ast_down_up_gen_vr(Ast *p)
{
	if(!p)
		return -1;

	int b = trace_ast_down_up_gen_vr(p->right);
	int a = trace_ast_down_up_gen_vr(p->left);
	string c;
	SymbolVar *symb = 0;
	ThreeAddrCode *inst = 0;

	switch(p->semty)
	{
	// leaf node
	case SEM_VAR:
		symb = p->symb_var;
		assert(symb);
		if(symb->vr < 0)
			symb->vr = vrm.new_vr(p);

		// todo symb->vr to be defined in "new =" to gen ssa
		return symb->vr;

	case SEM_CONST_NUM:
		p->vr_id = vrm.new_vr(p);
		return p->vr_id;

		// x = y : return x
	case OP_ASSIGN:
		if(p->left->var_type != p->right->var_type)
			ERR("assign type mismatch %d %d", p->left->var_type, p->right->var_type);

		p->var_type = p->left->var_type;
		p->vr_id = a;
		return a;

		// todo gen a assign inst
	case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
			if(p->left->var_type != p->right->var_type)
				ERR("op type mismatch %d %d", p->left->var_type, p->right->var_type);

			p->var_type = p->left->var_type;
			p->vr_id = vrm.new_vr(p);
			return p->vr_id;

	case SEM_RETURN:
		if(p->sem_home_scp->return_type != p->left->var_type)
			ERR("return type mismatch %d %d", p->sem_home_scp->return_type, p->left->var_type);

		return -1;

	case SEM_VAR_DECLARE:
		return -1;

	case SEM_FUNC_DECLARE:
	case SEM_FUNC_DEFINE:
	case SEM_FUNC_CALL:
		printf("todo sem_func* semty %d \n", p->semty);
		return -1;

	default:
		ERR("%d \n", p->semty);
		break;
	}

	return -1;
}
static void sem_analysis_gen_vr(Scope *scp)
{
	LOG("scp %s \n", scp->name.c_str());

	for(auto it = scp->asts.begin(); it != scp->asts.end();) {
		LOG("%s, scp %s, ast %ld\n", __FUNCTION__, scp->name.c_str(), it - scp->asts.begin());
		Ast *p = *it;

		trace_ast_down_up_gen_vr(p);
		it++;
	}

	for(Scope *p : scp->clds) {
		sem_analysis_gen_vr(p);
	}
	return;
}

void sem_analysis()
{
	sem_analysis_named_var(&file_scp);
//	dump_ast();

	sem_analysis_gen_vr(&file_scp);
//	dump_ast();
}


