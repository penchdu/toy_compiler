/*
 * a.cpp
 *
 *  Created on: 2026年9月5日
 *      Author: x
 */

#include "frontend.h"
#include "basic_block.h"

extern Scope file_scp;
vector<BasicBlock> basic_blocks;

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
	if (!p || p->sem_stamp == SEM_VAR_DECLARE)
		return -1;

	vector<Tac> &tacs = basic_blocks.back().tacs;

	int b = trace_ast_down_up_gen_3_address_code(p->right);
	int a = trace_ast_down_up_gen_3_address_code(p->left);
	SymbolVar *symb = 0;
	Tac t;

	switch (p->sem_stamp)
	{
	// leaf node
	case SEM_VAR:
		symb = p->symb_var;
		assert(symb);

		// todo symb->vr to be defined in "new =" to gen ssa
		return symb->vr;

	case SEM_CONST_NUM:
		t = Tac(p);
		t.dst = p->vr_id;
		t.const_num_value = p->const_value;
		tacs.push_back(t);
		return p->vr_id;

	case SEM_OPERATOR:
		if (p->op == OP_ASSIGN)
		{
			// x = y : return x
			t = Tac(p);
			t.dst = a;
			t.s1 = b;
			tacs.push_back(t);
			return a;
		}
		if (p->op == OP_LOGIC_AND || p->op == OP_LOGIC_OR)
			ERR("op: %d", p->op);

		t = Tac(p);
		t.dst = p->vr_id;
		t.s1 = a;
		t.s2 = b;
		tacs.push_back(t);
		return p->vr_id;

	case SEM_SAVE_RET_VALUE_AND_JMP:
		t = Tac(p);
		t.s1 = p->left->vr_id;
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
static void gen_tac(Scope *scp)
{
	LOG("scp %s", scp->name.c_str());

	if (scp->sem_stamp == SEM_FUNC_DEFINE
		|| scp->sem_stamp == SEM_COND_JMP	// if, while
		|| scp->sem_stamp == SEM_WHILE_BODY
		|| scp->sem_stamp == SEM_LABEL
		|| scp->jmp_in.size() > 0
		)
	{
		BasicBlock newbb;
		newbb.entry_label = scp;
		basic_blocks.push_back(newbb);
	}

	for (auto *p : scp->asts)
		trace_ast_down_up_gen_3_address_code(p);

	if (scp->jmp_out != 0)	// || scp->sem == SEM_COND_JMP || scp->sem == SEM_JMP)
	{
		// SEM_IF scope have only one ast, SEM_JMP have no ast
//		Ast *cond = scp->asts[0];
		BasicBlock &bb = basic_blocks.back();
		bb.exit_jmp = scp;
		bb.jmp_to = bb.exit_jmp->jmp_out;

		if (scp->sem_stamp == SEM_SAVE_RET_VALUE_AND_JMP)
		{
			assert(scp->asts.size() == 1);
			Ast *ret = scp->asts[0];
			assert(ret->op == OP_SAVE_RET_VALUE);
			assert(ret->left);	// function just support return int

//			BasicBlock &bb = basic_blocks.back();
//			Tac t = Tac(ret);
//			t.s1 = ret->left->vr_id;
//			bb.tacs.push_back(t);

			bb.exit_jmp = scp;
			bb.jmp_to = bb.exit_jmp->jmp_out;
		}

		BasicBlock newbb;
		basic_blocks.push_back(newbb);
	}

	for (Scope *p : scp->clds)
		gen_tac(p);
}

static void dump_tac()
{
	printf("\n========== tac ==========\n");
	for (BasicBlock &bb : basic_blocks)
	{
		if (bb.entry_label != 0)
			printf("\nlabel %d:\n", bb.entry_label->id);

		for (Tac &r : bb.tacs)
		{
			Ast *p = r.ast;

			switch (p->sem_stamp)
			{
			case SEM_CONST_NUM:
				printf("const:\t %%%d num %d\n", r.dst, p->const_value);
				break;

			case SEM_OPERATOR:
				if (p->op == OP_ASSIGN)
					printf("assign:\t %%%d %s %%%d\n", r.dst, p->tk.src.c_str(), r.s1);
				else
					printf("op:\t %%%d = %%%d %s %%%d\n", r.dst, r.s1, p->tk.src.c_str(), r.s2);
				break;

			case SEM_VAR_DECLARE:
				printf("del:\t %s[%s] %%%d\n", p->tk.src.c_str(), p->symb_var->unique_name.c_str(), p->symb_var->vr);
				break;

			case SEM_VAR:
				printf("var:\t %%%d %s\n", r.dst, p->tk.src.c_str());
				break;

			case SEM_FUNC_CALL:
				printf("func call:\t %s\n", p->tk.src.c_str());
				break;

			case SEM_SAVE_RET_VALUE_AND_JMP:
				printf("return:\t %s %%%d\n", p->tk.src.c_str(), r.s1);
				break;

			default:
				ERR();
				break;
			}
		}

		if (bb.exit_jmp != 0)
		{
			printf("%s %d->%d:\n\n", Semantic_string[bb.exit_jmp->sem_stamp],
			    bb.exit_jmp->id, bb.exit_jmp->jmp_out->id);
		}
	}
}
void gen_three_address_code()
{
	BasicBlock bb;
	bb.entry_label = &file_scp;
	basic_blocks.push_back(bb);

	gen_tac(&file_scp);
	dump_ast();
	dump_tac();
}
