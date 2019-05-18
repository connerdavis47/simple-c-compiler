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

using std::string;

static int lookahead;
static string lexbuf;

/**
 * The return type of the most recently parsed function.
 */
static Type returnType;
/**
 * True if the last number read by number() was a long integer.
 * False if that number was an integer.
 */
static bool isLong;

static Type expression( bool& lvalue );
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
 * Function:	number
 *
 * Description:	Match the next token as a number and return its value.
 */

static unsigned long number()
{
    const string buf = lexbuf;
    match(NUM);
	
	// note whether this number is a long for type checking
	isLong = buf.find("L") != string::npos;

    return strtoul(buf.c_str(), NULL, 0);
}


/*
 * Function:	identifier
 *
 * Description:	Match the next token as an identifier and return its name.
 */

static string identifier()
{
    const string buf = lexbuf;
    match(ID);
    return buf;
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

static string specifier()
{
    if (lookahead == INT) 
	{
		match(INT);
		return "int";
    }

    if (lookahead == LONG) 
	{
		match(LONG);
		return "long";
    }

    match(STRUCT);
    return identifier();
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
    unsigned count = 0;

    while (lookahead == '*') 
	{
		match('*');
		count ++;
    }

    return count;
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

static void declarator(const string& typespec)
{
    const unsigned indirection = pointers();
    const string name = identifier();

    if (lookahead == '[') 
	{
		match('[');
		declareVariable(name, Type(typespec, indirection, number()));
		match(']');
    } 
	else
		declareVariable(name, Type(typespec, indirection));
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
    const string typespec = specifier();

    declarator(typespec);
    while (lookahead == ',') 
	{
		match(',');
		declarator(typespec);
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

static Type argument( bool& lvalue )
{
	Type result;

    if (lookahead == STRING)
	{
		match(STRING);
		result = Type("string");
	}
    else
		result = expression(lvalue);

	return result;
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

static Type primaryExpression( bool lparenMatched, bool& lvalue )
{
	string name;
	Type expr;

    if (lparenMatched)
	{
		expression(lvalue);
		match(')');
    } 
	else if (lookahead == NUM) 
	{
		number();
		expr = isLong ? Type("long") : Type("int");

		lvalue = false;
    } 
	else if (lookahead == ID) 
	{
		name = identifier();
		expr = checkIdentifier(name)->type();

		if (lookahead == '(') 
		{
			Parameters args;
			match('(');

			if (lookahead != ')') 
			{
				args.push_back(argument(lvalue));

				while (lookahead == ',') 
				{
					match(',');
					args.push_back(argument(lvalue));
				}
			}

			match(')');
			expr = checkFunction(name, args);
		}
		else if (expr.isSimple())
			lvalue = true;
    } 
	else
		error();

	return expr;
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

static Type postfixExpression( bool lparenMatched, bool& lvalue )
{
	Type left, right;

    left = primaryExpression(lparenMatched, lvalue);
    while (1) 
	{
		if (lookahead == '[') 
		{
			match('[');
			right = expression(lvalue);
			left = checkArray(left, right);
			match(']');

			lvalue = true;
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

	return left;
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

static Type prefixExpression( bool& lvalue )
{
	Type expr;

    if (lookahead == '!') 
	{
		match('!');
		expr = prefixExpression(lvalue);
		expr = checkNot(expr);

		lvalue = false;
    } 
	else if (lookahead == '-') 
	{
		match('-');
		expr = prefixExpression(lvalue);
		expr = checkNegate(expr);

		lvalue = false;
    } 
	else if (lookahead == '*') 
	{
		match('*');
		expr = prefixExpression(lvalue);
		expr = checkDereference(expr);

		lvalue = true;
    } 
	else if (lookahead == '&') 
	{
		match('&');
		expr = prefixExpression(lvalue);
		expr = checkAddress(expr, lvalue);

		lvalue = false;
    } 
	else if (lookahead == SIZEOF) 
	{
		match(SIZEOF);
		match('(');
		expr = expression(lvalue);
		expr = checkSizeof(expr);
		match(')');

		lvalue = false;
    } 
	else if (lookahead == '(') 
	{
		match('(');

		if (isSpecifier(lookahead)) 
		{
			const string typespec = specifier();
			const unsigned indirection = pointers();
			match(')');
			const Type right = prefixExpression(lvalue);
			checkTypeCast(Type(typespec, indirection), right);

			lvalue = false;
		}
		else 
			expr = postfixExpression(true, lvalue);
    } 
	else 
		expr = postfixExpression(false, lvalue);

	return expr;
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

static Type multiplicativeExpression( bool& lvalue )
{
	Type left, right;

    left = prefixExpression(lvalue);
    while (1)
	{
		if (lookahead == '*') 
		{
			match('*');
			right = prefixExpression(lvalue);
			left = checkMultiply(left, right);
			lvalue = false;
		} 
		else if (lookahead == '/') 
		{
			match('/');
			right = prefixExpression(lvalue);
			left = checkDivide(left, right);
			lvalue = false;
		} 
		else if (lookahead == '%') 
		{
			match('%');
			right = prefixExpression(lvalue);
			left = checkRemainder(left, right);
			lvalue = false;
		} 
		else break;
    }

	return left;
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

static Type additiveExpression( bool& lvalue )
{
	Type left, right;

    left = multiplicativeExpression(lvalue);
    while (1) 
	{
		if (lookahead == '+') 
		{
			match('+');
			right = multiplicativeExpression(lvalue);
			left = checkAdd(left, right);
			lvalue = false;
		} 
		else if (lookahead == '-') 
		{
			match('-');
			right = multiplicativeExpression(lvalue);
			left = checkSubtract(left, right);
			lvalue = false;
		} 
		else break;
    }

	return left;
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

static Type relationalExpression( bool& lvalue )
{
    Type left, right;
	
	left = additiveExpression(lvalue);
    while (1) 
	{
		if (lookahead == '<') 
		{
			match('<');
			right = additiveExpression(lvalue);
			left = checkLessThan(left, right);
			lvalue = false;
		} 
		else if (lookahead == '>') 
		{
			match('>');
			right = additiveExpression(lvalue);
			left = checkGreaterThan(left, right);
			lvalue = false;
		} 
		else if (lookahead == LEQ) 
		{
			match(LEQ);
			right = additiveExpression(lvalue);
			left = checkLessOrEqual(left, right);
			lvalue = false;
		} 
		else if (lookahead == GEQ) 
		{
			match(GEQ);
			right = additiveExpression(lvalue);
			left = checkGreaterOrEqual(left, right);
			lvalue = false;
		} 
		else break;
    }

	return left;
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

static Type equalityExpression( bool& lvalue )
{
	Type left, right;
    
	left = relationalExpression(lvalue);
    while (1) 
	{
		if (lookahead == EQL) 
		{
			match(EQL);
			right = relationalExpression(lvalue);
			left = checkEqual(left, right);
			lvalue = false;
		} 
		else if (lookahead == NEQ) 
		{
			match(NEQ);
			right = relationalExpression(lvalue);
			left = checkNotEqual(left, right);
			lvalue = false;
		} 
		else break;
    }

	return left;
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

static Type logicalAndExpression( bool& lvalue )
{
	Type left, right;

    left = equalityExpression(lvalue);
    while (lookahead == AND) 
	{
		match(AND);
		right = equalityExpression(lvalue);
		left = checkLogicalAnd(left, right);
		lvalue = false;
    }

	return left;
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

static Type expression( bool& lvalue )
{
	Type left, right;

    left = logicalAndExpression(lvalue);
    while (lookahead == OR) 
	{
		match(OR);
		right = logicalAndExpression(lvalue);
		left = checkLogicalOr(left, right);
		lvalue = false;
    }

	return left;
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
	bool lvalue_init = false;

    if (lookahead == '{') 
	{
		match('{');
		openScope();
		declarations();
		statements();
		closeScope();
		match('}');
    } 
	else if (lookahead == RETURN) 
	{
		match(RETURN);
		checkReturn(expression(lvalue_init), returnType);
		match(';');
    } 
	else if (lookahead == WHILE) 
	{
		match(WHILE);
		match('(');
		checkTest(expression(lvalue_init));
		match(')');
		statement();
    } 
	else if (lookahead == IF) 
	{
		match(IF);
		match('(');
		checkTest(expression(lvalue_init));
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
		const Type left = expression(lvalue_init);
		bool rvalue = false;

		if (lookahead == '=') 
		{
			match('=');
			const Type right = expression(rvalue);
			checkAssignment(left, right, lvalue_init);
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
	const string typespec = specifier();
	const unsigned indirection = pointers();
	const string name = identifier();

	const Type type(typespec, indirection);
    declareParameter(name, type);
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
    Parameters* params = new Parameters();

    if (lookahead == VOID)
		match(VOID);

    else 
	{
		params->push_back(parameter());

		while (lookahead == ',') 
		{
			match(',');
			params->push_back(parameter());
		}
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

static void globalDeclarator(const string& typespec)
{
    const unsigned indirection = pointers();
    const string name = identifier();

    if (lookahead == '(') 
	{
		match('(');
		declareFunction(name, Type(typespec, indirection, nullptr));
		match(')');
    } 
	else if (lookahead == '[') 
	{
		match('[');
		declareVariable(name, Type(typespec, indirection, number()));
		match(']');
    } 
	else
		declareVariable(name, Type(typespec, indirection));
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

static void remainingDeclarators(const string& typespec)
{
    while (lookahead == ',') 
	{
		match(',');
		globalDeclarator(typespec);
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
    const string typespec = specifier();

    if (typespec != "int" && typespec != "long" && lookahead == '{') 
	{
		openStruct(typespec);
		match('{');
		declaration();
		declarations();
		closeStruct(typespec);
		match('}');
		match(';');
    } 
	else 
	{
		const unsigned indirection = pointers();
		const string name = identifier();

		if (lookahead == '[') 
		{
			match('[');
			declareVariable(name, Type(typespec, indirection, number()));
			match(']');
			remainingDeclarators(typespec);
		} 
		else if (lookahead == '(') 
		{
			match('(');

			if (lookahead == ')') 
			{
				declareFunction(name, Type(typespec, indirection, nullptr));
				match(')');
				remainingDeclarators(typespec);
			} 
			else 
			{
				openScope();
				returnType = Type(typespec, indirection);
				defineFunction(name, Type(typespec, indirection, parameters()));
				match(')');
				match('{');
				declarations();
				statements();
				closeScope();
				match('}');
			}
		}
		else 
		{
			declareVariable(name, Type(typespec, indirection));
			remainingDeclarators(typespec);
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
