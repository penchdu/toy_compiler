/*
 * lexer.cpp
 *
 *  Created on: 2026年9月6日
 *      Author: x
 */

#include "h.h"

Tokens tokens;

static int lexer_getc(FILE *fp)
{
	int c = fgetc(fp);
	return c;
}

// Get a single-character operator / delimiter token.
static Token get_single_op_token(int c)
{
	Token tk;

	switch (c)
	{
	case '=':
		tk.type = TK_ASSIGN;
		break;

	case '+':
		tk.type = TK_ADD;
		break;

	case '-':
		tk.type = TK_SUB;
		break;

	case '*':
		tk.type = TK_MUL;
		break;

	case '/':
		tk.type = TK_DIV;
		break;

	case '(':
		tk.type = TK_PAREN_L;
		break;

	case ')':
		tk.type = TK_PAREN_R;
		break;

	case '{':
		tk.type = TK_BRACE_L;
		break;

	case '}':
		tk.type = TK_BRACE_R;
		break;

	case ':':
		tk.type = TK_LABEL;
		break;

	case ';':
		tk.type = TK_SEMICOLON;
		break;

	default:
		tk.type = TK_INVALID;
		return tk;
	}

	tk.src = c;
	return tk;
}
static Token get_cmp_token(FILE *fp, int c)
{
	Token tk;
	int next;

	switch (c)
	{

	case '<':
		next = lexer_getc(fp);

		if (next == '=')
		{
			tk.type = TK_CMP_LE;
			tk.src = "<=";
		}
		else
		{
			ungetc(next, fp);
			tk.type = TK_CMP_LT;
			tk.src = "<";
		}
		break;

	case '>':
		next = lexer_getc(fp);

		if (next == '=')
		{
			tk.type = TK_CMP_GE;
			tk.src = ">=";
		}
		else
		{
			ungetc(next, fp);
			tk.type = TK_CMP_GT;
			tk.src = ">";
		}
		break;

	case '=':
		next = lexer_getc(fp);

		if (next == '=')
		{
			tk.type = TK_CMP_E;
			tk.src = "==";
		}
		else
		{
			ungetc(next, fp);
			tk.type = TK_ASSIGN;
			tk.src = "=";
		}
		break;

	case '!':
		next = lexer_getc(fp);

		if (next == '=')
		{
			tk.type = TK_CMP_NE;
			tk.src = "!=";
		}
		else
		{
			ungetc(next, fp);
			tk.type = TK_INVALID;
			tk.src = "!";
		}
		break;

	case '&':
		next = lexer_getc(fp);

		if (next == '&')
		{
			tk.type = TK_LOGIC_AND;
			tk.src = "&&";
		}
		else
		{
			ungetc(next, fp);
			tk.type = TK_INVALID;
			tk.src = "&";
		}
		break;

	default:
		tk.type = TK_INVALID;
		return tk;
	}

	return tk;
}

static Token get_a_token_from_file(FILE *fp)
{
	Token tk;
	string word;
	int c;

	while ((c = lexer_getc(fp)) != EOF)
	{
		if (c != ' ' &&
				c != '\t' &&
				c != '\n' &&
				c != '\r')
		{
			break;
		}
	}

	if (c == EOF || c == '#')
		return Token
		{ "EOF", TK_EOF };

	if (c == '<' ||
			c == '>' ||
			c == '=' ||
			c == '!')
	{

		return get_cmp_token(fp, c);
	}

	Token t = get_single_op_token(c);

	if (t.type != TK_INVALID)
	{
		return t;
	}

	// 4. identifier / keyword
	if ((c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			c == '_')
	{
		word += static_cast<char>(c);

		while ((c = lexer_getc(fp)) != EOF)
		{

			if ((c >= 'a' && c <= 'z') ||
					(c >= 'A' && c <= 'Z') ||
					(c >= '0' && c <= '9') ||
					c == '_')
			{

				word += static_cast<char>(c);
			}
			else
			{
				// this character belongs to the next token
				ungetc(c, fp);
				break;
			}
		}

		if (word == "int")
			tk.type = TK_INT;

		else if (word == "float")
			tk.type = TK_FLOAT;

		else if (word == "if")
			tk.type = TK_IF;

		else if (word == "while")
			tk.type = TK_WHILE;

		else if (word == "return")
			tk.type = TK_RETURN;

		else
			tk.type = TK_VAR;

		tk.src = word;
		return tk;
	}

	if (c >= '0' && c <= '9')
	{
		word += static_cast<char>(c);

		while ((c = lexer_getc(fp)) != EOF)
		{
			if (c >= '0' && c <= '9')
			{
				word += static_cast<char>(c);
			}
			else
			{
				ungetc(c, fp);
				break;
			}
		}

		tk.type = TK_CONST_NUM;
		tk.src = word;
		return tk;
	}

	// 6. unknown character
	word += static_cast<char>(c);
	tk.type = TK_INVALID;
	tk.src = word;

	return tk;
}

int lexer(FILE *fp)
{
	while (1)
	{
		Token tk = get_a_token_from_file(fp);
		if (tk.type == TK_EOF)
			break;

		tokens.append(tk);
	}

	bool dump_token = 1;
	if (dump_token)
		tokens.dump();

	return 0;
}
