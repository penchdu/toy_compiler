/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"


//int get_const_vr(int v)
//{
//	if (const_num_vr_tbl.find(v) == const_num_vr_tbl.end()){
//
//	}
//
//}

static int trace_ast_down_up_gen_3_address_code(Ast *p)
{
	if(!p)
		return -1;

	int b = trace_ast_down_up_gen_3_address_code(p->right);
	int a = trace_ast_down_up_gen_3_address_code(p->left);
	string c;
	Symbol_var *symb = 0;
	Instruction *inst = 0;

	switch(p->semty)
	{
	// leaf node
	case sem_var:
		symb = p->symb_var;
		assert(symb);

		// todo symb->vr to be defined in "new =" to gen ssa
		return symb->vr;

	case sem_const_num:
		inst = new Instruction(p);
		inst->dst = p->vr_id;
		inst->const_num_value = p->const_value;
		three_addr_code.push_back(inst);
		return p->vr_id;

		// x = y : return x
	case op_assign:
		inst = new Instruction(p);
		inst->dst = a;
		inst->s1 = b;
		three_addr_code.push_back(inst);
		return a;

		// todo gen a assign inst
	case op_add:
		case op_sub:
		case op_mul:
		case op_div:

			inst = new Instruction(p);
			inst->dst = p->vr_id;
			inst->s1 = a;
			inst->s2 = b;
			three_addr_code.push_back(inst);
			return p->vr_id;

	case sem_return:
		inst = new Instruction(p);
		inst->s1 = a;
		three_addr_code.push_back(inst);
		return -1;

	case sem_var_declare:
		return -1;

		// todo
	case sem_func_declare:
	case sem_func_define:
	case sem_func_call:
		printf("todo sem_func* semty %d \n", p->semty);
		return -1;

	default:
		ERR("%d \n", p->semty);
		break;
	}

	return -1;
}
static void _gen_three_address_code(Scope *scp)
{
	LOG("scp %s \n", scp->name.c_str());

	for(auto it = scp->asts.begin(); it != scp->asts.end();) {
		LOG("%s, scp %s, ast %ld\n", __FUNCTION__, scp->name.c_str(), it - scp->asts.begin());
		Ast *p = *it;

		trace_ast_down_up_gen_3_address_code(p);
		it++;
	}

	for(Scope *p : scp->clds) {
		_gen_three_address_code(p);
	}
	return;
}
static void dump()
{
	printf("\n========== inst ==========\n");
	for(auto &r : three_addr_code) {
//		Ast *p = global_unique_vrid_tbl[r->dst];
		Ast *p = r->ast;

		switch(p->semty)
		{
		case sem_const_num:
			printf("const:\t %%%d num %d\n", r->dst, p->const_value);
			break;

		case op_assign:
			printf("assign:\t %%%d %s %%%d\n", r->dst, p->tk.src.c_str(), r->s1);
			break;

		case op_add:
		case op_sub:
		case op_mul:
		case op_div:
			printf("op:\t %%%d = %%%d %s %%%d\n", r->dst, r->s1, p->tk.src.c_str(), r->s2);
			break;

		case sem_var_declare:
			printf("del:\t %s[%s] %%%d\n", p->tk.src.c_str(), p->symb_var->unique_name.c_str(),p->symb_var->vr);
			break;

		case sem_var:
			printf("var:\t %%%d %s\n", r->dst, p->tk.src.c_str());
			break;

		case sem_func_call:
			printf("func call:\t %s\n", p->tk.src.c_str());
			break;

		case sem_return:
			printf("return:\t %s %%%d\n", p->tk.src.c_str(), r->s1);
			break;

		default:
			ERR();
			break;
		}
	}
}
void gen_three_address_code()
{
	_gen_three_address_code(&file_scope);
//	dump_ast();
	dump();
}
