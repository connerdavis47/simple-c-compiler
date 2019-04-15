/*
 * Author:  Conner Davis
 * File:	  tokens.h
 *
 * Description:	
 *  This file contains an enumeration of all the valid types of 
 *  tokens that may be found inside a Simple C source file.
 */

# ifndef TOKENS_H
# define TOKENS_H

enum 
{

  // valid keywords in Simple C
  AUTO = 256, BREAK, CASE, CHAR, CONST, CONTINUE, DEFAULT, DO, 
  DOUBLE, ELSE, ENUM, EXTERN, FLOAT, FOR, GOTO, IF, INT, LONG, 
  REGISTER, RETURN, SHORT, SIGNED, SIZEOF, STATIC, STRUCT, 
  SWITCH, TYPEDEF, UNION, UNSIGNED, VOID, VOLATILE, WHILE,

  // valid operations in Simple C
  OR, AND, EQL, NEQ, LEQ, GEQ, INC, DEC, ARROW,

  // special values used by the parser
  ID, NUM, STRING, ERROR, DONE

};

# endif /* TOKENS_H */
