/*
 * enums.h
 *
 *  Created on: 2026年9月17日
 *      Author: x
 */

#ifndef ENUMS_H_
#define ENUMS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define STR(x) #x

#define ERR(fmt, ...) do{ \
    printf("%s %d, error: " fmt "\n", __FUNCTION__, __LINE__ , ##__VA_ARGS__); \
    exit(1);	\
}while(0)

#define DEBUG
#ifdef DEBUG
#define LOG(fmt, ...) do{ \
		printf("%s %d, " fmt "\n", __FUNCTION__, __LINE__ , ##__VA_ARGS__); \
	}while(0)
#else
	#define LOG(fmt, ...)
#endif

#define TOKEN_LIST(X)	\
   X(TK_ASSIGN)		\
    X(TK_ADD)		\
    X(TK_SUB)		\
    X(TK_MUL)		\
    X(TK_DIV)		\
	X(TK_CMP_LT)	\
	X(TK_CMP_LE)	\
	X(TK_CMP_GE)	\
	X(TK_CMP_GT)    \
	X(TK_CMP_E)		\
	X(TK_CMP_NE)    \
					\
	X(TK_LOGIC_AND)	\
	X(TK_OP_ALL)    \
					\
	X(TK_INT)   	\
	X(TK_FLOAT)    	\
	X(TK_VAR)    	\
	X(TK_CONST_NUM) \
					\
	X(TK_IF)    	\
	X(TK_ELSE)    	\
	X(TK_WHILE)    	\
					\
	X(TK_PAREN_L)	/*(*/   \
	X(TK_PAREN_R)	/*)*/   \
	X(TK_BRACE_L)	/*{*/ \
	X(TK_BRACE_R)	/*}*/  \
					\
	X(TK_LABEL)    	\
	X(TK_SEMICOLON) \
	X(TK_RETURN)    \
					\
	X(TK_INVALID)   \
	X(TK_EOF)

#define ENUM_ITEM(name) name,
enum TokenType {
	TOKEN_LIST(ENUM_ITEM)
};
#undef ENUM_ITEM

//enum TokenType
//{
//	TK_ASSIGN,
//	TK_ADD,
//	TK_SUB,
//	TK_MUL,
//	TK_DIV,
//
//	TK_CMP_LT,
//	TK_CMP_LE,
//	TK_CMP_E,
//	TK_CMP_GE,
//	TK_CMP_GT,
//	TK_CMP_NE,
//
//	TK_LOGIC_AND,
//	TK_OP_ALL,
//
//	TK_INT,
//	TK_FLOAT,
//	TK_VAR,
//	TK_CONST_NUM,
//
//	TK_IF,
//	TK_WHILE,
//
//	TK_PAREN_L,		// (
//	TK_PAREN_R,		// )
//	TK_BRACE_L,		// {
//	TK_BRACE_R,		// }
//
//	TK_LABEL,
//	TK_SEMICOLON,         // ;
//	TK_RETURN,
//
//	TK_INVALID,
//	TK_EOF,
//};

#define SEMANTIC_LIST(X)	\
	    X(OP_ASSIGN)    \
		X(OP_ADD)    \
		X(OP_SUB)    \
		X(OP_MUL)    \
		X(OP_DIV)    \
						\
		X(OP_CMP_LT)    \
		X(OP_CMP_LE)    \
		X(OP_CMP_E)    \
		X(OP_CMP_GE)    \
		X(OP_CMP_GT)    \
		X(OP_CMP_NE)    \
							\
		X(OP_LOGIC_AND)    \
		X(OP_ALL)    		\
							\
		X(SEM_VAR_DECLARE)    \
		X(SEM_VAR)    			\
		X(SEM_CONST_NUM)    \
							\
		X(SEM_IF)    \
		X(SEM_ELSE)    \
		X(SEM_WHILE)    \
		X(SEM_LABEL)    \
		X(SEM_JMP)    \
								\
		X(SEM_FUNC_DECLARE)    \
		X(SEM_FUNC_DEFINE)    \
		X(SEM_FUNC_CALL)    \
		X(SEM_RETURN)    \
									\
		X(SEM_FILE_SCOPE)    \
		X(SEM_UNNAMED_SCOPE)    \
		X(SEM_NONE)    \
		X(SEM_INVALID)

