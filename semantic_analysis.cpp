/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "basic_block.h"
#include "frontend.h"

extern Scope file_scp;
map<string, SymbolVar*> global_unique_src_name_tbl;

static int case_sem_assign(Ast *p)
{
	assert(p);
	assert(p->left);
	assert(p->right);

	Ast *left = p->left;
	if (left->sem != SEM_VAR)
		ERR("semty %d", left->sem);

	Semantic ty = p->right->sem;

	if (!(ty == SEM_VAR
	        || ty == SEM_CONST_NUM
	        || (ty == SEM_OPERATOR)
	        || ty == SEM_FUNC_CALL))
	{

		ERR("semty %d, %s %s %s ", ty,
		        p->tk.src.c_str(),
		        left->tk.src.c_str(), p->right->tk.src.c_str());
	}

//	assert(p->left->var_type == p->right->var_type);

	return 0;
}
static int case_sem_op(Ast *p)
{
	if (p->op == OP_ASSIGN)
		return case_sem_assign(p);

	assert(p);
	assert(p->left);
	Semantic ty = p->left->sem;
	assert(ty == SEM_VAR
	        || ty == SEM_CONST_NUM
	        || ty == SEM_OPERATOR
	        || ty == SEM_FUNC_CALL);

	assert(p->right);
	ty = p->right->sem;
	assert(ty == SEM_VAR
	        || ty == SEM_CONST_NUM
	        || ty == SEM_OPERATOR
	        || ty == SEM_FUNC_CALL);

//	p->vr = vrid++;
//	p->vr_name = "%" + std::to_string(p->vr);
//	assert(p->left->var_type == p->right->var_type);
	return 0;
}
static int case_sem_return(Ast *p)
{
	assert(p);
	assert(!p->right);

	if(!p->left)
		ERR("only support return int");

	Semantic ty = p->left->sem;
	if (!(ty == SEM_VAR
	        || ty == SEM_CONST_NUM
	        || ty == SEM_OPERATOR
	        || ty == SEM_FUNC_CALL))
	{
		ERR("semty %d, %s %s ", ty,
		        p->tk.src.c_str(),
		        p->left->tk.src.c_str());
	}

	return 0;
}
static int case_sem_var(Ast *p)
{
	Scope *scp = p->this_scp;

	while (scp && scp->var_table)
	{
		auto r = scp->var_table->find(p->tk.src);
		if (r != scp->var_table->end())
		{
			p->symb_var = r->second;
			break;
		}
		scp = scp->parent;
	}
	if (!p->symb_var)
	{
		ERR("error: %s is undeclared", p->tk.src.c_str());
	}

	return 0;
}
//static int find_vr_for_declare(Ast *p)
//{
//	return case_sem_var(p);
//}
static int case_sem_variable_declare(Ast *ty)
{
	assert(ty->type == INT);
	assert(ty->left);
	assert(ty->right == nullptr);
	assert(ty->parent == nullptr);

	Scope *scp = ty->this_scp;
	auto tbl = scp->var_table;

	Ast *var = ty->left;
	var->type = ty->type;

	if (tbl->find(var->tk.src) != tbl->end())
	{
		ERR("%s is already declared", var->tk.src.c_str());
	}

	SymbolVar *symb = new SymbolVar;
//	symb->semty = sem_var;
	symb->type = var->type;
	symb->src = &var->tk.src;
//	symb->vr = get_vr(var);

// get a unique name
	if (global_unique_src_name_tbl.find(var->tk.src) == global_unique_src_name_tbl.end())
	{
		symb->unique_name = var->tk.src;
		global_unique_src_name_tbl[var->tk.src] = symb;
	}
	else
	{
		// a scope declared var->src_name before this point
		symb->unique_name = "b" + std::to_string(scp->id) + "_" + var->tk.src;
	}

	symb->explicit_unique_name = "b" + std::to_string(scp->id) + "_" + var->tk.src;
	var->symb_var = symb;
	tbl->insert( {var->tk.src, symb});
	return 0;
}
static int trace_ast_up_down__named_variable_declare(Ast *p)
{
	if (!p)
		return 0;

	switch (p->sem)
	{
	case SEM_VAR_DECLARE:
		case_sem_variable_declare(p);
		break;

	case SEM_VAR:
		case_sem_var(p);
		break;

	case SEM_CONST_NUM:
		break;

	case SEM_OPERATOR:
		case_sem_op(p);
		break;

	case SEM_SAVE_RET_VALUE_AND_JMP:
		case_sem_return(p);
		break;

	default:
		ERR("%s, %d", p->tk.src.c_str(), p->sem);
	}

	trace_ast_up_down__named_variable_declare(p->left);
	trace_ast_up_down__named_variable_declare(p->right);

	return 0;
}
static int sem_analysis_named_var(Scope *scp)
{
	LOG();
	LOG("%s", scp->name.c_str());

	for (int i = 0; i < scp->asts.size(); i++)
	{
		LOG("ast %d", i);
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

	for (Scope *p : scp->clds)
	{
		sem_analysis_named_var(p);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////

static int case_op(Ast *p)
{
	switch (p->op)
	{
// x = y : return x
	case OP_ASSIGN:
		if (p->left->type != p->right->type)
			ERR("assign type mismatch %d %d", p->left->type, p->right->type);

		p->type = p->left->type;
		// assign do not gen a new vr, just return left
		p->vr_id = p->left->vr_id;
		return p->vr_id;

// todo gen a assign inst
	case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:

			// todo, type bool
	case OP_CMP_L:
		case OP_CMP_LE:
		case OP_CMP_E:
		case OP_CMP_GE:
		case OP_CMP_G:
		case OP_CMP_NE:

	case OP_LOGIC_AND:

		if (p->left->type != p->right->type)
			ERR("op type mismatch %d %d", p->left->type, p->right->type);

		p->type = p->left->type;
		p->vr_id = vrm.new_vr(p);
		return p->vr_id;

	default:
		ERR();
	}
}
static int trace_ast_down_up_gen_vr(Ast *p)
{
	if (!p || p->sem == SEM_VAR_DECLARE)
		return -1;

//	if(SEM_SAVE_RET_VALUE_AND_JMP)

	int b = trace_ast_down_up_gen_vr(p->right);
	int a = trace_ast_down_up_gen_vr(p->left);
	SymbolVar *symb = 0;
	(void)a;
	(void)b;

	switch (p->sem)
	{
	// leaf node
	case SEM_VAR:
		symb = p->symb_var;
		assert(symb);
		if (symb->vr < 0)
			symb->vr = vrm.new_vr(p);

		// todo symb->vr to be defined in "new =" to gen ssa
		p->vr_id = symb->vr;
		return symb->vr;

	case SEM_CONST_NUM:
		p->vr_id = vrm.new_vr(p);
		return p->vr_id;

	case SEM_OPERATOR:
		return case_op(p);

	case SEM_SAVE_RET_VALUE_AND_JMP:
		if (p->sem_home_scp->return_type != p->left->type)
			ERR("return type mismatch %d %d", p->sem_home_scp->return_type, p->left->type);
		assert(p->op == OP_SAVE_RET_VALUE);

		return -1;

	case SEM_VAR_DECLARE:
		return -1;

	case SEM_FUNC_DECLARE:
		case SEM_FUNC_DEFINE:
		case SEM_FUNC_CALL:
		printf("todo sem_func* semty %d \n", p->sem);
		return -1;

	default:
		ERR("%d", p->sem);
		break;
	}

	return -1;
}
static void sem_analysis_gen_vr(Scope *scp)
{
	LOG("scp %s", scp->name.c_str());

	for (auto it = scp->asts.begin(); it != scp->asts.end();)
	{
		LOG("%s, scp %s, ast %ld", __FUNCTION__, scp->name.c_str(), it - scp->asts.begin());
		Ast *p = *it;

		trace_ast_down_up_gen_vr(p);
		it++;
	}

	for (Scope *p : scp->clds)
		sem_analysis_gen_vr(p);

	return;
}

void sem_analysis()
{
	sem_analysis_named_var(&file_scp);
//	dump_ast();

	sem_analysis_gen_vr(&file_scp);
//	dump_ast();
}

