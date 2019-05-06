/*
 * File:	parser.cpp
 *
 * Description:	This file contains the public and private function and
 *		variable definitions for the recursive-descent parser for
 *		Simple C.
 */

# include <cstdlib>
# include <iostream>

# include "lexer.h"
# include "tokens.h"
# include "checker.h"
# include "Scope.h"
# include "Symbol.h"
# include "Type.h"

using std::string;

static int lookahead;
static string lexbuf;
static string structName;

static void expression();
static void statement();

/*
 * Function:	error
 *
 * Description:	Report a syntax error to standard error.
 */

static void error()
{
    if (lookahead == DONE)
	report("syntax error at end of file");
    else
	report("syntax error at '%s'", lexbuf);

    exit(EXIT_FAILURE);
}


/*
 * Function:	match
 *
 * Description:	Match the next token against the specified token.  A
 *		failure indicates a syntax error and will terminate the
 *		program since our parser does not do error recovery.
 */

static void match(int t)
{
    if (lookahead != t)
	error();

    lookahead = lexan(lexbuf);
}


/*
 * Function:  grab
 * 
 * Description: Matches the next token, then returns a copy of it.
 */
static string grab(int t)
{
  string cpy = lexbuf;
  match(t);

  return cpy;
}


/*
 * Function:  grabNumber
 * 
 * Description: Matches the next token, then returns a copy of it,
 *    assuming the value is a positive integer.
 */
static unsigned grabNumber()
{
  return strtoul(grab(NUM).c_str(), NULL, 0);
}


/*
 * Function:	isSpecifier
 *
 * Description:	Return whether the given token is a type specifier.
 */

static bool isSpecifier(int token)
{
  return token == INT || token == LONG || token == STRUCT;
}


/*
 * Function:	specifier
 *
 * Description:	Parse a type specifier.  Simple C has int, long, and
 *		structure types.
 *
 *		specifier:
 *		  int
 *		  long
 *		  struct identifier
 */

static int specifier()
{
  if (lookahead == INT) 
  {
    match(INT);
    return INT;
  }

  if (lookahead == LONG) 
  {
    match(LONG);
    return LONG;
  }

  match(STRUCT);
  structName = grab(ID);
  return STRUCT;
}


/*
 * Function:	pointers
 *
 * Description:	Parse pointer declarators (i.e., zero or more asterisks).
 *
 *		pointers:
 *		  empty
 *		  * pointers
 */

static unsigned pointers()
{
  unsigned num = 0;
    
  while (lookahead == '*')
  {
    match('*');
    ++num;
  }

  return num;
}


/*
 * Function:	declarator
 *
 * Description:	Parse a declarator, which in Simple C is either a simple
 *		variable or an array, with optional pointer declarators.
 *
 *		declarator:
 *		  pointers identifier
 *		  pointers identifier [ num ]
 */

static void declarator(int spec)
{
  const unsigned indirection = pointers();
  const string name = grab(ID);

  if (lookahead == '[') 
  {
    match('[');

    const unsigned length = grabNumber();
    declareVariable(name, Type(spec, indirection, length));

    match(']');
  }
  else
    declareVariable(name, Type(spec, indirection));
}


/*
 * Function:	declaration
 *
 * Description:	Parse a local variable declaration.  Global declarations
 *		are handled separately since we need to detect a function
 *		as a special case.
 *
 *		declaration:
 *		  specifier declarator-list ;
 *
 *		declarator-list:
 *		  declarator
 *		  declarator , declarator-list
 */

static void declaration()
{
  const int spec = specifier();

  declarator(spec);

  while (lookahead == ',') 
  {
    match(',');
    declarator(spec);
  }

  match(';');
}


/*
 * Function:	declarations
 *
 * Description:	Parse a possibly empty sequence of declarations.
 *
 *		declarations:
 *		  empty
 *		  declaration declarations
 */

static void declarations()
{
  while (isSpecifier(lookahead))
	  declaration();
}


/*
 * Function:	argument
 *
 * Description:	Parse an argument to a function call.  The only place
 *		string literals are allowed in Simple C is here, to
 *		enable calling printf(), scanf(), and the like.
 *
 *		argument:
 *		  string
 *		  expression
 */

static void argument()
{
  if (lookahead == STRING)
	  match(STRING);
  else
	  expression();
}


