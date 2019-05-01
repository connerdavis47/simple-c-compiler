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

static bool debug = false;

static string redefined = "redefinition of '%s'";
static string conflicting = "conflicting types for '%s'";
static string redeclared = "redeclaration of '%s'";
static string undeclared = "'%s' undeclared";
static string ptrRequired = "pointer type required for '%s'";
static string incomplete = "'%s' has incomplete type";

static const Type error;

static Scope* topLevel;
static Scope* outermost;

Scope* openScope()
{
  if (debug) cout << "openScope() ..." << endl;
  topLevel = new Scope(topLevel);

  if (outermost == nullptr)
    outermost = topLevel;

  return topLevel;
}

Scope* closeScope()
{
  if (debug) cout << "closeScope() ..." << endl;
  Scope* prev = topLevel;

  topLevel = topLevel->enclosing();

  return prev;
}

Symbol* defineFunction(const std::string& name, const Type& type)
{
  if (debug) cout << "defineFunction -> " << name << ":" << type << endl;
  Symbol* symbol = outermost->find(name);

  if (symbol == nullptr)
    symbol = declareFunction(name, type);

  if (symbol->_defined)
    report(redefined, name);

  symbol->_defined = true;
  return symbol;
}

Symbol* declareFunction(const std::string& name, const Type& type)
{
  if (debug) cout << "declareFunction -> " << name << ":" << type << endl;
  
  Symbol* symbol = outermost->find(name);
  if (symbol == nullptr)
  {
    symbol = new Symbol(name, type);
    outermost->insert(symbol);
  }
  else if (type != symbol->type())
  {
    report(conflicting, name);
    delete type.parameters();
  }
  else
    delete type.parameters();

  return symbol;
}

Symbol* declareVariable(const std::string& name, const Type& type)
{
  if (debug) cout << "declareVariable -> " << name << ":" << type << endl;

  Symbol* symbol = topLevel->find(name);
  if (symbol == nullptr)
  {
    symbol = new Symbol(name, type);
    topLevel->insert(symbol);
  }
  else if (outermost != topLevel)
    report(redeclared, name);
  else if (type != symbol->type())
    report(conflicting, name);

  return symbol;
}

Symbol* checkIdentifier(const std::string& name)
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

Symbol* checkFunction(const std::string& name)
{
  Symbol* symbol = topLevel->lookup(name);

  if (symbol == nullptr)
  {
    report(undeclared, name);
    symbol = new Symbol(name, error);
    topLevel->insert(symbol);
  }

  return symbol;
}
