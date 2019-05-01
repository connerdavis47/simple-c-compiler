# include <cassert>
# include <iostream>

# include "checker.h"
# include "lexer.h" // note we only use this for report(..)
# include "tokens.h"
# include "Scope.h"
# include "Symbol.h"
# include "Type.h"

using std::cout;
using std::endl;
using std::string;

/*
  Enables "debug mode", which prints out each scope change,
  declaration, and definition.
 */
static bool debug = true;

/*
  The semantic errors this checker is capable of finding.
 */
static string redefined = "redefinition of '%s'";
static string conflicting = "conflicting types for '%s'";
static string redeclared = "redeclaration of '%s'";
static string undeclared = "'%s' undeclared";
static string ptrRequired = "pointer type required for '%s'";
static string incomplete = "'%s' has incomplete type";

/*
  A constant instance of the error type, which is defined
  in Type.h by the empty constructor.
 */
static const Type error;

/*
  The top-level scope, or in other words, the global scope.
 */
static Scope* topLevel;
/*
  The (deepest) scope we are currently in, or the active
  scope.
 */
static Scope* activeScope;

Scope* openScope( )
{
  if (debug) 
    cout << "openScope() ..." << endl;

  topLevel = new Scope(topLevel);

  if (activeScope == nullptr)
    activeScope = topLevel;

  return topLevel;
}

Scope* closeScope( )
{
  if (debug) 
    cout << "closeScope() ..." << endl;

  Scope* prev = topLevel;
  topLevel = topLevel->enclosing();

  return prev;
}

Symbol* defineFunction( const string& name, const Type& type )
{
  if (debug) 
    cout << "defineFunction -> " << name << ":" << type << endl;

  Symbol* symbol = activeScope->find(name);

  if (symbol == nullptr)
    symbol = declareFunction(name, type);

  else if (symbol->_defined)
    report(redefined, name);

  symbol->_defined = true;
  return symbol;
}

Symbol* defineStruct( const string& name )
{
  if (debug)
    cout << "defineStruct -> " << name << endl;

  Symbol* symbol = activeScope->find(name);

  if (symbol == nullptr)
  {
    symbol = new Symbol(name, Type(STRUCT));
    activeScope->insert(symbol);
  }

  /* The structure must not have been previously defined [E1]. */
  else if (symbol->_defined)
    report(redefined, name);

  symbol->_defined = true;
  return symbol;
}

Symbol* declareStruct( const string& name, const Type& type, const string& structName )
{
  assert(type.specifier() == STRUCT);
  if (debug) 
    cout << "declareStruct -> " << name << ":" << structName << endl;

  Symbol* symbol = activeScope->find(name);

  /* Additionally, if specifier is a structure type, then pointers must be non-empty [E5]. */
  if (type.indirection() == 0)
    report(ptrRequired, name);
    
  else if (symbol == nullptr)
  {
    symbol = new Symbol(name, type);
    activeScope->insert(symbol);
  }

  else
    report(redefined, name);

  return symbol;
}

Symbol* declareFunction( const string& name, const Type& type )
{
  if (debug) 
    cout << "declareFunction -> " << name << ":" << type << endl;

  /* Each variable is declared in the current scope. */
  Symbol* symbol = activeScope->find(name);

  if (symbol == nullptr)
  {
    symbol = new Symbol(name, type);
    activeScope->insert(symbol);
  }

  /* and any previous declaration must be identical [E2] */
  else if (type != symbol->type())
  {
    report(conflicting, name);

    /* ignoring any parameters */
    delete type.parameters();
  }

  /* The function must not have been previous defined [E1] */
  else
    report(redefined, name);

  return symbol;
}

Symbol* declareVariable( const string& name, const Type& type )
{
  if (debug) 
    cout << "declareVariable -> " << name << ":" << type << endl;

  Symbol* symbol = topLevel->find(name);

  /* 
    If specifier is a structure type, then the type must be complete or pointers 
    must be non-empty [E6].
   */
  if (activeScope != topLevel && type.specifier() == STRUCT && type.indirection() == 0)
    report(ptrRequired, name);

  if (symbol == nullptr)
  {
    symbol = new Symbol(name, type);
    topLevel->insert(symbol);
  }

  /*  
    If the variable is a local variable, then the variable must not be previously 
    declared in the current scope [E3]. 
   */
  else if (activeScope != topLevel)
    report(redeclared, name);

  /* 
    If the variable is a global variable, then any previous declaration must be 
    identical [E2]. 
   */
  else if (type != symbol->type())
    report(conflicting, name);

  return symbol;
}

Symbol* checkIdentifier( const string& name )
{
  Symbol* symbol = topLevel->lookup(name);

  if (symbol == nullptr)
  {
    report(undeclared, name);
    symbol = new Symbol(name, error); // declare type ERROR
    topLevel->insert(symbol);
  }

  return symbol;
}

Symbol* checkStruct( const string& name, const string& structName )
{
  Symbol* symbol = topLevel->lookup(name);

  if (symbol == nullptr)
    report(incomplete, structName);

  return symbol;
}
