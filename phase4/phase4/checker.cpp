/*
 * File:	checker.cpp
 *
 * Description:	This file contains the public and private function and
 *		variable definitions for the semantic checker for Simple C.
 *
 *		Extra functionality:
 *		- inserting an undeclared symbol with the error type
 */

# include <map>
# include <iostream>
# include "lexer.h"
# include "checker.h"
# include "Symbol.h"
# include "Scope.h"
# include "Type.h"

using namespace std;

static const Type error;
static const Type integer("int");
static const Type longinteger("long");

static map<string,Scope *> fields;
static Scope *outermost, *toplevel;

static string undeclared = "'%s' undeclared";
static string redefined = "redefinition of '%s'";
static string redeclared = "redeclaration of '%s'";
static string conflicting = "conflicting types for '%s'";
static string incomplete = "'%s' has incomplete type";
static string nonpointer = "pointer type required for '%s'";

static string invalidReturn = "invalid return type";                    // E1
static string invalidType = "invalid type for test expression";         // E2
static string reqLvalue = "lvalue required in expression";              // E3
static string invalidBinary = "invalid operands to binary %s";          // E4
static string invalidUnary = "invalid operands to unary %s";            // E5
static string invalidCast = "invalid operand in cast expression";       // E6
static string invalidSizeof = "invalid operand in sizeof expression";   // E7
static string notFunc = "called object is not a function";              // E8
static string invalidArgs = "invalid arguments to called function";     // E9
static string incompletePtr = "using pointer to incomplete type";       // E10

/*
 * Function:	checkIfComplete
 *
 * Description:	Check if the given type is complete.  A non-structure type
 *		is always complete.  A structure type is complete if its
 *		fields have been defined.
 */

static 
Type checkIfComplete(const string& name, const Type& type)
{
  if (!type.isStruct() || type.indirection() > 0)
	  return type;

  if (fields.count(type.specifier()) > 0)
	  return type;

  report(incomplete, name);
  return error;
}


/*
 * Function:	checkIfStructure
 *
 * Description:	Check if the given type is a structure.
 */

static 
Type checkIfStructure(const string &name, const Type &type)
{
  if (!type.isStruct() || type.indirection() > 0)
	  return type;

  report(nonpointer, name);
  return type;
}

static 
bool isIncompletePointer(const Type& t)
{
  return t.isStruct() && t.indirection() == 1 && fields.count(t.specifier()) == 0;
}

static 
bool isLvalue(const Type& t)
{
  return t.isScalar();
}


/*
 * Function:	openScope
 *
 * Description:	Create a scope and make it the new top-level scope.
 */

Scope *openScope()
{
    toplevel = new Scope(toplevel);

    if (outermost == nullptr)
	outermost = toplevel;

    return toplevel;
}


/*
 * Function:	closeScope
 *
 * Description:	Remove the top-level scope, and make its enclosing scope
 *		the new top-level scope.
 */

Scope *closeScope()
{
    Scope *old = toplevel;

    toplevel = toplevel->enclosing();
    return old;
}


/*
 * Function:	openStruct
 *
 * Description:	Open a scope for a structure with the specified name.  If a
 *		structure with the same name is already defined, delete it.
 */

void openStruct(const string &name)
{
    if (fields.count(name) > 0) {
	delete fields[name];
	fields.erase(name);
	report(redefined, name);
    }

    openScope();
}


/*
 * Function:	closeStruct
 *
 * Description:	Close the scope for the structure with the specified name.
 */

void closeStruct(const string &name)
{
    fields[name] = closeScope();
}


/*
 * Function:	defineFunction
 *
 * Description:	Define a function with the specified NAME and TYPE.  A
 *		function is always defined in the outermost scope.  This
 *		definition always replaces any previous definition or
 *		declaration.
 */

Symbol *defineFunction(const string &name, const Type &type)
{
    Symbol *symbol = outermost->find(name);

    if (symbol != nullptr) {
	if (symbol->type().isFunction() && symbol->type().parameters()) {
	    report(redefined, name);
	    delete symbol->type().parameters();

	} else if (type != symbol->type())
	    report(conflicting, name);

	outermost->remove(name);
	delete symbol;
    }

    symbol = new Symbol(name, checkIfStructure(name, type));
    outermost->insert(symbol);

    return symbol;
}


/*
 * Function:	declareFunction
 *
 * Description:	Declare a function with the specified NAME and TYPE.  A
 *		function is always declared in the outermost scope.  Any
 *		redeclaration is discarded.
 */

