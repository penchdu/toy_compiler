/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "basic_block.h"
#include "frontend.h"

extern Scope file_scp;
vector<SymbolVariable*> global_unique_src_name_tbl;

static int case_sem_assign(Ast *p)
{
	assert(p);
	assert(p->left);
	assert(p->right);

	Ast *left = p->left;
	if (left->sem_stamp != SEM_VAR)
		ERR("semty %d", left->sem_stamp);

	Semantic ty = p->right->sem_stamp;

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
	Semantic ty = p->left->sem_stamp;
	assert(ty == SEM_VAR
	    || ty == SEM_CONST_NUM
	    || ty == SEM_OPERATOR
	    || ty == SEM_FUNC_CALL);

	assert(p->right);
	ty = p->right->sem_stamp;
	assert(ty == SEM_VAR
	    || ty == SEM_CONST_NUM
	    || ty == SEM_OPERATOR
	    || ty == SEM_FUNC_CALL);

//	p->vr = vrid++;
//	p->vr_name = "%" + to_string(p->vr);
//	assert(p->left->var_type == p->right->var_type);
	return 0;
}
static int case_sem_save_retuen_up_down(Ast *p)
{
	assert(p);
	assert(p->left);
	assert(!p->right);

	if (!p->left)
		ERR("only support return int");

	Semantic ty = p->left->sem_stamp;
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

	for (auto symb : *(scp->symb_table))
	{
		if (symb->src == p->tk.src)
		{
			p->symb_stamp = SYMB_PRIVATE;
			p->var_symb = symb;
			return 0;
		}
	}

	while ((scp = scp->parent) && scp->symb_table)
	{
		for (auto symb : *(scp->symb_table))
		{
			if (symb->src == p->tk.src)
			{
				p->symb_stamp = SYMB_OUTER;
				p->var_symb = symb;
				break;
			}
		}
	}
	if (!p->var_symb)
		ERR("error: %s is undeclared", p->tk.src.c_str());

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

	Ast *var = ty->left;
	var->type = ty->type;

	for (auto s : *(scp->symb_table))
	{
		if (s->src == var->tk.src)
			ERR("%s is already declared", var->tk.src.c_str());
	}

	SymbolVariable *symb = new SymbolVariable;
	symb->type = var->type;
	symb->src = var->tk.src;

// get a unique name
	bool is_unique_name = 1;
	for (auto p : global_unique_src_name_tbl)
	{
		if (p->src == var->tk.src)
		{
			is_unique_name = 0;
			break;
		}
	}
	if (is_unique_name)
	{
		symb->unique_name = var->tk.src;
		global_unique_src_name_tbl.push_back(symb);
	}
	else
	{
		// a scope declared var->src_name before this point
		symb->unique_name = "b" + to_string(scp->id) + "_" + var->tk.src;
	}

	symb->explicit_unique_name = "b" + to_string(scp->id) + "_" + var->tk.src;
	var->symb_stamp = SYMB_PRIVATE;
	var->var_symb = symb;
	scp->symb_table->push_back(symb);
	return 0;
}
static int trace_ast_up_down__named_variable_declare(Ast *p)
{
	if (!p)
		return 0;

	switch (p->sem_stamp)
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

	case SEM_SAVE_RET_VALUE:
		case_sem_save_retuen_up_down(p);
		break;

	default:
		ERR("%s, %d", p->tk.src.c_str(), p->sem_stamp);
	}

	trace_ast_up_down__named_variable_declare(p->left);
	trace_ast_up_down__named_variable_declare(p->right);
	return 0;
}
static void sem_analysis_named_var(Scope *scp)
{
	LOG("%s", scp->name.c_str());
	for (Ast *p : scp->asts)
		trace_ast_up_down__named_variable_declare(p);

	for (Scope *p : scp->clds)
		sem_analysis_named_var(p);
}

///////////////////////////////////////////////////////////////////////////////////

