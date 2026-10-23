/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"

extern Scope file_scp;

static int trace_ast_down_up_gen_3_address_code(vector<Tac> &tacs, Ast *p)
{
	if (!p || p->sem_stamp == SEM_VAR_DECLARE)
		return -1;

	int b = trace_ast_down_up_gen_3_address_code(tacs, p->right);
	int a = trace_ast_down_up_gen_3_address_code(tacs, p->left);
	Tac t;

	switch (p->sem_stamp)
	{
	// leaf node
	case SEM_VAR:
	case SEM_FUNC_ARG:
		assert(p->symb);

		// todo symb->vr to be defined in "new =" to gen ssa
		return p->symb->vr;

	case SEM_IMMEDIATE:
		assert(p->symb);
		return p->symb->vr;

	case SEM_OPERATOR:
		if (p->op == OP_LOGIC_AND || p->op == OP_LOGIC_OR)
			ERR("op: %d", p->op);

		if (p->op == OP_ASSIGN)
		{
			if (a == b) // not put into tac
				return a;
			// x = y : return x
			t = Tac(p);
			t.dst = a;
			t.s1 = b;
			tacs.push_back(t);
			return a;
		}
		if (p->op == OP_TEST)
		{
			t = Tac(p);
			t.s1 = a;
			tacs.push_back(t);
			return -1;
		}

		t = Tac(p);
		t.dst = p->vr;
		t.s1 = a;
		t.s2 = b;
		tacs.push_back(t);
		return p->vr;

	case SEM_SAVE_RET_VALUE:
		t = Tac(p);
		t.s1 = a;
		tacs.push_back(t);
		break;

	case SEM_VAR_DECLARE:
		return -1;

		// todo
	case SEM_FUNC_DECLARE:
		case SEM_FUNC_DEFINE:
		case SEM_FUNC_CALL:
		printf("todo sem_func* semty %d \n", p->sem_stamp);
		return -1;

	default:
		ERR("%d \n", p->sem_stamp);
		break;
	}

	return -1;
}

static bool gen_tac(Scope *scp)
{
//	LOG("scp %s", scp->name.c_str());
	if (scp->sem_stamp == SEM_bb_terminate)
	{
		printf("%s have a SEM_bb_terminate scope %s\n", scp->parent->name.c_str(), scp->name.c_str());
		return true;
	}

	for (auto *p : scp->asts)
		trace_ast_down_up_gen_3_address_code(scp->tac, p);

	bool bb_terminated = 0;
	for (Scope *p : scp->clds)
	{
		bb_terminated = gen_tac(p);
		if (bb_terminated)
			break;
	}

	if (bb_terminated && scp->sem_stamp == SEM_INVALID)
	{
		//scp->jmp_in.size() == 0
//			scp->sem_stamp != SEM_COND_JMP
//			&& scp->sem_stamp != SEM_WHILE_BODY
//			&& scp->sem_stamp != SEM_JMP
		return true;
	}
	return false;
}

static void dump_tac(Scope *scp)
{
	if (scp->jmp_in.size() > 0)
		printf("\nlabel %d:\n", scp->id);

	for (Tac &r : scp->tac)
	{
		Ast *p = r.ast;

		switch (p->sem_stamp)
		{
		case SEM_IMMEDIATE:
			printf("const:\t %%%d num %d\n", r.dst, p->const_value);
			break;

		case SEM_OPERATOR:
			if (p->op == OP_ASSIGN)
				printf("assign:\t %%%d %s %%%d\n", r.dst, p->tk.src.c_str(), r.s1);
			else
				printf("op:\t %%%d = %%%d %s %%%d\n", r.dst, r.s1, p->tk.src.c_str(), r.s2);
			break;

		case SEM_VAR_DECLARE:
			printf("del:\t %s[%s] %%%d\n", p->tk.src.c_str(), p->symb->unique_name.c_str(), p->symb->vr);
			break;

		case SEM_VAR:
			printf("var:\t %%%d %s\n", r.dst, p->tk.src.c_str());
			break;

		case SEM_FUNC_CALL:
			printf("func call:\t %s\n", p->tk.src.c_str());
			break;

		case SEM_SAVE_RET_VALUE:
			printf("save_ret:\t %s %%%d\n", p->tk.src.c_str(), r.s1);
			break;

		default:
			ERR();
			break;
		}
	}

	if (scp->jmp_out != 0)
		printf("%d->%d:\n\n", scp->id, scp->jmp_out->id);

	for (Scope *p : scp->clds)
		dump_tac(p);
}

static void elimicate_transient_vr(Scope *scp)
{
	for(int i = 0; i < scp->tac.size(); )
	{
		if(i + 1 >= scp->tac.size())
			break;

		Tac &a = scp->tac[i];
		Tac &b = scp->tac[i + 1];

		if (a.dst < 0)
		{
			i++;
			continue;
		}

		Symbol *symb = declare_record.ast[a.dst]->symb;
		if(symb->stamp == SYMB_PRIVATE_transient
			&& b.ast->op == OP_ASSIGN
//			&& a.dst != a.s2
			&& b.s1 == a.dst)
		{
			a.dst = b.dst;
			scp->tac.erase(scp->tac.begin() + i + 1);
			continue;
		}

		i++;
	}

	for (Scope *p : scp->clds)
		elimicate_transient_vr(p);
}

void gen_three_address_code()
{
	gen_tac(&file_scp);
	dump_ast();

	printf("\n========== tac ==========\n");
	dump_tac(&file_scp);

	elimicate_transient_vr(&file_scp);

	printf("\n========== tac ==========\n");
	dump_tac(&file_scp);
}
