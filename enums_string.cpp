#include "enums.h"
// from enums.h

const char *TokenStamp_string[TK_EOF + 1] = {
	[TK_ASSIGN] = "TK_ASSIGN",
	[TK_ADD] = "TK_ADD",
	[TK_SUB] = "TK_SUB",
	[TK_MUL] = "TK_MUL",
	[TK_DIV] = "TK_DIV",

	[TK_CMP_LT] = "TK_CMP_LT",
	[TK_CMP_LE] = "TK_CMP_LE",
	[TK_CMP_GE] = "TK_CMP_GE",
	[TK_CMP_GT] = "TK_CMP_GT",
	[TK_CMP_E] = "TK_CMP_E",
	[TK_CMP_NE] = "TK_CMP_NE",

	[TK_LOGIC_AND] = "TK_LOGIC_AND",
	[TK_OP_ALL] = "TK_OP_ALL",

	[TK_INT] = "TK_INT",
	[TK_FLOAT] = "TK_FLOAT",
	[TK_VAR] = "TK_VAR",
	[TK_CONST_NUM] = "TK_CONST_NUM",

	[TK_IF] = "TK_IF",
	[TK_ELSE] = "TK_ELSE",
	[TK_WHILE] = "TK_WHILE",
	[TK_CONTINUE] = "TK_CONTINUE",
	[TK_BREAK] = "TK_BREAK",

	[TK_PAREN_L] = "TK_PAREN_L",
	[TK_PAREN_R] = "TK_PAREN_R",
	[TK_BRACE_L] = "TK_BRACE_L",
	[TK_BRACE_R] = "TK_BRACE_R",

	[TK_LABEL] = "TK_LABEL",
	[TK_SEMICOLON] = "TK_SEMICOLON",
	[TK_RETURN] = "TK_RETURN",

	[TK_INVALID] = "TK_INVALID",
	[TK_EOF] = "TK_EOF",
};

const char *SemOperator_string[OP_ALL + 1] = {
	[OP_ASSIGN] = "OP_ASSIGN",
	[OP_ADD] = "OP_ADD",
	[OP_SUB] = "OP_SUB",
	[OP_MUL] = "OP_MUL",
	[OP_DIV] = "OP_DIV",

	[OP_CMP_E] = "OP_CMP_E",
	[OP_CMP_NE] = "OP_CMP_NE",
	[OP_CMP_L] = "OP_CMP_L",
	[OP_CMP_LE] = "OP_CMP_LE",
	[OP_CMP_G] = "OP_CMP_G",
	[OP_CMP_GE] = "OP_CMP_GE",

	[OP_LOGIC_AND] = "OP_LOGIC_AND",
	[OP_LOGIC_OR] = "OP_LOGIC_OR",

	[OP_SAVE_RET_VALUE] = "OP_SAVE_RET_VALUE",
	[OP_ALL] = "OP_ALL",
};

const char *Semantic_string[SEM_INVALID + 1] = {
	[SEM_OPERATOR] = "SEM_OPERATOR",

	[SEM_VAR_DECLARE] = "SEM_VAR_DECLARE",
	[SEM_VAR] = "SEM_VAR",
	[SEM_CONST_NUM] = "SEM_CONST_NUM",

	[SEM_COND_JMP] = "SEM_COND_JMP",
	[SEM_JMP] = "SEM_JMP",
	[SEM_ELSE] = "SEM_ELSE",
	[SEM_WHILE_BODY] = "SEM_WHILE_BODY",
	[SEM_CONTINUE] = "SEM_CONTINUE",
	[SEM_BREAK] = "SEM_BREAK",

	[SEM_LABEL] = "SEM_LABEL",

	[SEM_FUNC_DECLARE] = "SEM_FUNC_DECLARE",
	[SEM_FUNC_DEFINE] = "SEM_FUNC_DEFINE",
	[SEM_FUNC_CALL] = "SEM_FUNC_CALL",

	[SEM_SAVE_RET_VALUE_AND_JMP] = "SEM_SAVE_RET_VALUE_AND_JMP",

	[SEM_FILE_SCOPE] = "SEM_FILE_SCOPE",
	[SEM_UNNAMED_SCOPE] = "SEM_UNNAMED_SCOPE",

	[SEM_NONE] = "SEM_NONE",
	[SEM_INVALID] = "SEM_INVALID",
};

const char *Type_string[INVALID_TYPE + 1] = {
	[INT] = "INT",
	[FLOAT] = "FLOAT",
	[VOID] = "VOID",
	[INVALID_TYPE] = "INVALID_TYPE",
};

const char *SemanticNodeType_string[Semantic_node_func + 1] = {
	[Semantic_node_ast] = "Semantic_node_ast",
	[Semantic_node_scope] = "Semantic_node_scope",
	[Semantic_node_func] = "Semantic_node_func",
};

