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

static void error( string src, string msg )
{
  report("Syntax error at token [ %s ]", buffer);
  report("Error source --> <%s()> ]", src);
  report("Error message --> %s", msg);
  exit(EXIT_FAILURE);
}

static void match( int token )
{
  if (lookahead == token)
    lookahead = lexan(buffer);
  else
    error("match", 
      string("token mismatch - expected <") 
      + to_string(token) + "> found <" + to_string(lookahead) + ">"
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
  else
  {
    error("match_specifier", "expected specifier but there was none");

    return -1;
  }
}

static void print( string output )
{
  cout << output << endl;
}

static bool is_specifier( )
{
  return lookahead == INT || lookahead == LONG || lookahead == STRUCT;
}

// (END) - Tooling

// (START) - Expressions

/*
 expression                  :=  logical_compare_expression
                                | expression || logical_compare_expression
 */
static void expression( )
{
  logical_cmp_expression();

  while (lookahead == OR)
  {
    match(OR);
    logical_cmp_expression();
    print("or");
  }
}

/*
  logical_compare_expression  :=  equality_expression
                                | logical_compare_expression && equality_expression
 */
static void logical_cmp_expression( )
{
  equality_expression();

  while (lookahead == AND)
  {
    match(AND);
    equality_expression();
    print("and");
  }
}

/*
  equality_expression         :=  relational_expression
                                | equality_expression == relational_expression
                                | equality_expression != relational_expression
 */
static void equality_expression( )
{
  relation_expression();

  while (true)
  {
    if (lookahead == EQL)
    {
      match(EQL);
      relation_expression();
      print("eql");
    }
    else if (lookahead == NEQ)
    {
      match(NEQ);
      relation_expression();
      print("neq");
    }
    else break;
  }
}

/*
  relational_expression       :=  additive_expression
                                | relational_expression <= additive_expression
                                | relational_expression >= additive_expression
                                | relational_expression < additive_expression
                                | relational_expression > additive_expression
 */
static void relation_expression( )
{
  add_expression();

  while (true)
  {
    if (lookahead == LEQ)
    {
      match(LEQ);
      add_expression();
      print("leq");
    }
    else if (lookahead == GEQ)
    {
      match(GEQ);
      add_expression();
      print("geq");
    }
    else if (lookahead == '<')
    {
      match('<');
      add_expression();
      print("ltn");
    }
    else if (lookahead == '>')
    {
      match('>');
      add_expression();
      print("gtn");
    }
    else break;
  }
}

/*
  additive_expression         :=  multiplicative_expression
                                | additive_expression + multiplicative_expression
                                | additive_expression - multiplicative_expression
 */
static void add_expression( )
{
  multiply_expression();

  while (true)
  {
    if (lookahead == '+')
    {
      match('+');
      multiply_expression();
      print("add");
    }
    else if (lookahead == '-')
    {
      match('-');
      multiply_expression();
      print("sub");
    }
    else break;
  }
}

/*
  multiplicative_expression   :=  prefix_expression
                                | multiplicative_expression * prefix_expression
                                | multiplicative_expression / prefix_expression
                                | multiplicative_expression % prefix_expression
 */
static void multiply_expression( )
{
  prefix_expression();

  while (true)
  {
    if (lookahead == '*')
    {
      match('*');
      prefix_expression();
      print("mul");
    }
    else if (lookahead == '/')
    {
      match('/');
      prefix_expression();
      print("div");
    }
    else if (lookahead == '%')
    {
      match('%');
      prefix_expression();
      print("rem");
    }
    else break;
  }
}

/*
  prefix_expression           :=  post_expression
                                | ! prefix_expression
                                | - prefix_expression
                                | & prefix_expression
                                | * prefix_expression
                                | sizeof ( prefix_expression )
 */
static void prefix_expression( )
{
  if (lookahead == '!')
  {
    match('!');
    prefix_expression();
    print("not");
  }
  else if (lookahead == '-')
  {
    match('-');
    prefix_expression();
    print("neg");
  }
  else if (lookahead == '&')
  {
    match('&');
    prefix_expression();
    print("addr");
  }
  else if (lookahead == '*')
  {
    match('*');
    prefix_expression();
    print("deref");
  }
  else if (lookahead == SIZEOF)
  {
    match(SIZEOF);

    if (lookahead == '(')
    {
      match('(');
      prefix_expression();
      match(')');
    }

    print("sizeof");
  }
  else 
    post_expression();
}

/*
  post_expression             :=  cast_expression
                                | post_expression [ expression ]
                                | post_expression . id
                                | post_expression -> id
 */
static void post_expression( )
{
  cast_expression();

  while (true)
  {
    if (lookahead == '[')
    {
      match('[');
      expression();
      match(']');
      print("index");
    }
    else if (lookahead == '.')
    {
      match('.');
      match(ID);
      print("dot");
    }
    else if (lookahead == ARROW)
    {
      match(ARROW);
      match(ID);
      print("arrow");
    }
    else break;
  }
}

/*
  cast_expression             :=  general_expression
                                | ( specifier pointers ) expression
                                | ( expression )
 */
static void cast_expression( )
{
  general_expression();

  if (lookahead == '(')
  {
    match('(');

    if (is_specifier())
    {
      match_specifier();
      pointers();
      match(')');
      expression();
      print("cast");
    }
    else
    {
      expression();
      match(')');
    }
  }
}

/*
  general_expression          :=  id ( argument_list )
                                | id ( )
                                | id
                                | num
 */
static void general_expression( )
{
  if (lookahead == NUM)
    match(NUM);
  else if (lookahead == ID)
  {
    match(ID);
    
    if (lookahead == '(')
    {
      match('(');
      
      if (lookahead != ')')
        arguments();

      match(')');
    }
  }
}

// (END) - Expressions

// (START) - Functions

/*
  argument_list               :=  argument
                                | argument , argument_list
 */
static void arguments( )
{
  argument();

  while (lookahead == ',')
  {
    match(',');
    argument();
  }
}

/*
  argument                    :=  string
                                | expression
 */
static void argument( )
{
  if (lookahead == STRING)
    match(STRING);
  else
    expression();
}

/*
  parameters                  :=  void
                                | parameter_list
 */
static void parameters( )
{
  if (lookahead == ')')
    return;
  else if (lookahead == VOID)
    match(VOID);
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

/*
  parameter                   :=  specifier pointers id
 */
static void parameter( )
{
  match_specifier();
  pointers();
  match(ID);
}

/*
  pointers                    :=  empty
                                | * pointers
 */
static void pointers( )
{
  while (lookahead == '*')
    match('*');
}

// (END) - Functions

// (START) - Translation unit

/*
  statements                  :=  empty
                                | statement statements
 */
static void statements( )
{
  while (lookahead != '}')
    statement();
}

/*
  statement                   :=  { declarations statements }
                                | return expression ;
                                | while ( expression ) statement
                                | if ( expression ) statement
                                | if ( expression ) statement else statement
                                | expression = expression;
                                | expression ;
 */
static void statement( )
{
  if (lookahead == '{')
  {
    match('{');
    declarations();
    statements();
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
  declarations                :=  empty
                                | declaration declarations
 */
static void declarations( )
{
  while (is_specifier())
    declaration();
}

/*
  declaration                 :=  specifier declarator_list ;
 */
static void declaration( )
{
  int spec = match_specifier();
  declarator(spec);

  while (lookahead == ',')
  {
    match(',');
    declarator(spec);
  }

  match(';');
}

/*
  declarator_list             :=  declarator
                                | declarator , declarator_list
 */
static void declarators( )
{
  if (lookahead == ';')
    match(';');
  else
  {
    while (lookahead == ',')
    {
      match(',');
      global_declarator();
    }

    match(';');
  }
}

/*
  declarator                  :=  pointers id
                                | pointers id [ num ]
 */
static void declarator( int spec )
{
  pointers();
  match(ID);

  if (lookahead == '[')
  {
    match('[');
    match(NUM);
    match(']');
  }
}

/*
  global_declarator           :=  pointers id
                                | pointers id ( )
                                | pointers id [ num ]
 */
static void global_declarator( )
{
  pointers();
  match(ID);

  if (lookahead == '(')
  {
    match('(');
    
    if (lookahead != ')')
      arguments();

    match(')');
  }
  else if (lookahead == '[')
  {
    match('[');
    match(NUM);
    match(']');
  }
}

/*
  translation_unit            :=  empty
                                | struct id { declaration declarations } ; translation_unit
                                | specifier global_declarator_list ;
                                | specifier global_declarator ( parameters ) { declarations statements }
 */
static void translation_unit( )
{
  if (lookahead == STRUCT)
  {
    match(STRUCT);
    match(ID);

    if (lookahead == '{')
    {
      match('{');
      declarations();
      match('}');
      match(';');
    }
  }
  else
  {
    match_specifier();
    pointers();
    match(ID);

    if (lookahead == '(')
    {
      match('(');
      parameters();
      match(')');

      if (lookahead == '{')
      {
        match('{');
        declarations();
        statements();
        match('}');
      }
      else 
        declarators();
    }
    else if (lookahead == '[')
    {
      match('[');
      match(NUM);
      match(']');
      declarators();
    }
  }
}

// (END) - Translation unit

int main( void )
{
  // read the first token
  lookahead = lexan(buffer);

  // read tokens until end of input stream
  while (lookahead != DONE)
    translation_unit();

  return EXIT_SUCCESS;
}