/*
 * Function:	primaryExpression
 *
 * Description:	Parse a primary expression.
 *
 *		primary-expression:
 *		  ( expression )
 *		  identifier ( argument-list )
 *		  identifier ( )
 *		  identifier
 *		  num
 *
 *		argument-list:
 *		  argument
 *		  argument , argument-list
 */

static void primaryExpression(bool lparenMatched)
{
  if (lparenMatched) 
  {
	  expression();
	  match(')');
  } 
  else if (lookahead == NUM) 
  {
	  match(NUM);
  } 
  else if (lookahead == ID) 
  {
    const string name = grab(ID);

	  if (lookahead == '(') 
    {
	    match('(');

	    if (lookahead != ')') 
      {
        argument();

        while (lookahead == ',')
        {
          match(',');
          argument();
        }
	    }

	    match(')');
    }

    /*
      The identifier must be declared in the current scope or in an enclosing scope [E4].
     */
    checkIdentifier(name);
  } 
  else error();
}


/*
 * Function:	postfixExpression
 *
 * Description:	Parse a postfix expression.
 *
 *		postfix-expression:
 *		  primary-expression
 *		  postfix-expression [ expression ]
 *		  postfix-expresion . identifier
 *		  postfix-expression -> identifier
 */

static void postfixExpression(bool lparenMatched)
{
  primaryExpression(lparenMatched);

  while (1) 
  {
    if (lookahead == '[') 
    {
        match('[');
        expression();
        match(']');
    } 
    else if (lookahead == '.') 
    {
        match('.');
        match(ID);

    } 
    else if (lookahead == ARROW) 
    {
        match(ARROW);
        match(ID);
    } 
    else break;
  }
}


/*
 * Function:	prefixExpression
 *
 * Description:	Parse a prefix expression.
 *
 *		prefix-expression:
 *		  postfix-expression
 *		  ! prefix-expression
 *		  - prefix-expression
 *		  * prefix-expression
 *		  & prefix-expression
 *		  sizeof ( expression )
 *		  ( specifier pointers ) prefix-expression
 */

static void prefixExpression()
{
  if (lookahead == '!') 
  {
	  match('!');
	  prefixExpression();
  } 
  else if (lookahead == '-') 
  {
	  match('-');
	  prefixExpression();
  } 
  else if (lookahead == '*') 
  {
  	match('*');
	  prefixExpression();
  } 
  else if (lookahead == '&') 
  {
  	match('&');
	  prefixExpression();
  } 
  else if (lookahead == SIZEOF) 
  {
  	match(SIZEOF);
	  match('(');
	  expression();
	  match(')');
  } 
  else if (lookahead == '(') 
  {
	  match('(');

	  if (isSpecifier(lookahead)) 
    {
	    specifier();
	    pointers();
	    match(')');
	    prefixExpression();
    } 
    else
	    postfixExpression(true);

  } 
  else
	  postfixExpression(false);
}


/*
 * Function:	multiplicativeExpression
 *
 * Description:	Parse a multiplicative expression.
 *
 *		multiplicative-expression:
 *		  prefix-expression
 *		  multiplicative-expression * prefix-expression
 *		  multiplicative-expression / prefix-expression
 *		  multiplicative-expression % prefix-expression
 */

static void multiplicativeExpression()
{
  prefixExpression();

  while (1) 
  {
	  if (lookahead == '*') 
    {
	    match('*');
	    prefixExpression();
	  } 
    else if (lookahead == '/') 
    {
	    match('/');
	    prefixExpression();
	  } 
    else if (lookahead == '%') 
    {
	    match('%');
	    prefixExpression();
	  } 
    else
	    break;
  }
}


/*
 * Function:	additiveExpression
 *
 * Description:	Parse an additive expression.
 *
 *		additive-expression:
 *		  multiplicative-expression
 *		  additive-expression + multiplicative-expression
 *		  additive-expression - multiplicative-expression
 */

static void additiveExpression()
{
  multiplicativeExpression();

  while (1) {
  	if (lookahead == '+') 
    {
	    match('+');
	    multiplicativeExpression();
	  } 
    else if (lookahead == '-') 
    {
	    match('-');
	    multiplicativeExpression();
	  } 
    else break;
  }
}


/*
 * Function:	relationalExpression
 *
 * Description:	Parse a relational expression.  Note that Simple C does not
 *		have shift operators, so we go immediately to additive
 *		expressions.
 *
 *		relational-expression:
 *		  additive-expression
 *		  relational-expression < additive-expression
 *		  relational-expression > additive-expression
 *		  relational-expression <= additive-expression
 *		  relational-expression >= additive-expression
 */

