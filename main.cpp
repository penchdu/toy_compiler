/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "h.h"

bool dump_token = false;

Tokens tokens;
Scope file_scope;
Scope *scope;
map<string, Symbol_var*> global_unique_src_name_table;
int vrid = 0;
int scope_id = 0;
vector<string> ir;

struct Vr{
	Sem_type sem;
	int t;
	int s1;
	int s2;

	Ast *p = 0;
	Var_type ty;
	int value;
	string str;
};
vector<Vr> irs;
static int tid = 0;

int gen_ir_from_ast(Ast *p)
{
//	printf("%p\n", p);
	assert(p);
	int l = -1;
	int r = -1;

	if(p->left)
		l = gen_ir_from_ast(p->left);

	if(p->right)
		r = gen_ir_from_ast(p->right);

	Vr ir;
	switch(p->semty)
	{
	case op_add:
		case op_sub:
		case op_mul:
		case op_div:
		assert(l != -1 && r != -1);

		ir.p = p;
		ir.s1 = l;
		ir.s2 = r;
//			ir.expr
		break;

	case op_assign:
		assert(l != -1 && r != -1);
		assert(p->left->semty != sem_const_num);
		ir.t = l;
		ir.p = p;
		ir.s1 = l;
		ir.s2 = r;
		break;

	case sem_return:
		assert(r == -1);
		ir.p = p;
		ir.s2 = r;
		break;

	case sem_var:
		ir.ty = INT;
		ir.p = p;
		ir.str = p->src_name;
		break;

	case sem_const_num:
		ir.ty = INT;
		ir.value = p->const_value;
		ir.p = p;
		ir.str = p->src_name;
		break;

	default:
		printf("%d\n", p->semty);
		assert(0);
		return -1;
	}

	ir.sem = p->semty;
	irs.push_back(ir);
	return ir.t;
}
int gen_ir_from_scope(Scope *scp)
{
	for(Ast *p : scp->asts) {
		gen_ir_from_ast(p);
	}

	for(Scope *p : scp->clds) {
		gen_ir_from_scope(p);
	}

	return 0;
}
int gen_ir()
{
	gen_ir_from_scope(&file_scope);

	printf("%zu\n", irs.size());
	for(Vr &r : irs){
//		string s = string("t") + std::to_string(r.id) + " ="
//		+ " " + string("t") + std::to_string(r.l)
//		+ " " + r.p->tk.str;
//		+ " " + string("t") + std::to_string(r.r);
//
//		printf("%s\n", s.c_str());


		printf("t%d = t%d %s t%d\n", r.t, r.s1, r.p->src_name.c_str(), r.s2);

	}

	return 0;
}


int main(int argc, char **argv)
{
//	assert(argc >= 2);
//	string source_file = argv[1];
//	probe_arg();
//	if(argc > 2) {
//		string option = argv[2];
//		if(option == "-dump-token")
//			dump_token = true;
//	}
//	FILE *fp = fopen(source_file.c_str(), "r");
	FILE *fp = fopen("./t1", "r");
	assert(fp);
	dump_token = true;

	scope = &file_scope;
	scope->id = scope_id++;
	scope->name = "b" + std::to_string(scope->id);
	scope->is_virtual_scope = false;
	scope->parent = 0;

	lexer(fp);
	parser();

	return 0;
}

