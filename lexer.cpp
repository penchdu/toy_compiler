/*
 * lexer.cpp
 *
 *  Created on: 2026年9月6日
 *      Author: x
 */

#include "h.h"

//enum Op_priority get_op_priority(enum Token_type type)
//{
//	switch(type)
//	{
//	case tk_assign:
//		return op_assign_priority;
//	case tk_add:
//		return op_add_priority;
//	case tk_sub:
//		return op_sub_priority;
//	case tk_mul:
//		return op_mul_priority;
//	case tk_div:
//		return op_div_priority;
//
//	case tk_left_round_bracket:
//	case tk_right_round_bracket:
//		return op_bracket_priority;
//
//	default:
//		return op_invalid_priority;
//	}
//
//	return op_invalid_priority;
//}

// get a word from fp, read until =+-*/(){};
//stripe blank char like blank '\t' '\n'...
Token get_single_op_token(int c)
{
	Token token;
    switch(c) {
    case '=':
    	token.type = tk_assign;
    	break;
    case '+':
    	token.type = tk_add;
    	break;
    case '-':
    	token.type = tk_sub;
    	break;
    case '*':
    	token.type = tk_mul;
    	break;
    case '/':
    	token.type = tk_div;
    	break;

    case '(':
    	token.type = tk_lparen;
    	break;
    case ')':
    	token.type = tk_rparen;
    	break;

    case '{':
    	token.type = tk_lbrace;
    	break;

    case '}':
    	token.type = tk_rbrace;
    	break;

    case ';':
    	token.type = tk_semicolon;
    	break;

    default:
    	return token;
    }

    token.source_code = c;
    return token;
}

int my_getc(FILE *fp)
{
	int c = fgetc(fp);
//	LOG("%c ", c);
	return c;
}
Token get_a_token_from_file(FILE *fp)
{
	Token token;
	string word;
	int c;

	// 1. skip whitespace
	while((c = my_getc(fp)) != EOF) {
		if(c != ' ' && c != '\t' && c != '\n' && c != '\r') {
			break;
		}
	}

	if(c == EOF)
		return Token{"EOF", tk_EOF};

	// 2. single-character token
    Token t = get_single_op_token(c);
    if(t.type != tk_EOF)
    {
        return t;
    }
//    LOG("%d \n", type);

	// 3. identifier / keyword
	if((c >= 'a' && c <= 'z') ||
	        (c >= 'A' && c <= 'Z') ||
	        c == '_')
	{
		word += static_cast<char>(c);

		while((c = my_getc(fp)) != EOF) {
			if((c >= 'a' && c <= 'z') ||
			        (c >= 'A' && c <= 'Z') ||
			        (c >= '0' && c <= '9') ||
			        c == '_') {
				word += static_cast<char>(c);
			}
			else {
				// this character belongs to the next token
				ungetc(c, fp);
				break;
			}
		}

		if(word == "int")
			token.type = tk_int;
		else if(word == "return")
			token.type = tk_return;
		else
			token.type = tk_var;

		token.source_code = word;
		return token;
	}

	// 4. integer literal
	if(c >= '0' && c <= '9') {
		word += static_cast<char>(c);

		while((c = my_getc(fp)) != EOF) {
			if(c >= '0' && c <= '9') {
				word += static_cast<char>(c);
			}
			else {
				ungetc(c, fp);
				break;
			}
		}

		token.type = tk_const_num;
		token.source_code = word;
		return token;
	}

	// 5. unknown character
	word += static_cast<char>(c);
	token.source_code = word;
	return token;
}

int lexer(FILE *fp)
{
	while(1) {
		Token token = get_a_token_from_file(fp);
		if(token.type == tk_EOF)
			break;

		tokens.append(token);
	}

	if(dump_token) {
		tokens.dump();
	}

	return 0;
}