static void relationalExpression()
{
  additiveExpression();

  while (1) {
	  if (lookahead == '<') 
    {
	    match('<');
	    additiveExpression();
	  } 
    else if (lookahead == '>') 
    {
	    match('>');
	    additiveExpression();
	  } 
    else if (lookahead == LEQ) 
    {
	    match(LEQ);
	    additiveExpression();
	  } 
    else if (lookahead == GEQ) 
    {
	    match(GEQ);
	    additiveExpression();
	  } 
    else break;
  }
}


/*
 * Function:	equalityExpression
 *
 * Description:	Parse an equality expression.
 *
 *		equality-expression:
 *		  relational-expression
 *		  equality-expression == relational-expression
 *		  equality-expression != relational-expression
 */

static void equalityExpression()
{
  relationalExpression();

  while (1) {
    if (lookahead == EQL) 
    {
      match(EQL);
      relationalExpression();
    } 
    else if (lookahead == NEQ) 
    {
      match(NEQ);
      relationalExpression();
    } 
    else break;
  }
}


/*
 * Function:	logicalAndExpression
 *
 * Description:	Parse a logical-and expression.  Note that Simple C does
 *		not have bitwise-and expressions.
 *
 *		logical-and-expression:
 *		  equality-expression
 *		  logical-and-expression && equality-expression
 */

static void logicalAndExpression()
{
  equalityExpression();

  while (lookahead == AND) 
  {
    match(AND);
    equalityExpression();
  }
}


/*
 * Function:	expression
 *
 * Description:	Parse an expression, or more specifically, a logical-or
 *		expression, since Simple C does not allow comma or
 *		assignment as an expression operator.
 *
 *		expression:
 *		  logical-and-expression
 *		  expression || logical-and-expression
 */

static void expression()
{
  logicalAndExpression();

  while (lookahead == OR) 
  {
    match(OR);
    logicalAndExpression();
  }
}


/*
 * Function:	statements
 *
 * Description:	Parse a possibly empty sequence of statements.  Rather than
 *		checking if the next token starts a statement, we check if
 *		the next token ends the sequence, since a sequence of
 *		statements is always terminated by a closing brace.
 *
 *		statements:
 *		  empty
 *		  statement statements
 */

static void statements()
{
  while (lookahead != '}')
	  statement();
}


/*
 * Function:	statement
 *
 * Description:	Parse a statement.  Note that Simple C has so few
 *		statements that we handle them all in this one function.
 *
 *		statement:
 *		  { declarations statements }
 *		  return expression ;
 *		  while ( expression ) statement
 *		  if ( expression ) statement
 *		  if ( expression ) statement else statement
 *		  expression = expression ;
 *		  expression ;
 */

static void statement()
{
  if (lookahead == '{') 
  {
    match('{');

    /* The scope of the block begins before the declarations and persists until the end of the statements. */
    openScope();
    declarations();
    statements();
    closeScope();
    /* and persists until the end of the statements. */

    match('}');
  } 
  else if (lookahead == RETURN) 
  {
    match(RETURN);
    expression();
    match(';');
  } 
  else if (lookahead == WHILE) 
  {
    match(WHILE);
    match('(');
    expression();
    match(')');
    statement();
  } 
  else if (lookahead == IF) 
  {
    match(IF);
    match('(');
    expression();
    match(')');
    statement();

    if (lookahead == ELSE) 
    {
        match(ELSE);
        statement();
    }
  } 
  else 
  {
    expression();

    if (lookahead == '=') 
    {
        match('=');
        expression();
    }

    match(';');
  }
}


/*
 * Function:	parameter
 *
 * Description:	Parse a parameter, which in Simple C is always a simple
 *		variable with optional pointer declarators.
 *
 *		parameter:
 *		  specifier pointers ID
 */

static Type parameter()
{
  const int spec = specifier();
  const unsigned indirection = pointers();
  const string name = grab(ID);
  
  const Type type = Type(spec, indirection);
  declareVariable(name, type);

  return type;
}


/*
 * Function:	parameters
 *
 * Description:	Parse the parameters of a function, but not the opening or
 *		closing parentheses.
 *
 *		parameters:
 *		  void
 *		  parameter-list
 *
 *		parameter-list:
 *		  parameter
 *		  parameter , parameter-list
 */

