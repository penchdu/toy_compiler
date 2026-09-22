/*
 * scope.h
 *
 *  Created on: 2026年9月21日
 *      Author: x
 */

#ifndef SCOPE_H_
#define SCOPE_H_

#include "frontend.h"
#include "x64_back_end.h"

extern Scope file_scp;

struct Scope
{
public:
	int id;
	string name;
	Semantic sem = SEM_INVALID;

	vector<Ast*> asts;
	map<string, SymbolVar*> _var_table;
	map<string, SymbolVar*> *var_table = &_var_table;
	map<string, SymbolFunc*> _func_table;
	map<string, SymbolFunc*> *func_table = &_func_table;
	//	Sem_type region_header = sem_none;

	// ifc
//	Scope *jmp_then = 0;
//	Scope *jmp_else = 0;

	// ift, iff
	vector<Scope*> jmp_in;
	Scope *jmp_out = 0;

	Scope *parent;
	vector<Scope*> clds;
	bool is_virtual;
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


#endif /* SCOPE_H_ */