static void increase_use_cnt(Ast *p)
{
	if (p->symb_stamp == SYMB_OUTER)
		p->var_symb->cld_use_cnt++;
	else
		p->var_symb->use_cnt++;
}
static void new_PRIVATE_transient_symb(Ast *p)
{
	SymbolVariable *symb = new SymbolVariable;
	symb->type = p->type;
	symb->src = to_string(p->vr_id);
	symb->vr = p->vr_id;
//	symb->use_cnt = 1;

	p->var_symb = symb;
	p->this_scp->symb_table->push_back(symb);
}

static int case_op(Ast *p)
{
	switch (p->op)
	{
// x = y : return x
	case OP_ASSIGN:
		if (p->left->type != p->right->type)
			ERR("assign type mismatch %d %d", p->left->type, p->right->type);

		p->type = p->left->type;
		assert(p->left->symb_stamp != SYMB_PRIVATE_transient);
		// assign do not gen a new vr, just return left
		p->vr_id = p->left->vr_id;
		return p->vr_id;

// todo gen a assign inst
	case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
		case OP_CMP_L:
		case OP_CMP_LE:
		case OP_CMP_E:
		case OP_CMP_GE:
		case OP_CMP_G:
		case OP_CMP_NE:

		if (p->left->type != p->right->type)
			ERR("op type mismatch %d %d", p->left->type, p->right->type);

		p->type = p->left->type;
		p->vr_id = vr_manager.new_vr(p);
		p->symb_stamp = SYMB_PRIVATE_transient;
		p->use_cnt++;
		new_PRIVATE_transient_symb(p);
		return p->vr_id;

	case OP_LOGIC_AND:
		default:
		ERR();
	}
}
static void case_save_return_down_up(Ast *p)
{
	LOG("%s", p->tk.src.c_str());

	Scope *func = p->this_scp;
	while (func && func->sem_stamp != SEM_FUNC_DEFINE)
		func = func->parent;

	if (!func)
		ERR();

	if (p->left->type != func->return_type)
		ERR("return type mismatch %d %d", func->return_type, p->left->type);

	assert(p->right == 0);
}
static int trace_ast_down_up_gen_vr(Ast *p)
{
	if (!p || p->sem_stamp == SEM_VAR_DECLARE)
		return -1;

	int b = trace_ast_down_up_gen_vr(p->right);
	int a = trace_ast_down_up_gen_vr(p->left);
	SymbolVariable *symb = 0;
	(void) a;
	(void) b;

	switch (p->sem_stamp)
	{
	// leaf node
	case SEM_VAR:
		symb = p->var_symb;
		assert(symb);
		if (symb->vr < 0)
			symb->vr = vr_manager.new_vr(p);

		// todo symb->vr to be defined in "new =" to gen ssa
		p->vr_id = symb->vr;
		return symb->vr;

	case SEM_CONST_NUM:
		p->vr_id = vr_manager.new_vr(p);
		p->symb_stamp = SYMB_PRIVATE_transient;
		p->use_cnt = 1;
		new_PRIVATE_transient_symb(p);
		return p->vr_id;

	case SEM_OPERATOR:
		return case_op(p);

	case SEM_SAVE_RET_VALUE:
		p->vr_id = a;
//		if(p->symb_live_region == SYMB_PRIVATE_transient)
//			p->symb_live_region = SYMB_PRIVATE;
		case_save_return_down_up(p);
		return -1;

	case SEM_VAR_DECLARE:
		return -1;

	case SEM_FUNC_DECLARE:
		case SEM_FUNC_DEFINE:
		case SEM_FUNC_CALL:
		printf("todo sem_func* semty %d \n", p->sem_stamp);
		return -1;

	default:
		ERR("%d", p->sem_stamp);
		break;
	}

	return -1;
}
static void sem_analysis_gen_vr(Scope *scp)
{
	LOG("scp %s", scp->name.c_str());

	for (Ast *p : scp->asts)
		trace_ast_down_up_gen_vr(p);

	for (Scope *p : scp->clds)
		sem_analysis_gen_vr(p);
}

void sem_analysis()
{
	sem_analysis_named_var(&file_scp);
//	dump_ast();

	sem_analysis_gen_vr(&file_scp);
//	dump_ast();
}