static Parameters* parameters()
{
  unsigned spec;

  if (lookahead == VOID)
  {
    spec = VOID;
	  match(VOID);

    if (lookahead == ')')
      return nullptr;
  }
  else if (lookahead == ')')
    return nullptr;
  else
    spec = specifier();

  Parameters* params = new Parameters();
  const unsigned indirection = pointers();
  const string name = grab(ID);

  const Type type = Type(spec, indirection);
  /* Each parameter is declared in the current scope */
  declareVariable(name, type);
  params->push_back(type);

  while (lookahead == ',') 
  {
      match(',');
      params->push_back(parameter());
  }

  return params;
}


/*
 * Function:	globalDeclarator
 *
 * Description:	Parse a declarator, which in Simple C is either a simple
 *		variable, an array, or a function, with optional pointer
 *		declarators.
 *
 *		global-declarator:
 *		  pointers identifier
 *		  pointers identifier ( )
 *		  pointers identifier [ num ]
 */

static void globalDeclarator(int spec)
{
  const unsigned indirection = pointers();
  const string name = grab(ID);

  if (lookahead == '(') 
  {
	  match('(');

    // set parameters to nullptr as we are "ignoring any parameters.", for now
    Parameters* params = parameters();
    declareFunction(name, Type(spec, indirection, params));

	  match(')');
  } 
  else if (lookahead == '[') 
  {
    match('[');

    const unsigned length = grabNumber();
    declareVariable(name, Type(spec, indirection, length));

    match(']');
  }
  else
    declareVariable(name, Type(spec, indirection));
}


/*
 * Function:	remainingDeclarators
 *
 * Description:	Parse any remaining global declarators after the first.
 *
 * 		remaining-declarators
 * 		  ;
 * 		  , global-declarator remaining-declarators
 */

static void remainingDeclarators(int spec)
{
  while (lookahead == ',') 
  {
	  match(',');
	  globalDeclarator(spec);
  }

  match(';');
}


/*
 * Function:	globalOrFunction
 *
 * Description:	Parse a global declaration or function definition.
 *
 * 		global-or-function:
 * 		  struct idenfifier { declaration declarations } ;
 * 		  specifier pointers identifier remaining-decls
 * 		  specifier pointers identifier ( ) remaining-decls 
 * 		  specifier pointers identifier [ NUM ] remaining-decls
 * 		  specifier pointers identifier ( parameters ) { ... }
 */

static void globalOrFunction()
{
  Parameters* params = new Parameters();
  const unsigned spec = specifier();

  if (spec == STRUCT && lookahead == '{') 
  {
    defineStruct(structName);

    match('{');

    /* The scope of the structure begins immediately before the first declaration */
    openScope();
    declaration();
    declarations();
    closeScope();
    /* and persists until immediately after the declarations */
    /* at which point the type definition is considered complete. */

    match('}');
    match(';');
  } 
  else 
  {
    const unsigned indirection = pointers();
    const string name = grab(ID);

    if (spec == STRUCT && indirection == 0)
      checkStruct(structName, name);

    if (lookahead == '[') 
    {
      match('[');

      const unsigned length = grabNumber();
      declareVariable(name, Type(spec, indirection, length));

      match(']');

      remainingDeclarators(spec);
    } 
    else if (lookahead == '(')
    {
      /* The function is both declared and defined in the current translation unit. */
      match('(');
      
      if (lookahead == ')')
      {
        params = nullptr;
        
        if (spec == STRUCT)
          declareStruct(name, Type(spec, indirection, params), structName);
        else
          declareFunction(name, Type(spec, indirection, params));

        match(')');

        remainingDeclarators(spec);
      }
      else
      {
        /* The scope of the function begins immediately after the identifier */
        openScope();

        Parameters* params = parameters();
        defineFunction(name, Type(spec, indirection, params));

        match(')');
        match('{');

        declarations();
        statements();
        closeScope();
        /* and persists until the end of statements. */

        match('}');
	    }
    } 
    else
    {
      /* Each variable is declared in the current scope. */
      declareVariable(name, Type(spec, indirection));
      remainingDeclarators(spec);
    }
  }
}


/*
 * Function:	main
 *
 * Description:	Analyze the standard input stream.
 */

int main()
{
  openScope();

  lookahead = lexan(lexbuf);
  while (lookahead != DONE)
	  globalOrFunction();

  closeScope();
  exit(EXIT_SUCCESS);
}
