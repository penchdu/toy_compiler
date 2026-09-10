/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"

static string trace_ast_down_up_gen_inst(Ast *p)
{
	if(!p)
		return "";

	string a = trace_ast_down_up_gen_inst(p->left);
	string b = trace_ast_down_up_gen_inst(p->right);
	string c;
	Symbol_var *symb = 0;

	switch(p->semty)
	{
	// leaf node
	case sem_var:
		symb = p->symb_var;
		assert(symb);

		// todo symb->vr to be defined in new =
		symb->vr_name = "%" + std::to_string(symb->vr);
		return symb->vr_name;

		// todo gen a leaf inst
	case sem_const_num:
		return std::to_string(p->const_value);

		// todo gen a assign inst
	case op_assign:
		c = a + " " + p->source_code + " " + b;
		ir.push_back(c);
		return a;

		// todo gen a assign inst
	case op_add:
		case op_sub:
		case op_mul:
		case op_div:
			p->vr = vrid++;
			p->vr_name = "%" + std::to_string(p->vr);
			c = p->vr_name + " = " + a + " " + p->source_code + " " + b;
			ir.push_back(c);
			return p->vr_name;

	case sem_return:
		c = p->source_code + " " + a;
		ir.push_back(c);
		return "";

	case sem_declare:
		return "";

	case sem_func:
	default:
		ERR("%d \n", p->semty);
		break;
	}

	return 0;
}
void _gen_vr(Scope *scp)
{
	LOG("scp %s \n", scp->name.c_str());

	for(auto it = scp->asts.begin(); it != scp->asts.end();) {
		LOG("%s, scp %s, ast %ld\n", __FUNCTION__, scp->name.c_str(), it - scp->asts.begin());
		Ast *p = *it;

		trace_ast_down_up_gen_inst(p);
		it++;
	}

	for(Scope *p : scp->clds) {
		_gen_vr(p);
	}
	return;
}
void dump_vr()
{
	printf("\n========== ir ==========\n");
	for(auto &r : ir) {
		printf("%s\n", r.c_str());
	}
}
void gen_vr()
{
	_gen_vr(&file_scope);
	dump_ast();
	dump_vr();
}