Symbol *declareFunction(const string &name, const Type &type)
{
    Symbol *symbol = outermost->find(name);

    if (symbol == nullptr) {
	symbol = new Symbol(name, checkIfStructure(name, type));
	outermost->insert(symbol);

    } else if (type != symbol->type()) {
	report(conflicting, name);
	delete type.parameters();
    }

    return symbol;
}


/*
 * Function:	declareParameter
 *
 * Description:	Declare a parameter with the specified NAME and TYPE.  Any
 *		redeclaration is discarded.  The only difference between
 *		declaring a parameter and a variable is that a parameter
 *		cannot be a structure type.
 */

Symbol *declareParameter(const string &name, const Type &type)
{
    return declareVariable(name, checkIfStructure(name, type));
}


/*
 * Function:	declareVariable
 *
 * Description:	Declare a variable with the specified NAME and TYPE.  Any
 *		redeclaration is discarded.
 */

Symbol *declareVariable(const string &name, const Type &type)
{
    Symbol *symbol = toplevel->find(name);

    if (symbol == nullptr) {
	symbol = new Symbol(name, checkIfComplete(name, type));
	toplevel->insert(symbol);

    } else if (outermost != toplevel)
	report(redeclared, name);

    else if (type != symbol->type())
	report(conflicting, name);

    return symbol;
}


/*
 * Function:	checkIdentifier
 *
 * Description:	Check if NAME is declared.  If it is undeclared, then
 *		declare it as having the error type in order to eliminate
 *		future error messages.
 */

Symbol *checkIdentifier(const string &name)
{
    Symbol *symbol = toplevel->lookup(name);

    if (symbol == nullptr) {
	report(undeclared, name);
	symbol = new Symbol(name, error);
	toplevel->insert(symbol);
    }

    return symbol;
}


Type checkAssignment( const Type& left, const Type& right )
{
  const Type t1 = left;
  const Type t2 = right.promote();

  if (isLvalue(t1))
  {
    if (t1 == t2)
      return t1;

    report(reqLvalue);
    return error;
  }

  report(invalidBinary, "=");
  return error;
}


Type checkLogical(const Type& left, const Type& right, const string& op)
{
  // The types of the two operands need not be compatible.

  // The type of each operand must be a scalar type [E4].
  if (left.isScalar() && right.isScalar())
    return integer; /* The result has type int and is not an lvalue. */

  report(invalidBinary, op);
  return error;
}

Type checkLogicalAnd(const Type& left, const Type& right)
{
  return checkLogical(left, right, "&&");
}

Type checkLogicalOr(const Type& left, const Type& right)
{
  return checkLogical(left, right, "||");
}


Type checkEquality(const Type& left, const Type& right, const string& op)
{
  // The types of the left and right operands must be compatible [E4].
  if (left == right)
    return integer; /* The result has type int and is not an lvalue. */

  report(invalidBinary, op);
  return error;
}

Type checkEqual(const Type& left, const Type& right)
{
  return checkEquality(left, right, "==");
}

Type checkNotEqual(const Type& left, const Type& right)
{
  return checkEquality(left, right, "!=");
}


Type checkRelational(const Type& left, const Type& right, const string& op)
{
  // The types of the left and right operands must be compatible [E4].
  if (left == right)
    return integer; /* The result has type int and is not an lvalue. */

  report(invalidBinary, op);
  return error;
}

Type checkLessOrEqual(const Type& left, const Type& right)
{
  return checkRelational(left, right, "<=");
}

Type checkGreaterOrEqual(const Type& left, const Type& right)
{
  return checkRelational(left, right, ">=");
}

Type checkLessThan(const Type& left, const Type& right)
{
  return checkRelational(left, right, "<");
}

Type checkGreaterThan(const Type& left, const Type& right)
{
  return checkRelational(left, right, ">");
}


