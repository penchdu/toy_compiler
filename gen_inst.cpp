/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"

int get_vr(Ast *p)
{
	static int id = -1;
	id++;
	return id;

//	global_unique_vrid_tbl.push_back(p);
//	return global_unique_vrid_tbl.size() - 1;
}
//int get_const_vr(int v)
//{
//	if (const_num_vr_tbl.find(v) == const_num_vr_tbl.end()){
//
//	}
//
//}

static int trace_ast_down_up_gen_inst(Ast *p)
{
	if(!p)
		return -1;

	int b = trace_ast_down_up_gen_inst(p->right);
	int a = trace_ast_down_up_gen_inst(p->left);
	string c;
	Symbol_var *symb = 0;
	Instruction *inst = 0;

	switch(p->semty)
	{
	// leaf node
	case sem_var:
		symb = p->symb_var;
		assert(symb);
		if(symb->vr < 0)
			symb->vr = get_vr(p);

		// todo symb->vr to be defined in "new =" to gen ssa
		return symb->vr;

	case sem_const_num:
		p->vr = get_vr(p);
		inst = new Instruction(p);
		inst->dst = p->vr;
		insts.push_back(inst);
		return p->vr;

		// x = y : return x
	case op_assign:
		inst = new Instruction(p);
		inst->dst = a;
		inst->src1 = b;
		insts.push_back(inst);

		p->vr = a;
		p->var_type = p->left->var_type;
		assert(p->left->var_type == p->right->var_type);
		return a;

		// todo gen a assign inst
	case op_add:
		case op_sub:
		case op_mul:
		case op_div:
			p->vr = get_vr(p);
			inst = new Instruction(p);
			inst->dst = p->vr;
			inst->src1 = a;
			inst->src2 = b;
			insts.push_back(inst);

			p->var_type = p->left->var_type;
			assert(p->left->var_type == p->right->var_type);
			return p->vr;

	case sem_return:
		inst = new Instruction(p);
		inst->src1 = a;
		insts.push_back(inst);

		assert(p->sem_home_scp->return_type == p->left->var_type);
		return -1;

	case sem_var_declare:
		return -1;

	case sem_func_call:		// todo
	default:
		ERR("%d \n", p->semty);
		break;
	}

	return -1;
}
void _gen_inst(Scope *scp)
{
	LOG("scp %s \n", scp->name.c_str());

	for(auto it = scp->asts.begin(); it != scp->asts.end();) {
		LOG("%s, scp %s, ast %ld\n", __FUNCTION__, scp->name.c_str(), it - scp->asts.begin());
		Ast *p = *it;

		trace_ast_down_up_gen_inst(p);
		it++;
	}

	for(Scope *p : scp->clds) {
		_gen_inst(p);
	}
	return;
}
void dump_inst()
{
	printf("\n========== inst ==========\n");
	for(auto &r : insts) {
//		Ast *p = global_unique_vrid_tbl[r->dst];
		Ast *p = r->ast;

		switch(p->semty)
		{
		case op_assign:
			printf("assign[%%%d]:\t %%%d %s %%%d\n", r->dst, r->dst, p->tk.src.c_str(), r->src1);
			break;

		case op_add:
		case op_sub:
		case op_mul:
		case op_div:
			printf("op:\t\t %%%d = %%%d %s %%%d\n", r->dst, r->src1, p->tk.src.c_str(), r->src2);
			break;

		case sem_var_declare:
			printf("del:\t\t %s[%s] %%%d\n", p->tk.src.c_str(), p->symb_var->unique_name.c_str(),p->symb_var->vr);
			break;

		case sem_var:
			printf("var:\t\t %%%d %s\n", r->dst, p->tk.src.c_str());
			break;

		case sem_const_num:
			printf("const:\t\t %%%d num %d\n", r->dst, p->const_value);
			break;

		case sem_func_call:
			printf("func call:\t\t %s\n", p->tk.src.c_str());
			break;

		case sem_return:
			printf("return:\t\t %s %%%d\n", p->tk.src.c_str(), r->src1);
			break;

		default:
			ERR();
			break;
		}
	}
}
void gen_inst()
{
	_gen_inst(&file_scope);
	dump_ast();
	dump_inst();
}
