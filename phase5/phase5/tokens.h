/*
 * File:	tokens.h
 *
 * Description:	This file contains the token definitions for use by the
 *		lexical analyzer and parser for Simple C.  Single character
 *		tokens use their ASCII values, so we can refer to them
 *		either as character literals or as symbolic names.
 */

# ifndef TOKENS_H
# define TOKENS_H

enum {
    ASSIGN = '=', LTN = '<', GTN = '>', PLUS = '+', MINUS = '-',
    STAR = '*', DIV = '/', REM = '%', ADDR = '&', NOT = '!', DOT = '.',
    LPAREN = '(', RPAREN = ')', LBRACK = '[', RBRACK = ']',
    LBRACE = '{', RBRACE = '}', SEMI = ';', COLON = ':', COMMA = ',',

    AUTO = 256, BREAK, CASE, CHAR, CONST, CONTINUE, DEFAULT, DO, DOUBLE,
    ELSE, ENUM, EXTERN, FLOAT, FOR, GOTO, IF, INT, LONG, REGISTER,
    RETURN, SHORT, SIGNED, SIZEOF, STATIC, STRUCT, SWITCH, TYPEDEF,
    UNION, UNSIGNED, VOID, VOLATILE, WHILE,

    OR, AND, EQL, NEQ, LEQ, GEQ, INC, DEC, ARROW,
    ID, NUM, STRING, DONE = 0, ERROR = -1
};

# endif /* TOKENS_H */