Type checkAdditive(const Type& left, const Type& right, const string& op)
{
  // In all cases, operands undergo type promotion and the result is never an lvalue.
  const Type t1 = left.promote();
  const Type t2 = right.promote();

  // If the types of both operands are numeric, then the result has type long if either operand has 
  // type long, and has type int otherwise.
  if (t1.isNumeric() && t2.isNumeric())
    return (t1 == longinteger || t2 == longinteger) ? longinteger : integer;

  // If the left operand has type “pointer to T” and the right operand has a numeric type, then the 
  // result has type “pointer to T.
  if (t1.isPointer() && t2.isNumeric())
  {
    /* Additionally, if any operand has type “pointer to T” then T must be complete [E10]. */
    if (isIncompletePointer(t1))
    {
      report(incompletePtr);
      return error;
    }

    return t1;
  }

  // For addition only, if the left operand has a numeric type and the right operand has type 
  // “pointer to T” then the result has type “pointer to T.”
  if (op == "+" && t1.isNumeric() && t2.isPointer())
  {
    /* Additionally, if any operand has type “pointer to T” then T must be complete [E10]. */
    if (isIncompletePointer(t2))
    {
      report(incompletePtr);
      return error;
    }

    return t2;
  }

  // For subtraction only, if both operands have type “pointer to T” then the result has type 
  // long.
  if (op == "-" && t1.isPointer() && t2.isPointer())
    return longinteger;

  // Otherwise, the result is an error [E4].
  report(invalidBinary, op);
  return error;
}

Type checkAdd(const Type& left, const Type& right)
{
  return checkAdditive(left, right, "+");
}

Type checkSubtract(const Type& left, const Type& right)
{
  return checkAdditive(left, right, "-");
}


Type checkMultiplicative(const Type& left, const Type& right, const string& op)
{
  // The types of both operands must be numeric [E4].
  // ...
  // If either operand has type long, then the result has type long. Otherwise, the result has 
  // type int. The result is never an lvalue.
  if (left.isNumeric() && right.isNumeric())
    return (left == longinteger || right == longinteger) ? longinteger : integer;

  report(invalidBinary, op);
  return error;
}

Type checkMultiply(const Type& left, const Type& right)
{
  return checkMultiplicative(left, right, "*");
}

Type checkDivide(const Type& left, const Type& right)
{
  return checkMultiplicative(left, right, "/");
}

Type checkRemainder(const Type& left, const Type& right)
{
  return checkMultiplicative(left, right, "%");
}


Type checkNegate(const Type& operand)
{
  // The operand in a unary - expression must have a numeric type [E5] and the result has the 
  // same type.
  if (operand.isNumeric())
    return operand;

  report(invalidUnary, "-");
  return error;
}

Type checkNot(const Type& operand)
{
  // The operand in a ! expression must have a scalar type [E5], and the result has type int.
  if (operand.isScalar())
    return integer;

  report(invalidUnary, "!");
  return error;
}

Type checkAddress(const Type& operand)
{
  // The operand in a unary & expression must be an lvalue [E3].
  // ...
  // If the operand has type T, then the result has type “pointer to T” and is not an lvalue. 
  if (isLvalue(operand))
    return Type(operand.specifier(), operand.indirection() + 1);

  report(reqLvalue, "&");
  return error;
}

Type checkDereference(const Type& operand)
{
  // The operand in a unary * expression must have type “pointer to T” after any promotion [E5],
  // and T must be complete [E10]. The result has type T and is an lvalue.

  Type t = operand.promote();

  if (t.isPointer())
  {
    if (!isIncompletePointer(t))
      return Type(operand.specifier(), operand.indirection() - 1);

    report(incompletePtr, "&");
    return error;
  }

  report(invalidUnary, "&");
  return error;
}

Type checkSizeof(const Type& operand)
{
  // The operand of a sizeof expression must not have a function type [E7]. 
  // The result of the expression has type long. In none of these cases is the result an lvalue.

  if (!operand.isFunction())
    return longinteger;

  report(invalidSizeof);
}


Type checkArray(const Type& left, const Type& right)
{
  // The left operand in an array reference expression must have type “pointer to T” after any 
  // promotion, the expression must have a numeric type [E4], and T must be complete [E10]. 
  // The result has type T and is an lvalue

  Type t1 = left.promote();

  if (t1.isPointer() && right.isNumeric())
  {
    if (!isIncompletePointer(t1))
      return Type(t1.specifier(), t1.indirection() - 1);     

    report(incompletePtr);
    return error;
  }

  report(invalidBinary, "[]");
  return error;
}

Type checkStructField(const Type& left, const Type& right)
{
  // The operand in a direct structure field reference must be a structure type and the 
  // identifier must be a field of the structure [E4], in which case the type of the 
  // expression is the type of the identifier. The result is an lvalue if the expression 
  // is an lvalue and if the type of the identifier is not an array type.

  return error;
}

Type checkStructPointerField(const Type& left, const Type& right)
{
  // The operand in an indirect structure field reference must be a pointer to a structure 
  // type (after any promotion), the structure type must be complete [E10], and the 
  // identifier must be a field of the structure [E4], in which case the type of the 
  // expression is the type of the identifier. The result is an lvalue if the type of the 
  // expression is not an array type.
}
