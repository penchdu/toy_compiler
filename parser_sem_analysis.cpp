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
			p->symb = symb;
			p->home_scp = scp;
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
				p->symb = symb;
				p->home_scp = scp;
				break;
			}
		}
	}
	if (!p->symb)
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
	int i = 0;
	for (; i < global_unique_src_name_tbl.size(); i++)
	{
		if (global_unique_src_name_tbl[i]->src == var->tk.src)
			break;
	}
	if (i == global_unique_src_name_tbl.size())
		symb->unique_name = var->tk.src;
	else
		symb->unique_name = "b" + to_string(scp->id) + "_" + var->tk.src;

	symb->explicit_unique_name = "b" + to_string(scp->id) + "_" + var->tk.src;
	var->home_scp = var->this_scp;
	var->symb_stamp = SYMB_PRIVATE;
	var->symb = symb;
	scp->symb_table->push_back(symb);
	global_unique_src_name_tbl.push_back(symb);
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
		p->symb->cld_use_cnt++;
	else
		p->symb->use_cnt++;
}
static void new_PRIVATE_transient_symb(Ast *p)
{
	SymbolVariable *symb = new SymbolVariable;
	symb->type = p->type;
	symb->vr = p->vr;
	symb->src = "%" + to_string(p->vr);
	symb->unique_name = symb->src;
	symb->stamp = SYMB_PRIVATE_transient;
//	symb->use_cnt = 1;

	p->symb_stamp = SYMB_PRIVATE_transient;
	p->symb = symb;
	p->this_scp->symb_table->push_back(symb);
	p->home_scp = p->this_scp;
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
		p->vr = p->left->vr;
		return p->vr;

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
		p->vr = vr_declare_manager.new_vr(p);
		new_PRIVATE_transient_symb(p);
		return p->vr;

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
		symb = p->symb;
		assert(symb);
		if (symb->vr < 0)
			symb->vr = vr_declare_manager.new_vr(p);

		// todo symb->vr to be defined in "new =" to gen ssa
		p->vr = symb->vr;
		return symb->vr;

	case SEM_CONST_NUM:
		p->vr = vr_declare_manager.new_vr(p);
		new_PRIVATE_transient_symb(p);
		return p->vr;

	case SEM_OPERATOR:
		return case_op(p);

	case SEM_SAVE_RET_VALUE:
		p->vr = a;
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

