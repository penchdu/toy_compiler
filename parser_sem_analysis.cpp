/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "basic_block.h"
#include "frontend.h"

extern Scope file_scp;
vector<Symbol*> global_unique_src_name_tbl;

static int case_sem_assign(Ast *p)
{
	assert(p);
	assert(p->left);
	assert(p->right);

	Ast *left = p->left;
	if (left->sem_stamp != SEM_VAR)
		ERR("%d is not lvalue", left->sem_stamp);

	Semantic ty = p->right->sem_stamp;

	if (!(ty == SEM_VAR
	    || ty == SEM_IMMEDIATE
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
static int case_sem_save_retuen_up_down(Ast *p)
{
	assert(p);
	assert(p->left);
	assert(!p->right);

	if (!p->left)
		ERR("only support return int");

	Semantic ty = p->left->sem_stamp;
	if (!(ty == SEM_VAR
	    || ty == SEM_IMMEDIATE
	    || ty == SEM_OPERATOR
	    || ty == SEM_FUNC_CALL))
	{
		ERR("semty %d, %s %s ", ty,
		    p->tk.src.c_str(),
		    p->left->tk.src.c_str());
	}

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
	    || ty == SEM_IMMEDIATE
	    || ty == SEM_OPERATOR
	    || ty == SEM_FUNC_CALL);

	if (p->op == OP_TEST)
	{
		assert(!p->right);
		return 0;
	}
	if (p->op == OP_SAVE_RET_VALUE)
	{
		case_sem_save_retuen_up_down(p);
		return 0;
	}
	assert(p->right);
	ty = p->right->sem_stamp;
	assert(ty == SEM_VAR
	    || ty == SEM_IMMEDIATE
	    || ty == SEM_OPERATOR
	    || ty == SEM_FUNC_CALL);

//	p->vr = vrid++;
//	p->vr_name = "%" + to_string(p->vr);
//	assert(p->left->var_type == p->right->var_type);
	return 0;
}

static int case_sem_var(Ast *p)
{
//	assert(p->symb == 0);
	Scope *scp = p->scope;

	for (auto symb : *(scp->symb_table_ptr))
	{
		if (symb->src == p->tk.src)
		{
			p->symb = symb;
			p->symb_stamp = SYMB_PRIVATE;
			return 0;
		}
	}

	Scope *parent = scp->parent;
	while (parent && parent->symb_table_ptr)
	{
		for (auto symb : *(parent->symb_table_ptr))
		{
			if (symb->src == p->tk.src)
			{
				p->symb = symb;
				p->symb_stamp = SYMB_OUTER;
				return 0;
			}
		}
		parent = parent->parent;
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

	Scope *scp = ty->scope;

	Ast *var = ty->left;
	var->type = ty->type;

	for (auto symb : *(scp->symb_table_ptr))
	{
		if (symb->src == var->tk.src)
			ERR("%s is already declared", var->tk.src.c_str());
	}

	Symbol *symb = new Symbol;
	symb->stamp = SYMB_PRIVATE;
	symb->type = var->type;
	symb->src = var->tk.src;
	symb->depth = var->scope->depth;
	symb->ast = var;

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

	var->symb = symb;
	if(var->sem_stamp == SEM_FUNC_ARG)
	{
		symb->ssa.push_back(declare_record.new_vr(var));
		var->vr = symb->ssa.back();

		Scope *func = var->scope;
		while (func && func->sem_stamp != SEM_FUNC_DEFINE)
			func = func->parent;

		if (!func)
			ERR();
		func->arg.push_back(var->vr);
	}
	scp->symb_table_ptr->push_back(symb);
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
		return 0;

	case SEM_FUNC_ARG:
	case SEM_VAR:
		case_sem_var(p);
		break;

	case SEM_IMMEDIATE:
		break;

	case SEM_OPERATOR:
		case_sem_op(p);
		break;

//	case SEM_SAVE_RET_VALUE:
//		case_sem_save_retuen_up_down(p);
//		break;

	default:
		ERR("%s, %d", p->tk.src.c_str(), p->sem_stamp);
	}

	trace_ast_up_down__named_variable_declare(p->left);
	trace_ast_up_down__named_variable_declare(p->right);
	return 0;
}
static void sem_analysis_named_var(Scope *scp)
{
	assert(scp->bb == 0);
	scp->bb = new BasicBlock;

	for (Ast *p : scp->asts)
		trace_ast_up_down__named_variable_declare(p);

	for (Scope *cld : scp->clds)
		sem_analysis_named_var(cld);
}

///////////////////////////////////////////////////////////////////////////////////

//static void increase_use_cnt(Ast *p)
//{
//	if (p->symb_stamp == SYMB_OUTER)
//		p->symb->cld_use_cnt++;
//	else
//		p->symb->use_cnt++;
//}
static Symbol* new_private_transient_vr(Ast *p)
{
	Symbol *symb = new Symbol;
	symb->type = p->type;

	if(p->sem_stamp == SEM_IMMEDIATE)
	{
		p->symb_stamp = symb->stamp = SYMB_IMMEDIATE;
		symb->src = p->tk.src;
	}
	else
	{
		p->symb_stamp = symb->stamp = SYMB_PRIVATE_transient;
		symb->src = p->tk.src; // + "%" + to_string(p->vr);
	}

	symb->unique_name = symb->src;
	symb->depth = p->scope->depth;
//	symb->use_cnt = 1;

	p->symb = symb;
	symb->ssa.push_back(declare_record.new_vr(p));
	p->vr = symb->ssa.back();
	p->scope->symb_table_ptr->push_back(symb);

	return symb;
}

static int case_arg(Ast *p)
{
//	Scope *func = p->scope;
//	while (func && func->sem_stamp != SEM_FUNC_DEFINE)
//		func = func->parent;
//
//	if (!func)
//		ERR();

	Symbol *symb = p->symb;
	assert(symb);

	assert(p->symb->ssa.size() > 0);
//	{
//		int vr = declare_record.new_vr(p);
//		p->symb->ssa.push_back(vr);
//	}
	p->vr = p->symb->ssa.back();
	return p->vr;
}

static int new_ssa(Ast *p)
{
	Scope *scp = p->scope;

	if (p->symb_stamp == SYMB_PRIVATE)
	{
		p->symb->ssa.push_back(declare_record.new_vr(p));
		p->vr = p->symb->ssa.back();
		return p->vr;
	}

	assert(p->symb_stamp == SYMB_OUTER);
	for (auto symb : *(scp->outer_symb_table_ptr))
	{
		if (symb->src == p->tk.src)
		{
			p->symb = symb;
			p->symb_stamp = SYMB_PRIVATE_ssa_of_outer_symb;
			symb->ssa.push_back(declare_record.new_vr(p));
			p->vr = symb->ssa.back();
			return p->vr;
		}
	}

	Scope *parent = scp;
	while ((parent = parent->parent) != nullptr)
	{
		for (auto symb : *(parent->symb_table_ptr))
		{
			if (symb->src == p->tk.src)
			{
				if (symb->ssa.size() == 0)
					symb->ssa.push_back(declare_record.new_vr(symb->ast));

				Symbol *outer_symb = new Symbol;
				p->symb = outer_symb;

				outer_symb->stamp = SYMB_PRIVATE_ssa_of_outer_symb;
				outer_symb->outer_symb = symb;
				outer_symb->out_base_vr = symb->ssa.back();
				outer_symb->type = symb->type;
				outer_symb->src = symb->src;
				outer_symb->unique_name = symb->unique_name;
				outer_symb->explicit_unique_name = symb->explicit_unique_name;
				outer_symb->depth = scp->depth;
				outer_symb->ssa.push_back(declare_record.new_vr(p));
				outer_symb->new_ssa_scp.push_back(p->scope);

				p->vr = outer_symb->ssa.back();
				scp->outer_symb_table_ptr->push_back(outer_symb);
				return p->vr;
			}
		}

		for (auto symb : *(parent->outer_symb_table_ptr))
		{
			if (symb->src == p->tk.src)
			{
				if (symb->ssa.size() == 0)
					symb->ssa.push_back(declare_record.new_vr(symb->ast));

				Symbol *outer_symb = new Symbol;
				p->symb = outer_symb;

				outer_symb->stamp = SYMB_PRIVATE_ssa_of_outer_symb;
				outer_symb->outer_symb = symb;
				outer_symb->out_base_vr = symb->ssa.back();
				outer_symb->type = symb->type;
				outer_symb->src = symb->src;
				outer_symb->unique_name = symb->unique_name;
				outer_symb->explicit_unique_name = symb->explicit_unique_name;
				outer_symb->depth = scp->depth;
				outer_symb->ssa.push_back(declare_record.new_vr(p));

				p->vr = outer_symb->ssa.back();
				scp->outer_symb_table_ptr->push_back(outer_symb);
				return p->vr;
			}
		}
	}

	if (!p->symb)
		ERR("error: %s is undeclared", p->tk.src.c_str());

	ERR();
	return -1;
}
static int get_var_ssa_vr(Ast *p)
{
	if (p->symb_stamp == SYMB_PRIVATE)
		return p->vr = p->symb->ssa.back();

	Scope *scp = p->scope;

	for (auto symb : *(scp->outer_symb_table_ptr))
	{
		if (symb->src == p->tk.src)
		{
			p->symb = symb;
			p->symb_stamp = SYMB_PRIVATE_ssa_of_outer_symb;
			p->vr = symb->ssa.back();
			return p->vr;
		}
	}

	assert(p->symb_stamp == SYMB_OUTER);

	Scope *parent = scp;
	while ((parent = parent->parent) != nullptr)
	{
		for (auto symb : *(parent->symb_table_ptr))
		{
			if (symb->src == p->tk.src)
			{
				p->symb = symb;
				p->symb_stamp = SYMB_OUTER;
				p->vr = symb->ssa.back();
				return p->vr;
			}
		}

		for (auto symb : *(parent->outer_symb_table_ptr))
		{
			if (symb->src == p->tk.src)
			{
				p->symb = symb;
				p->symb_stamp = SYMB_OUTER;
				p->vr = symb->ssa.back();
				return p->vr;
			}
		}
	}

	ERR();
	return -1;
}

static void case_save_return_down_up(Ast *p)
{
	LOG("%s", p->tk.src.c_str());

	Scope *func = p->scope;
	while (func && func->sem_stamp != SEM_FUNC_DEFINE)
		func = func->parent;

	if (!func)
		ERR();

	if (p->left->type != func->return_type)
		ERR("return type mismatch %d %d", func->return_type, p->left->type);

	assert(p->right == 0);
}
static int case_op(Ast *p)
{
	switch (p->op)
	{
// x = y : return x
	case OP_ASSIGN:
		if (p->left->type != p->right->type)
			ERR("assign type mismatch %d %d", p->left->type, p->right->type);
		assert(p->left->symb->stamp != SYMB_PRIVATE_transient);

//		new_ssa(p->left);
		// assign do not gen a new vr, just return left
		p->type = p->left->type;
		p->vr = p->left->vr;
		return p->vr;

	case OP_SAVE_RET_VALUE:
		p->vr = p->left->vr;
//		if(p->symb_live_region == SYMB_PRIVATE_transient)
//			p->symb_live_region = SYMB_PRIVATE;
		case_save_return_down_up(p);
		return -1;

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
		p->symb = new_private_transient_vr(p);
		return p->vr;

	case OP_TEST:
		return -1;

	case OP_LOGIC_AND:
		default:
		ERR();
	}
}

static int trace_ast_down_up_gen_vr(Ast *p, bool new_ssa_vr = false)
{
	if (!p || p->sem_stamp == SEM_VAR_DECLARE)
		return -1;

	int b = trace_ast_down_up_gen_vr(p->right);
	int a = trace_ast_down_up_gen_vr(p->left, p->op == OP_ASSIGN);
	(void) a;
	(void) b;

	switch (p->sem_stamp)
	{
	case SEM_FUNC_ARG:
		return case_arg(p);

	// leaf node
	case SEM_VAR:
		assert(p->symb);
		// is vr did not be initialized
		if (p->symb->ssa.size() == 0 || new_ssa_vr)
		{
//			p->symb->ssa.push_back(declare_record.new_vr(p));
			new_ssa(p);
		}

		p->vr = get_var_ssa_vr(p);
		return p->vr;

	case SEM_IMMEDIATE:
		assert(p->vr == -1);
		p->symb = new_private_transient_vr(p);
		return p->vr;

	case SEM_OPERATOR:
		return case_op(p);

//	case SEM_SAVE_RET_VALUE:
//		p->vr = a;
////		if(p->symb_live_region == SYMB_PRIVATE_transient)
////			p->symb_live_region = SYMB_PRIVATE;
//		case_save_return_down_up(p);
//		return -1;

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
//	LOG("scp %s", scp->name.c_str());

	for (Ast *p : scp->asts)
		trace_ast_down_up_gen_vr(p);

	for (Scope *cld : scp->clds)
		sem_analysis_gen_vr(cld);
}

void sem_analysis()
{
	sem_analysis_named_var(&file_scp);
//	dump_ast();

	sem_analysis_gen_vr(&file_scp);
	dump_ast();
}
