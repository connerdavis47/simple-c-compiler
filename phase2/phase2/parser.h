/*
 * Author:  Conner Davis
 * File:	  parser.h
 *
 * Description:	
 *  This file contains a parser for Simple C. The parser is simply 
 *  one giant set of if-statements that handle all possible valid 
 *  inputs established by the reduced Simple C grammar.
 */

# ifndef PARSER_H
# define PARSER_H

# include <string>

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

static int match_specifier( );

/**
 * Function: print( std::string )
 * 
 * Description:
 *  Prints a result to the standard output which is how we are
 *  graded.
 */
static void print( std::string );

static bool is_specifier( );

// (START) - Expressions

static void expression( );
static void logical_cmp_expression( );
static void equality_expression( );
static void relation_expression( );
static void add_expression( );
static void multiply_expression( );
static void prefix_expression( );
static void post_expression( );
static void cast_expression( );
static void general_expression( );

// (END) - Expressions

// (START) - Statements

static void statements( );
static void statement( );
static void declarations( );
static void declaration( );
static void declarator( int );
static void pointers( );

// (END) - Statements

// (START) - Translation unit

static void translation_unit( );
static void parameters( );
static void parameter( );
static void declarators( );
static void global_declarator( );
static void arguments( );
static void argument( );

// (END) - Translation unit

# endif /* PARSER_H */
