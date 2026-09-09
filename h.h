/*
 * h.h
 *
 *  Created on: 2026年9月6日
 *      Author: x
 */

#ifndef H_H_
#define H_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <stack>

using std::vector;
using std::string;
using std::map;
using std::deque;

extern bool dump_token;

enum Token_type{
	tk_assign,
	tk_add,
	tk_sub,
	tk_mul,
	tk_div,
	tk_op_all,

	tk_var,
	tk_const_num,

	tk_int,
	tk_float,

	tk_lparen,
	tk_rparen,
	tk_lbrace,
	tk_rbrace,

    tk_semicolon,         // ;
    tk_return,
    tk_invalid,
    tk_EOF,
};
struct Token {
	string src_name;
	Token_type type = tk_EOF;
};
class Tokens{
public:
	void append(Token t){
		v.push_back(t);
	}
	Token get(){
		if(i == v.size()){
			return {"", tk_EOF};
		}
		return v[i++];
	}
	void unget(){
		if(i <= 0) {
			return;
		}
		i--;
	}
	Token peek(){
		if(i == v.size()){
			return {"", tk_EOF};
		}
		return v[i];
	}

	Token prew(){
		if(i <= 0){
			return {"", tk_EOF};
		}
		return v[i - 1];
	}
	bool empty(){
		return i == v.size();
	}
	void dump() {
		printf("lexer: \n" );
		for(auto x : v)
			printf("%s ", x.src_name.c_str());
		printf("\n");
	}

private:
	vector<Token> v;
	int i = 0;
};


enum Sem_type{
	op_assign,
	op_add,
	op_sub,
	op_mul,
	op_div,
	op_all,

	sem_declare,
	sem_var,
	sem_const_num,

	sem_func,
	sem_return,

//	sem_if,
//	sem_while,
//	sem_none,
	sem_invalid,
};
enum Op_priority{
	op_semicolon_priority,

	op_declare_priority,
	op_assign_priority,

	op_add_priority,
	op_sub_priority = op_add_priority,
	op_mul_priority,
	op_div_priority = op_mul_priority,

	op_paren_priority,

	op_invalid_priority = -1,
};

enum Var_type{
	INT,
	FLOAT,
	VOID,
	INVALID_TYPE,
};

struct Scope;

struct Symbol_var{
	Sem_type semty;
	Var_type var_type = INT;
	int value;

	string src_name;
	string unique_name;		// global unique
	string explicit_unique_name;	// global unique

	int vr = 0;
	string vr_name;
	Scope *scp;
};
struct Symbol_func{
	Var_type return_type = INT;
	string name;
	int argc = 0;
	Scope *func_scope;
};
struct Ast {
public:
	Sem_type semty;
	string src_name;
	string sem_name;

	// op
	Op_priority op_prio;
	int op_operant_cnt = 2;
	int vr = -1;
	string vr_name;

	// var
	Var_type var_type;		// var_type in ast or symtable?
	Symbol_var *var_symb = 0;
	int const_value;

	Token tk;
	Ast *left = 0;
	Ast *right = 0;

	Scope *this_scp = 0;
	Scope *sem_home_scp = 0;
//	map<string, Symbol_var*> *this_table = 0;
//	map<string, Symbol_var*> *sem_home_table = 0;

	Ast(Token &token)
	{
		tk = token;
		src_name = token.src_name;

		switch(token.type)
		{
		case tk_assign:
			semty = op_assign;
			op_prio = op_assign_priority;
			break;
		case tk_add:
			semty = op_add;
			op_prio = op_add_priority;
			break;
		case tk_sub:
			semty = op_sub;
			op_prio = op_sub_priority;
			break;
		case tk_mul:
			semty = op_mul;
			op_prio = op_mul_priority;
			break;
		case tk_div:
			semty = op_div;
			op_prio = op_div_priority;
			break;

		case tk_return:
			semty = sem_return;
			sem_name = src_name;
			break;

//		case tk_int:
//		case tk_float:
//			semty = sem_declare;
//			sem_name = src_name;
//			break;

		case tk_var:
			semty = sem_var;
			var_type = INT;
			break;

		case tk_const_num:
			semty = sem_const_num;
			var_type = INT;
			const_value = atoi(token.src_name.c_str());
			break;

		default:
			break;
		}

		if(semty < op_all)
			sem_name = src_name;
	}
};

struct Scope{
public:
	int id;
	string name;
	Sem_type sem = sem_invalid;
//	Var_type return_type;

	vector<Ast*> asts;
	map<string, Symbol_var*> _var_table;
	map<string, Symbol_var*> *var_table = &_var_table;
	map<string, Symbol_func*> _func_table;
	map<string, Symbol_func*> *func_table = &_func_table;
//	Sem_type region_header = sem_none;

	Scope * parent;
	vector<Scope*> clds;
	bool is_virtual_scope;

	Scope* new_cld()
	{
		Scope* p = new Scope;
		Scope* real_parent;

		if(!this->is_virtual_scope)
			real_parent = this;
		else
			real_parent = this->parent;

		real_parent->clds.push_back(p);
		p->parent = real_parent;
		return p;
	}
};

extern Tokens tokens;
extern Scope file_scope;
extern Scope *scope;
extern map<string, Symbol_var*> global_unique_src_name_table;
extern int vrid;
extern int scope_id;
extern vector<string> ir;

int lexer(FILE *fp);
Ast* parse_stmt();
void _parser();
void parser();
void dump_ast();
int semantic_analysis(Scope *scp);
int gen_ir_from_scope(Scope *scp);
int gen_ir();
void dump_ir();

#include <cstdio>
#include <cstdlib>

#define ERR(fmt, ...) do{ \
    printf("%s:%d: error:  " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    exit(1);	\
}while(0)

//#define DEBUG
#ifdef DEBUG
	#define LOG(fmt, ...) do{ \
		printf("LOG %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
	}while(0)
#else
	#define LOG(fmt, ...)
#endif

#endif /* H_H_ */
