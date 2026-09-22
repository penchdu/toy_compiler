///*
// * a.cpp
// *
// *  Created on: 2026年9月5日
// *      Author: x
// */
//
//#include "h.h"
//#include "x64_back_end.h"
//#include "scope.h"
//
//extern Scope file_scp;
//
//const McInfo mc_info[MC_INVALID + 1] = {
//        [MC_LI] = {1, "mov"},
//        [MC_LD] = {3, "mov"},
//        [MC_ST] = {3, "mov"},
//
//        [MC_ASSIGN] = {1, "mov"},
//        [MC_ADD] = {1, "add"},
//        [MC_SUB] = {1, "sub"},
//        [MC_IMUL] = {3, "imul"},
//        [MC_DIV] = {10, "div"},
//
//        [MC_CMP_LT] = {1, "cmplt"},
//        [MC_CMP_LE] = {1, "cmple"},
//        [MC_CMP_E] = {1, "cmpe"},
//        [MC_CMP_GE] = {1, "cmpge"},
//        [MC_CMP_GT] = {1, "cmpgt"},
//        [MC_CMP_NE] = {1, "cmpne"},
//
//        [MC_LOGIC_AND] = {1, "logic_and"},
//
//        [MC_RET] = {1, "ret"},
//};
//
//struct Op2mc{
//	enum SemOperator op;
//	MachineCodeType mc;
//	char *mc_code = 0;
//};
//Op2mc op2mc[] = {
//        {OP_ASSIGN, MC_ASSIGN},
//        {OP_ADD, MC_ADD},
//        {OP_SUB, MC_SUB},
//        {OP_MUL, MC_IMUL},
//        {OP_DIV, MC_DIV},
//
//        {OP_CMP_LT, MC_CMP_LT},
//        {OP_CMP_LE, MC_CMP_LE},
//        {OP_CMP_E, MC_CMP_E},
//        {OP_CMP_GE, MC_CMP_GE},
//        {OP_CMP_GT, MC_CMP_GT},
//        {OP_CMP_NE, MC_CMP_NE},
//
//        {OP_LOGIC_AND, MC_LOGIC_AND},
//};
//
//static void gen_op_mc(vector<X64mc> &x64mc, MachineCodeType mc, const string &ori_sem,
//        int tac_dst, int tac_s1, int tac_s2)
//{
//	X64mc inst;
//
//	inst = X64mc(MC_ASSIGN, tac_dst, tac_s1);
//	inst.ori_sem = ori_sem;
//	x64mc.push_back(inst);
//
//	inst = X64mc(mc, tac_dst, tac_s2);
//	inst.ori_sem = ori_sem;
//	x64mc.push_back(inst);
//}
//static void gen_scope_mc(Scope *scp)
//{
//	LOG("scp %s", scp->name.c_str());
//
//	X64mc inst;
//
//	for (auto &tac : scp->tacs)
//	{
//		Semantic sem = tac->ast->sem;
//		if (sem == SEM_OPERATOR)
//		{
//			if (tac->ast->op == OP_ASSIGN)
//			{
//				inst = X64mc(MC_ASSIGN, tac->dst, tac->s1);
//				inst.ori_sem = "assign";
//				x64mc.push_back(inst);
//				continue;
//			}
//
//			// CMP 通常不需要写回目标寄存器，逻辑上可能不对, LOGIC_AND ?
//			MachineCodeType mc = op2mc[tac->ast->op].mc;
//			gen_op_mc(x64mc, mc, mc_info[mc].mc_code, tac->dst, tac->s1, tac->s2);
//			continue;
//		}
//
//		switch (sem)
//		{
//		case SEM_CONST_NUM:
//			inst = X64mc(MC_LI, tac->dst);
//			inst.const_num = tac->const_num_value;
//			inst.ori_sem = "li";
//			x64mc.push_back(inst);
//			break;
//
//		case SEM_RETURN:
//			inst = X64mc(MC_RET, tac->s1);
//			inst.ori_sem = "ret";
//			x64mc.push_back(inst);
//			break;
//
//			// todo
//		case SEM_FUNC_CALL:
//			ERR("todo sem_func* semty %d \n", sem);
//			break;
//
//		case SEM_OPERATOR:
//			default:
//			ERR("%d \n", sem);
//			break;
//		}
//	}
//
//	for (Scope *p : scp->clds)
//		gen_scope_mc(p);
//}
//void dump_mc(vector<X64mc> &v)
//{
//	for (auto &mc : v)
//	{
//		MachineCodeType ty = mc.mcty;
//
//		switch (ty)
//		{
//		case MC_LI:
//			printf("%s %%%d, num %d ", mc.asm_code.c_str(), mc.s1, mc.const_num);
//			PRINT_MORE
//			break;
//
//		case MC_LD:
//			printf("%s %%%d, dword ptr [%d] ", mc.asm_code.c_str(), mc.s1, mc.of1);
//			PRINT_MORE
//			break;
//
//		case MC_ST:
//			printf("%s dword ptr [%d], %%%d ", mc.asm_code.c_str(), mc.of1, mc.s1);
//			PRINT_MORE
//			break;
//
//		case MC_ASSIGN:
//			case MC_ADD:
//			case MC_SUB:
//			case MC_IMUL:
//			printf("%s %%%d, %%%d ", mc.asm_code.c_str(), mc.s1, mc.s2);
//			PRINT_MORE
//			break;
//
//		case MC_DIV:
//			printf("mov eax, %%%d", mc.s1);
//			PRINT_MORE
//
//			printf("cdq \n");
//			printf("idiv %%%d \t div \n", mc.s2);
//			printf("mov %%%d, eax \t div \n", mc.s1);
//			break;
//
//		case MC_RET:
//			printf("mov eax, %%%d", mc.s1);
//			PRINT_MORE
//			break;
//
//		default:
//			ERR("%d \n", ty);
//			break;
//		}
//	}
//}
//void dump_scope(Scope *scp)
//{
////	LOG("scp %s", scp->name.c_str());
//	printf("scp %s:\n", scp->name.c_str());
//
//	dump_mc(x64mc);
//
//	for (Scope *p : scp->clds)
//		dump_scope(p);
//	return;
//}
//static void dump_ori_mc()
//{
//	printf("\n========== mc ==========\n");
//
//	// push rbp
//	// mov rbp, rsp
//	// sub rsp, <num>
//	printf("push rbp\n");
//	printf("mov rbp, rsp\n");
//	printf("sub rsp, %d\n\n", vrm.offset);
//
//	dump_scope(&file_scp);
//
//	printf("\nmov rsp, rbp \n");
//	printf("pop rbp \n");
//	printf("ret \n\n");
//}
//
//void gen_machine_code()
//{
//	gen_scope_mc(&file_scp);
//	dump_ori_mc();
//}
