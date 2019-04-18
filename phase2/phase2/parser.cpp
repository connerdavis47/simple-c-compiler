/*
 * Author:  Conner Davis
 * File:	  parser.h
 */

# include <cctype>
# include <iostream>

# include "lexer.h"
# include "tokens.h"
# include "parser.h"

using std::cout;
using std::endl;
using std::string;
using std::to_string;

/**
 * The next token's type, whose possible values are found in 
 * tokens.h.
 * 
 * When the input has been fully read, the token is "DONE".
 */
static int lookahead;
/**
 * The next token's actual value. For example, tokens of type
 * STRING will have a value "contained within the quotes".
 */
static string buffer;

// (START) - Tooling

static void error( string msg )
{
  report("Syntax error at token: %s", buffer);
  report("... %s", msg);
  exit(EXIT_FAILURE);
}

static bool is_specifier( )
{
  return lookahead == INT || lookahead == LONG || lookahead == STRUCT;
}

static void match( int token )
{
  if (lookahead == token)
    lookahead = lexan(buffer);
  else
    error(
      string("token mismatch - expected <") + to_string(token) + "> found <" + to_string(lookahead) + ">"
    );
}

static int match_specifier( )
{
  int spec = lookahead;

  if (is_specifier())
  {
    match(lookahead);

    /* 
      STRUCTS are of special format to other specifiers,
      struct <id> <pointers> <id> ;
     */
    if (spec == STRUCT)
      match(ID);

    return spec;
  }
  
  return -1;
}

static void print( string output )
{
  cout << output << endl;
}

// (END) - Tooling

// (START) - Expressions

static void expression( )
{
  logical_cmp_expression();

  // expression || expression
  while (lookahead == OR)
  {
    match(OR);
    logical_cmp_expression();
    print("or");
  }
}

static void logical_cmp_expression( )
{
  equality_expression();

  // expression && expression
  while (lookahead == AND)
  {
    match(AND);
    equality_expression();
    print("and");
  }
}

static void equality_expression( )
{
  relation_expression();

  while (true)
  {
    // expression == expression
    if (lookahead == EQL)
    {
      match(EQL);
      relation_expression();
      print("eql");
    }
    // expression != expression
    else if (lookahead == NEQ)
    {
      match(NEQ);
      relation_expression();
      print("neq");
    }
    else break;
  }
}

static void relation_expression( )
{
  add_expression();

  while (true)
  {
    // expression <= expression
    if (lookahead == LEQ)
    {
      match(LEQ);
      add_expression();
      print("leq");
    }
    // expression >= expression
    else if (lookahead == GEQ)
    {
      match(GEQ);
      add_expression();
      print("geq");
    }
    // expression < expression
    else if (lookahead == '<')
    {
      match('<');
      add_expression();
      print("ltn");
    }
    // expression > expression
    else if (lookahead == '>')
    {
      match('>');
      add_expression();
      print("gtn");
    }
    else break;
  }
}

static void add_expression( )
{
  multiply_expression();

  while (true)
  {
    // expression + expression
    if (lookahead == '+')
    {
      match('+');
      multiply_expression();
      print("add");
    }
    // expression - expression
    else if (lookahead == '-')
    {
      match('-');
      multiply_expression();
      print("sub");
    }
    else break;
  }
}

static void multiply_expression( )
{
  prefix_expression();

  while (true)
  {
    // expression * expression
    if (lookahead == '*')
    {
      match('*');
      prefix_expression();
      print("mul");
    }
    // expression / expression
    else if (lookahead == '/')
    {
      match('/');
      prefix_expression();
      print("div");
    }
    // expression % expression
    else if (lookahead == '%')
    {
      match('%');
      prefix_expression();
      print("rem");
    }
    else break;
  }
}

static void prefix_expression( )
{
  // ! expression
  if (lookahead == '!')
  {
    match('!');
    prefix_expression();
    print("not");
  }
  // | - expression
  else if (lookahead == '-')
  {
    match('-');
    prefix_expression();
    print("neg");
  }
  // | & expression
  else if (lookahead == '&')
  {
    match('&');
    prefix_expression();
    print("addr");
  }
  // | * expression
  else if (lookahead == '*')
  {
    match('*');
    prefix_expression();
    print("deref");
  }
  // | sizeof ( expression )
  else if (lookahead == SIZEOF)
  {
    match(SIZEOF);
    match('(');
    prefix_expression();
    match(')');
    print("sizeof");
  }
  else 
    post_expression();
}

static void post_expression( )
{
  cast_expression();

  while (true)
  {
    // expression [ expression ]
    if (lookahead == '[')
    {
      match('[');
      expression();
      match(']');
      print("index");
    }
    // | expression . id
    else if (lookahead == '.')
    {
      match('.');
      match(ID);
      print("dot");
    }
    // | expression -> id
    else if (lookahead == ARROW)
    {
      match(ARROW);
      match(ID);
      print("arrow");
    }
    else break;
  }
}

