/*
 * Author:  Conner Davis
 * File:	  parser.h
 *
 * Description:	
 *  This file contains a parser for Simple C. The parser is simply 
 *  one giant set of if-statements that handle all possible valid 
 *  inputs established by our fully factored Simple C grammar.
 */

# ifndef PARSER_H
# define PARSER_H

# include <string>

// (START) - Tooling

/**
 * Function: error( string& )
 * 
 * Description:
 *  Report an error to the standard error including a reference to 
 *  the calling function. Helps trace more precisely where things 
 *  went wrong.
 */
static void error( std::string );

/**
 * Function: keep( int )
 * 
 * Description:
 *  Match the provided token, but return its value with this
 *  function so that it can be referenced later. Typically
 *  used to pass a field to child recursive functions.
 */
static std::string keep( int );

/**
 * Function: match( int )
 * 
 * Description:
 *  Peeks into the lookahead to see if the provided token is found. 
 *  If it is, the token is "consumed" and we set the lookahead to 
 *  the next token.
 */
static void match( int );

/**
 * Function: match_specifier( )
 * 
 * Description:
 *  Matches a valid specifier, then returns its type. For example,
 *  a valid return could be STRUCT or INT.
 */
static int match_specifier( );

/**
 * Function: print( std::string )
 * 
 * Description:
 *  Prints a result to the standard output which is how we are
 *  graded.
 */
static void print( std::string );

/**
 * Function: is_specifier( )
 * 
 * Description:
 *  Returns true if the current lookahead is a valid specifier.
 *  In Simple C, valid specifiers include int, long, and struct.
 */
static bool is_specifier( );

// (END) - Tooling

// (START) - Expressions

/**
 * Entry point for recursive tree processing expressions.
 */
static void expression( );
/**
 * Scans for logical comparison operators, && and ||.
 */
static void logical_cmp_expression( );
/**
 * Scans for equality operators, == and !=.
 */
static void equality_expression( );
/**
 * Scans for relational operators, <, >, <=, >=.
 */
static void relation_expression( );
/**
 * Scans for additive operators, + and -.
 */
static void add_expression( );
/**
 * Scans for multiplicative operators, *, /, %.
 */
static void multiply_expression( );
/**
 * Scans for operators that prefix an identifier,
 * !, &, *, -, sizeof(..).
 */
static void prefix_expression( );
/**
 * Scans for operators postfix to the identifier,
 * [], ->, and . (dot).
 */
static void post_expression( );
/**
 * Scans for type casts. In Simple C, an expression followed 
 * by a left parenthesis may instead be another expression.
 * We handle both of those cases here.
 */
static void cast_expression( );
/**
 * Finally, scans for numbers and identifiers. This is the
 * deepest level of the expression recursion tree, which means
 * that any symbols reaching this point will be one or the
 * other.
 * 
 * If the expression matches an identifier at this point, we
 * will look to see whether it is a function with parameters or
 * a variable.
 */
static void general_expression( );

// (END) - Expressions

// (START) - Functions

static void arguments( );
static void argument( );

static void parameters( );
static void parameter( );

static void pointers( );

// (END) - Functions

// (START) - Translation unit

static void statements( );
static void statement( );

static void declarations( );
static void declaration( );
static void declarator( int );

static void declarators( );
static void global_declarator( );

/**
 * The entry point of the parser, or in other words,
 * the very first function in the whole recursive tree.
 */
static void translation_unit( );

// (END) - Translation unit

# endif /* PARSER_H */
