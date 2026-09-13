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
map<string, Symbol_var*> global_unique_src_name_tbl;
map<int, int> const_num_vr_tbl;
int scope_id = 0;
vector<Instruction*> three_addr_code;
vector<Instruction*> machine_code;
Virtual_reg vreg;





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
	sem_analysis();
	gen_three_address_code();
	gen_machine_code();


	return 0;
}

