/*
 * parser.h
 *
 *  Created on: 2026年9月19日
 *      Author: x
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "h.h"

#define DEBUG
#ifdef DEBUG
#define PARSER_LOG(fmt, ...) do{ \
		printf("%s %d, scope-%s-%d,        " fmt "\n",	\
	__FUNCTION__, __LINE__ , \
	current_scope_pointer->name.c_str(), \
	current_scope_pointer->id, \
	##__VA_ARGS__); \
	}while(0)
#else
	#define LOG(fmt, ...)
#endif

extern Tokens tokens;
extern const char *tk_ty_names[];
extern const char *sem_ty_names[];

extern Scope file_scp;
extern Scope *current_scope_pointer;
extern int scope_id;
extern bool in_func_define;

Scope* new_scope_and_drop_in();
Scope* new_virtual_scp_and_drop_in();
void exit_current_scope();

Ast* parse_scope(bool eat = true);
Ast* case_tk_right_brace(bool eat = true);

Ast* parse_stmt();
Ast* parse_expr_with_paren();

Scope* case_tk_else();
Scope* case_tk_if();

#endif /* PARSER_H_ */
