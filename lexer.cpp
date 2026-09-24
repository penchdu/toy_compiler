/*
 * lexer.cpp
 *
 *  Created on: 2026年9月6日
 *      Author: x
 */

#include "frontend.h"

Tokens tokens;

static int lexer_getc(FILE *fp)
{
	int c = fgetc(fp);
	return c;
}

/*
 * Skip a single-line comment (// ... \n).
 * Consumes everything up to and including '\n', or until EOF if no newline exists.
 */
static void skip_line_comment(FILE *fp)
{
	int c;
	while ((c = lexer_getc(fp)) != EOF) {
		if (c == '\n')
			return;
	}
	// File ended without newline — just return normally, c == EOF
}

static void skip_block_comment(FILE *fp)
{
	int c;
	while ((c = lexer_getc(fp)) != EOF) {
		if (c == '*') {
			int next = lexer_getc(fp);
			if (next == '/')
				return;         // Found closing */
			ungetc(next, fp);   // Put back the non-'/' char
		}
	}
	// Unterminated block comment — file ended without */
}

// Get a single-character operator / delimiter token.
static Token get_single_op_token(int c)
{
	Token tk;

	switch (c)
	{
	case '=':
		tk.stamp = TK_ASSIGN;
		break;

	case '+':
		tk.stamp = TK_ADD;
		break;

	case '-':
		tk.stamp = TK_SUB;
		break;

	case '*':
		tk.stamp = TK_MUL;
		break;

	case '(':
		tk.stamp = TK_PAREN_L;
		break;

	case ')':
		tk.stamp = TK_PAREN_R;
		break;

	case '{':
		tk.stamp = TK_BRACE_L;
		break;

	case '}':
		tk.stamp = TK_BRACE_R;
		break;

	case ':':
		tk.stamp = TK_LABEL;
		break;

	case ';':
		tk.stamp = TK_SEMICOLON;
		break;

	default:
		tk.stamp = TK_INVALID;
		return tk;
	}

	tk.src = c;
	return tk;
}

// Get a comparison / logic operator token (handles two-char disambiguation like ==, <=, &&, etc.)
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
			tk.stamp = TK_CMP_LE;
			tk.src = "<=";
		}
		else
		{
			ungetc(next, fp);
			tk.stamp = TK_CMP_LT;
			tk.src = "<";
		}
		break;

	case '>':
		next = lexer_getc(fp);

		if (next == '=')
		{
			tk.stamp = TK_CMP_GE;
			tk.src = ">=";
		}
		else
		{
			ungetc(next, fp);
			tk.stamp = TK_CMP_GT;
			tk.src = ">";
		}
		break;

	case '=':
		next = lexer_getc(fp);

		if (next == '=')
		{
			tk.stamp = TK_CMP_E;
			tk.src = "==";
		}
		else
		{
			ungetc(next, fp);
			tk.stamp = TK_ASSIGN;
			tk.src = "=";
		}
		break;

	case '!':
		next = lexer_getc(fp);

		if (next == '=')
		{
			tk.stamp = TK_CMP_NE;
			tk.src = "!=";
		}
		else
		{
			ungetc(next, fp);
			tk.stamp = TK_INVALID;
			tk.src = "!";
		}
		break;

	case '&':
		next = lexer_getc(fp);

		if (next == '&')
		{
			tk.stamp = TK_LOGIC_AND;
			tk.src = "&&";
		}
		else
		{
			ungetc(next, fp);
			tk.stamp = TK_INVALID;
			tk.src = "&";
		}
		break;

	default:
		tk.stamp = TK_INVALID;
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
		if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
		{
			continue;   // Skip whitespace
		}

		if (c == '/')
		{
			int next = lexer_getc(fp);
			if (next == '/')
			{
				skip_line_comment(fp);
				continue;   // Loop back to get next real token
			}
			else if (next == '*')
			{
				skip_block_comment(fp);
				continue;   // Loop back to get next real token
			}
			else
			{
				ungetc(next, fp);   // Not a comment — put char back
				// Fall through to handle '/' as division operator
				tk.stamp = TK_DIV;
				tk.src = "/";
				return tk;
			}
		}

		break;  // Found a non-whitespace, non-comment character
	}

	if (c == EOF || c == '#')
		return Token
		{ "EOF", TK_EOF };

	if (c == '<' ||
			c == '>' ||
			c == '=' ||
			c == '!' ||
			c == '&')
	{

		return get_cmp_token(fp, c);
	}

	Token t = get_single_op_token(c);

	if (t.stamp != TK_INVALID)
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
			tk.stamp = TK_INT;

		else if (word == "float")
			tk.stamp = TK_FLOAT;

		else if (word == "if")
			tk.stamp = TK_IF;

		else if (word == "else")
			tk.stamp = TK_ELSE;

		else if (word == "while")
			tk.stamp = TK_WHILE;

		else if (word == "continue")
			tk.stamp = TK_CONTINUE;

		else if (word == "break")
			tk.stamp = TK_BREAK;

		else if (word == "return")
			tk.stamp = TK_RETURN;

		else
			tk.stamp = TK_VAR;

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

		tk.stamp = TK_CONST_NUM;
		tk.src = word;
		return tk;
	}

	// 6. unknown character
	word += static_cast<char>(c);
	tk.stamp = TK_INVALID;
	tk.src = word;

	return tk;
}

int lexer(FILE *fp)
{
	while (1)
	{
		Token tk = get_a_token_from_file(fp);
		if (tk.stamp == TK_EOF)
		{
			tokens.append(tk);
			break;
		}
		tokens.append(tk);
	}

	bool dump_token = 1;
	if (dump_token)
		tokens.dump();

	return 0;
}
