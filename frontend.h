/*
 * h.h
 *
 *  Created on: 2026年9月6日
 *      Author: x
 */

#ifndef FRONTEND_H_
#define FRONTEND_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <stack>
#include <algorithm>

using std::vector;
using std::string;
using std::map;
using std::deque;
using std::to_string;

#include "enums.h"

struct Token
{
	string src;
	TokenStamp stamp = TK_EOF;
};
class Tokens
{
public:
	void append(Token t)
	{
		v.push_back(t);
	}
	Token get()
	{
		if (i == v.size())
		{
			return
			{	"", TK_EOF};
		}
		return v[i++];
	}
	void unget()
	{
		if (i <= 0)
		{
			return;
		}
		i--;
	}
	Token peek()
	{
		if (i == v.size())
		{
			return
			{	"", TK_EOF};
		}
		return v[i];
	}

	Token prev()
	{
		if (i <= 0)
		{
			return
			{	"", TK_EOF};
		}
		return v[i - 1];
	}
	bool empty()
	{
		return i == v.size();
	}
	void dump()
	{
		printf("lexer: \n");
		for (auto x : v)
			printf("%s ", x.src.c_str());
		printf("\n");
	}

private:
	vector<Token> v;
	int i = 0;
};

struct Scope;

struct SymbolVar
{
//	Semantic_type semty;
	Type type = INT;
	//	int value;

	string *src;
	string unique_name;		// global unique
	string explicit_unique_name;	// global explicit unique

	int vr = -1;
	Scope *scp;

	vector<SymbolVar*> cld;
};
struct SymbolFunc
{
	Type return_type = INT;
	string name;
	int argc = 0;
	Scope *func_scope;
};
struct Ast {
public:
	Semantic sem_stamp;

	// op
	Operator op = OP_ALL;
	OpPriority op_prio;
	// for op, vr is temp vr
	int vr_id = -1;

	// var
	// for op, var_type is type of temp vr
	Type type;		// var_type in ast or symtable?
	SymbolVar *symb_var = 0;
	int const_value;

	Token tk;
	Ast *parent = 0;
	Ast *left = 0;
	Ast *right = 0;

	Scope *this_scp = 0;
	Scope *home_scp = 0;

	Ast(const Token &_token)
	{
		tk = _token;

		switch (_token.stamp)
		{
		case TK_ASSIGN:
			op = OP_ASSIGN;
			op_prio = OP_ASSIGN_PRIORITY;
			break;

		case TK_ADD:
			op = OP_ADD;
			op_prio = OP_ADD_PRIORITY;
			break;
		case TK_SUB:
			op = OP_SUB;
			op_prio = OP_SUB_PRIORITY;
			break;
		case TK_MUL:
			op = OP_MUL;
			op_prio = OP_MUL_PRIORITY;
			break;
		case TK_DIV:
			op = OP_DIV;
			op_prio = OP_DIV_PRIORITY;
			break;

		case TK_CMP_LT:
			op = OP_CMP_L;
			op_prio = OP_CMP_LT_PRIORITY;
			break;
		case TK_CMP_LE:
			op = OP_CMP_LE;
			op_prio = OP_CMP_LE_PRIORITY;
			break;
		case TK_CMP_E:
			op = OP_CMP_E;
			op_prio = OP_CMP_E_PRIORITY;
			break;
		case TK_CMP_GE:
			op = OP_CMP_GE;
			op_prio = OP_CMP_GE_PRIORITY;
			break;
		case TK_CMP_GT:
			op = OP_CMP_G;
			op_prio = OP_CMP_GT_PRIORITY;
			break;
		case TK_CMP_NE:
			op = OP_CMP_NE;
			op_prio = OP_CMP_NE_PRIORITY;
			break;

		case TK_LOGIC_AND:
			op = OP_LOGIC_AND;
			op_prio = OP_LOGIC_AND_PRIORITY;
			break;

		case TK_IF:
			sem_stamp = SEM_COND_JMP;
			break;
		case TK_ELSE:
//			sem_stamp = SEM_ELSE;
			break;

		case TK_INT:
			sem_stamp = SEM_VAR_DECLARE;
			type = get_declare_type(tk.stamp);
			break;

		case TK_VAR:
			sem_stamp = SEM_VAR;
			type = INT;
			break;

		case TK_CONST_NUM:
			sem_stamp = SEM_CONST_NUM;
			type = INT;
			const_value = atoi(_token.src.c_str());
			break;

		case TK_SEMICOLON:
			sem_stamp = SEM_NONE;
			break;

		case TK_RETURN:
			sem_stamp = SEM_SAVE_RET_VALUE;
//			op = OP_SAVE_RET_VALUE;
			break;

		default:
			ERR("unexpect token %s", _token.src.c_str());
			break;
		}

		if (op != OP_ALL)
			sem_stamp = SEM_OPERATOR;
	}
};