static void cast_expression( )
{
  general_expression();

  if (lookahead == '(')
  {
    match('(');

    // | ( specifier pointers ) expression
    if (is_specifier())
    {
      match_specifier();
      pointers();
      match(')');
      expression();
      print("cast");
    }
    // ( expression )
    else
    {
      expression();
      match(')');
    }
  }
}

static void general_expression( )
{
  // num
  if (lookahead == NUM)
    match(NUM);
  // | id
  else if (lookahead == ID)
  {
    match(ID);
    
    // | id ( )
    if (lookahead == '(')
    {
      match('(');
      
      // | id ( argument-list )
      if (lookahead != ')')
        arguments();

      match(')');
    }
  }
}

// (END) - Expressions

// (START) - Functions

static void arguments( )
{
  // argument
  argument();

  // | argument , argument-list
  while (lookahead == ',')
  {
    match(',');
    argument();
  }
}

static void argument( )
{
  // string
  if (lookahead == STRING)
    match(STRING);
  // | expression
  else
    expression();
}

static void parameters( )
{
  // empty
  if (lookahead == ')')
    return;
  // | void
  else if (lookahead == VOID)
    match(VOID);
  // | parameter-list
  else
  {
    parameter();

    while (lookahead == ',')
    {
      match(',');
      parameter();
    }
  }
}

static void parameter( )
{
  // specifier pointers id
  match_specifier();
  pointers();
  match(ID);
}

static void pointers( )
{
  // empty

  // | * pointers
  while (lookahead == '*')
    match('*');
}

static void statements( )
{
  // empty

  // | statements (which end when the surrounding block ends with })
  while (lookahead != '}')
    statement();
}

static void statement( )
{
  // { declarations statements }
  if (lookahead == '{')
  {
    match('{');
    declarations();
    statements();
    match('}');
  }
  // | return expression ;
  else if (lookahead == RETURN)
  {
    match(RETURN);
    expression();
    match(';');
  }
  // | while ( expression ) statement
  else if (lookahead == WHILE)
  {
    match(WHILE);
    match('(');
    expression();
    match(')');
    statement();
  }
  // | if ( expression ) statement
  else if (lookahead == IF)
  {
    match(IF);
    match('(');
    expression();
    match(')');
    statement();

    // | if ( expression ) statement else statement 
    if (lookahead == ELSE)
    {
      match(ELSE);
      statement();
    }
  }
  else
  {
    // | expression ;
    expression();

    // | expression = expression ;
    if (lookahead == '=')
    {
      match('=');
      expression();
    }

    match(';');
  }
}

static void declarations( )
{
  // empty

  // | declarations (each starts with specifier)
  while (is_specifier())
    declaration();
}

static void declaration( )
{
  // specifier declarator-list ;
  match_specifier();
  declarators();
  match(';');
}

static void declarators( )
{
  // declarator
  declarator();

  // | declarator , declarator-list
  while (lookahead == ',')
  {
    match(',');
    declarator();
  }
}

static void declarator( )
{
  // pointers id
  pointers();
  match(ID);

  // | pointers id [ num ]
  if (lookahead == '[')
  {
    match('[');
    match(NUM);
    match(']');
  }
}

// (END) - Functions

// (START) - Translation unit

static void global_declaration( )
{
  // specifier global-declarator-list ;
  // note ; handled within global_declarators()
  match_specifier();
  global_declarators();
}

static void global_declarators( )
{
  // global-declarator
  const bool func = global_declarator();

  // | global-declarator , global-declarator-list
  while (lookahead == ',')
  {
    match(',');
    global_declarator();
  }

  // global decls (not function defs) end with ;
  if (!func)
    match(';');
}

static bool global_declarator( )
{
  // pointers id
  pointers();
  match(ID);

  // | pointers id ( ) 
  if (lookahead == '(')
  {
    match('(');
    
    // | pointers id ( parameters )
    if (lookahead != ')')
      parameters();

    match(')');

    // | pointers id ( parameters ) { declarations statements }
    if (lookahead == '{')
    {
      match('{');
      declarations();
      statements();
      match('}');
      
      return true; // indicate function decl, which does not end with ;
    }
  }
  // | pointers id [ num ]
  else if (lookahead == '[')
  {
    match('[');
    match(NUM);
    match(']');
  }

  return false; // indicate global decl, which ends with ;
}

static void translation_unit( )
{
  // type-definition --> struct id { declarations } ;
  if (lookahead == STRUCT)
  {
    match(STRUCT);
    match(ID);

    // optional: { declarations }
    if (lookahead == '{')
    {
      match('{');
      declarations();
      match('}');
    }

    match(';');
  }
  // global-declaration | function-definition
  // --> left-factored: specifier pointers [...]
  else
    global_declaration();
}

// (END) - Translation unit

int main( void )
{
  // read the first token
  lookahead = lexan(buffer);

  // read tokens until end of input stream
  while (lookahead != DONE && lookahead != ERROR)
    translation_unit();

  return EXIT_SUCCESS;
}
