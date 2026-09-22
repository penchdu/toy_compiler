/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"

extern Scope file_scp;
vector<ThreeAddrCode*> three_addr_code;

//static int sem_if_scope(Ast *p)
//{
//	Scope *scp = p->this_scp;
//	Ast *cond = scp->asts[0];
//	assert(cond->op >= OP_CMP_LT && cond->op <= OP_CMP_NE);
//	cond->vr_id;

//	ThreeAddrCode *inst = new ThreeAddrCode(p);
//	inst->dst = p->vr_id;
//	inst->s1 = a;
//	inst->s2 = b;
//	three_addr_code.push_back(inst);
//}

static int trace_ast_down_up_gen_3_address_code(Ast *p)
{
	if (!p || p->sem == SEM_VAR_DECLARE)
			return -1;

	int b = trace_ast_down_up_gen_3_address_code(p->right);
	int a = trace_ast_down_up_gen_3_address_code(p->left);
	SymbolVar *symb = 0;
	ThreeAddrCode *inst = 0;

	switch(p->sem)
	{
	// leaf node
	case SEM_VAR:
		symb = p->symb_var;
		assert(symb);

		// todo symb->vr to be defined in "new =" to gen ssa
		return symb->vr;

	case SEM_CONST_NUM:
		inst = new ThreeAddrCode(p);
		inst->dst = p->vr_id;
		inst->const_num_value = p->const_value;
		three_addr_code.push_back(inst);
		return p->vr_id;

		case SEM_OPERATOR:
				if (p->op == OP_ASSIGN)
				{
					// x = y : return x
					inst = new ThreeAddrCode(p);
					inst->dst = a;
					inst->s1 = b;
					three_addr_code.push_back(inst);
					return a;
				}
				inst = new ThreeAddrCode(p);
				inst->dst = p->vr_id;
				inst->s1 = a;
				inst->s2 = b;
				three_addr_code.push_back(inst);
				return p->vr_id;

	case SEM_RETURN:
		inst = new ThreeAddrCode(p);
		inst->s1 = a;
		three_addr_code.push_back(inst);
		return -1;

	case SEM_VAR_DECLARE:
		return -1;

		// todo
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
static void gen_tac(Scope *scp)
{
	LOG("scp %s \n", scp->name.c_str());

	// SEM_IF scope have only one ast
//	if (scp->sem == SEM_IF)
//	{
//		Ast *cond = scp->asts[0];
//		cond->vr_id;
//	}
//	// then and SEM_ELSE scope have jmp_in
//	if (scp->sem == SEM_ELSE)
//	{
//	}
//	if (scp->sem == SEM_JMP_UNIT)	// have no ast
//	{
//	}

	for(auto it = scp->asts.begin(); it != scp->asts.end();) {
		LOG("%s, scp %s, ast %ld\n", __FUNCTION__, scp->name.c_str(), it - scp->asts.begin());
		Ast *p = *it;

		trace_ast_down_up_gen_3_address_code(p);
		it++;
	}

	for(Scope *p : scp->clds) {
		gen_tac(p);
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
		case SEM_CONST_NUM:
			printf("const:\t %%%d num %d\n", r->dst, p->const_value);
			break;

		case OP_ASSIGN:
			printf("assign:\t %%%d %s %%%d\n", r->dst, p->tk.src.c_str(), r->s1);
			break;

		case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
			printf("op:\t %%%d = %%%d %s %%%d\n", r->dst, r->s1, p->tk.src.c_str(), r->s2);
			break;

		case SEM_VAR_DECLARE:
			printf("del:\t %s[%s] %%%d\n", p->tk.src.c_str(), p->symb_var->unique_name.c_str(),p->symb_var->vr);
			break;

		case SEM_VAR:
			printf("var:\t %%%d %s\n", r->dst, p->tk.src.c_str());
			break;

		case SEM_FUNC_CALL:
			printf("func call:\t %s\n", p->tk.src.c_str());
			break;

		case SEM_RETURN:
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
	gen_tac(&file_scp);
//	dump_ast();
//	dump();
}