struct Scope
{
public:
	int id;
	string name;
	Semantic sem_stamp = SEM_INVALID;

	vector<Ast*> asts;
	map<string, SymbolVar*> _var_table;
	map<string, SymbolVar*> *var_table = &_var_table;
	map<string, SymbolFunc*> _func_table;
	map<string, SymbolFunc*> *func_table = &_func_table;
	//	Sem_type region_header = sem_none;

	Scope *parent;
	vector<Scope*> clds;
	bool is_virtual;

	// for if
//	Scope *jmp_then = 0;
//	Scope *jmp_else = 0;
	vector<Scope*> jmp_in;
	Scope *jmp_out = 0;

	// continue, break
	vector<Scope*> continue__break;
	bool bb_terminated;

	// for function
	Scope *return_label = 0;
	enum Type return_type = INVALID_TYPE;

	Scope* new_cld()
	{
		Scope *p = new Scope;
		Scope *real_parent;

		if (!this->is_virtual)
			real_parent = this;
		else
			real_parent = this->parent;

		real_parent->clds.push_back(p);
		p->parent = real_parent;
		return p;
	}
};

struct Tac {
	Tac(Ast *p = 0)
	{
		ast = p;
	}
	Ast *ast = 0;

	int dst = -1;
	int s1 = -1;
	int s2 = -1;

	int const_num_value = 0;
};

//struct BasicBlock
//{
//public:
//	int id;
//	string name;
//	Semantic sem = SEM_INVALID;
//
//	vector<Ast*> asts;
//
//	map<string, SymbolVar*> *var_table = 0;
//	map<string, SymbolFunc*> *func_table = 0;
//
//	Scope *parent;
//};
//struct SemanticNode
//{
//	SemanticNodeType nodety;
//	union
//	{
//		Ast *ast;
//		Scope *scp;
//	};
//};

struct VirtualRegManager
{
	int new_vr(Ast *p, int size = 4)
	{
		id++;
		offset += size;
		vr_off.push_back(offset);
		return id;
	}

	int get_vr_off(unsigned int id)
	{
		if (id >= vr_off.size() || vr_off[id] < 0)
		{
			ERR("id %d, %zu\n", id, vr_off.size());
			return -1;
		}
		return vr_off[id];
	}

	int id = -1;
	int offset = 0;
	vector<int> vr_off;
};



#if 0
#define PARSER_LOG(fmt, ...) do{ \
		printf("%s %d, scope-%s-%d,        " fmt "\n",	\
	__FUNCTION__, __LINE__ , \
	current_scope_pointer->name.c_str(), \
	current_scope_pointer->id, \
	##__VA_ARGS__); \
	}while(0)
#else
	#define PARSER_LOG(fmt, ...)
#endif


constexpr bool enable_bb_terminate = 1;

extern Tokens tokens;
extern Scope file_scp;
extern Scope *current_scope_pointer;
extern int scope_id;
extern bool in_func_define;
extern VirtualRegManager vregm;



Scope* new_scope_and_drop_in();
Scope* new_virtual_scope_and_drop_in();
void exit_current_scope();

Ast* parse_stmt();
Ast* parse_expr_with_paren();
void parse_scope(bool eat = true);
void case_tk_right_brace(bool eat = true);
void case_tk_if();
void case_tk_while();
void case_tk_continue();
void case_tk_break();
void make_bb_terminate();

int lexer(FILE *fp);
void dump_ast();
void parser();
void sem_analysis();
void gen_three_address_code();

#endif /* FRONTEND_H_ */