#define ENUM_ITEM(name) name,
enum SemanticType {
	SEMANTIC_LIST(ENUM_ITEM)
};
#undef ENUM_ITEM

//enum SemanticType
//{
//	OP_ASSIGN,
//	OP_ADD,
//	OP_SUB,
//	OP_MUL,
//	OP_DIV,
//
//	OP_CMP_LT,
//	OP_CMP_LE,
//	OP_CMP_E,
//	OP_CMP_GE,
//	OP_CMP_GT,
//	OP_CMP_NE,
//
//	OP_LOGIC_AND,
//	OP_ALL,
//
//	SEM_VAR_DECLARE,
//	SEM_VAR,
//	SEM_CONST_NUM,
//
//	SEM_LABEL,
//	SEM_IF,
//	SEM_ELSE,
//	SEM_WHILE,
//
//	SEM_FUNC_DECLARE,
//	SEM_FUNC_DEFINE,
//	SEM_FUNC_CALL,
//	SEM_RETURN,
//
//	SEM_FILE_SCOPE,
//	SEM_UNNAMED_SCOPE,
//	SEM_NONE,
//	SEM_INVALID,
//};

enum OpPriority
{
	OP_SEMICOLON_PRIORITY,
	OP_DECLARE_PRIORITY,
	OP_ASSIGN_PRIORITY,

	OP_LOGIC_AND_PRIORITY,

	OP_CMP_LT_PRIORITY,
	OP_CMP_LE_PRIORITY = OP_CMP_LT_PRIORITY,
	OP_CMP_E_PRIORITY = OP_CMP_LT_PRIORITY,
	OP_CMP_GE_PRIORITY = OP_CMP_LT_PRIORITY,
	OP_CMP_GT_PRIORITY = OP_CMP_LT_PRIORITY,
	OP_CMP_NE_PRIORITY = OP_CMP_LT_PRIORITY,

	OP_ADD_PRIORITY,
	OP_SUB_PRIORITY = OP_ADD_PRIORITY,
	OP_MUL_PRIORITY,
	OP_DIV_PRIORITY = OP_MUL_PRIORITY,

	OP_PAREN_PRIORITY,

	OP_INVALID_PRIORITY = -1,
};
enum VarType
{
	INT,
	FLOAT,
	VOID,
	INVALID_TYPE,
};

enum BlockType
{

};

enum SemanticNodeType
{
	Semantic_node_ast,
	Semantic_node_scope,
	Semantic_node_func,
};

//struct Virtual_reg2{
//	struct vr_off{
//		int vr;
//		int off;
//		int size;
//		int live = true;
//	};
//
//	int new_vr(Ast *p)
//	{
//		id++;
//
//		int size = 4;
//		if(p->var_type == INT)
//			size = 4;
//
//		vr_off t{id, offset, size, true};
//		vr_tbl.push_back(t);
//		offset += size;
//		return id;
//	}
//	void rm_vr(unsigned int id){
//		if(id >= vr_tbl.size())
//			ERR();
//		if(!vr_tbl[id].live)
//			ERR();
//
//		vr_tbl[id].live = false;
//	}
//	int get_vr(unsigned int id){
//		if(id >= vr_tbl.size() || !vr_tbl[id].live)
//			ERR();
//
//		return vr_tbl[id].vr;
//	}
//	int get_vr_off(unsigned int id){
//		if(id >= vr_tbl.size() || !vr_tbl[id].live || vr_tbl[id].off < 0)
//			ERR("id %d, %zu\n", id, vr_tbl.size());
//
//		return vr_tbl[id].off;
//	}
//	void rebuild()
//	{
//		id = 0;
//		offset = 0;
//		for(auto &r : vr_tbl)
//		{
//			if(!r.live)
//				continue;
//
//			r.vr = id++;
//			r.off = offset;
//			offset += r.size;
//		}
//	}
//
//	int id = -1;
//	int offset = 0;
//	vector<vr_off> vr_tbl;
//};

#endif /* ENUMS_H_ */
