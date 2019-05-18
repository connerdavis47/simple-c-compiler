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

using std::cout;
using std::endl;
using std::map;
using std::string;

static map<string, Scope*> fields;
static Scope* outermost;
static Scope* toplevel;

static const Type error;
static const Type integer("int");
static const Type longinteger("long");

static string undeclared = "'%s' undeclared";
static string redefined = "redefinition of '%s'";
static string redeclared = "redeclaration of '%s'";
static string conflicting = "conflicting types for '%s'";
static string incomplete = "'%s' has incomplete type";
static string nonpointer = "pointer type required for '%s'";

static string invalidReturn = "invalid return type";                    // E1
static string invalidTest = "invalid type for test expression";         // E2
static string lvalueRequired = "lvalue required in expression";         // E3
static string invalidBinary = "invalid operands to binary %s";          // E4
static string invalidUnary = "invalid operand to unary %s";             // E5
static string invalidCast = "invalid operand in cast expression";       // E6
static string invalidSizeof = "invalid operand in sizeof expression";   // E7
static string funcRequired = "called object is not a function";         // E8
static string invalidArgs = "invalid arguments to called function";     // E9
static string ptrIncomplete = "using pointer to incomplete type";       // E10

/*
 * Function:	checkIfComplete
 *
 * Description:	Check if the given type is complete.  A non-structure type
 *		is always complete.  A structure type is complete if its
 *		fields have been defined.
 */

static Type checkIfComplete( const string& name, const Type& type )
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

static Type checkIfStructure( const string& name, const Type& type )
{
    if (!type.isStruct() || type.indirection() > 0)
	    return type;

    report(nonpointer, name);
    return type;
}


/*
 * Function:	openScope
 *
 * Description:	Create a scope and make it the new top-level scope.
 */

Scope* openScope()
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

Scope* closeScope()
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

void openStruct( const string& name )
{
    if (fields.count(name) > 0) 
    {
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

void closeStruct( const string& name )
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

Symbol *defineFunction( const string& name, const Type& type )
{
    Symbol *symbol = outermost->find(name);

    if (symbol != nullptr) 
    {
        if (symbol->type().isFunction() && symbol->type().parameters()) 
        {
            report(redefined, name);
            delete symbol->type().parameters();

        } 
        else if (type != symbol->type())
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

Symbol *declareFunction( const string& name, const Type& type )
{
    Symbol *symbol = outermost->find(name);

    if (symbol == nullptr) 
    {
	    symbol = new Symbol(name, checkIfStructure(name, type));
	    outermost->insert(symbol);
    } 
    else if (type != symbol->type()) 
    {
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

Symbol *declareParameter( const string& name, const Type& type )
{
    return declareVariable(name, checkIfStructure(name, type));
}


/*
 * Function:	declareVariable
 *
 * Description:	Declare a variable with the specified NAME and TYPE.  Any
 *		redeclaration is discarded.
 */

Symbol *declareVariable( const string& name, const Type& type )
{
    Symbol *symbol = toplevel->find(name);

    if (symbol == nullptr) 
    {
        symbol = new Symbol(name, checkIfComplete(name, type));
        toplevel->insert(symbol);
    } 
    else if (outermost != toplevel)
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

Symbol *checkIdentifier( const string& name )
{
    Symbol *symbol = toplevel->lookup(name);

    if (symbol == nullptr) 
    {
        report(undeclared, name);
        symbol = new Symbol(name, error);
        toplevel->insert(symbol);
    }

    return symbol;
}


static bool isIncompletePointer( const Type& t )
{
  return t.isStruct() && t.indirection() == 1 && fields.count(t.specifier()) == 0;
}


static bool coerceIntToLong( const Type& t1, const Type& t2)
{
    return t1.isNumeric() && t2.isNumeric()
        && (t1.specifier() == "long" || t2.specifier() == "long");
}


Type checkReturn( const Type& expr, const Type& type ) 
{ 
    if (expr.isError() || type.isError())
        return error;

    if (expr == type)
        return expr;

    report(invalidReturn);
    return error;
}
Type checkTest( const Type& expr )
{
    if (expr.isError())
        return error;

    if (expr.isScalar())
        return expr;

    report(invalidTest);
    return error;
}
Type checkAssignment( const Type& left, const Type& right, const bool& lvalue ) 
{ 
    if (left.isError() || right.isError())
        return error;

    if (lvalue)
    {
        if (left == right)
            return left;

        else if (coerceIntToLong(left, right))
            return longinteger;

        report(invalidBinary, "=");
        return error;
    }

    report(lvalueRequired);
    return error;
}


Type checkLogical( const Type& left, const Type& right, const std::string& op ) 
{ 
    if (left.isError() || right.isError())
        return error;
        
    if (left.isScalar() && right.isScalar())
        return integer;

    report(invalidBinary, op);
    return error;
}
Type checkLogicalAnd( const Type& left, const Type& right ) 
{
    return checkLogical(left, right, "&&");
}
Type checkLogicalOr( const Type& left, const Type& right ) 
{ 
    return checkLogical(left, right, "||");
}


Type checkEquality( const Type& left, const Type& right, const std::string& op ) 
{ 
    if (left.isError() || right.isError())
        return error;
        
    if (left == right)
        return integer;

    report(invalidBinary, op);
    return error;
}
Type checkEqual( const Type& left, const Type& right ) 
{ 
    return checkEquality(left, right, "==");
}
Type checkNotEqual( const Type& left, const Type& right ) 
{ 
    return checkEquality(left, right, "!=");
}


Type checkRelational( const Type& left, const Type& right, const std::string& op ) 
{ 
    if (left.isError() || right.isError())
        return error;
        
    if (left == right)
        return integer;

    report(invalidBinary, op);
    return error;
}
Type checkLessOrEqual( const Type& left, const Type& right ) 
{ 
    return checkRelational(left, right, "<=");
}
Type checkGreaterOrEqual( const Type& left, const Type& right ) 
{ 
    return checkRelational(left, right, ">=");
}
Type checkLessThan( const Type& left, const Type& right ) 
{ 
    return checkRelational(left, right, "<");
}
Type checkGreaterThan( const Type& left, const Type& right ) 
{ 
    return checkRelational(left, right, ">");
}


Type checkAdditive( const Type& left, const Type& right, const std::string& op ) 
{ 
    if (left.isError() || right.isError())
        return error;
        
    const Type t1 = left.promote();
    const Type t2 = right.promote();

    if (t1.isNumeric() && t2.isNumeric())
        return (t1 == longinteger || t2 == longinteger) ? longinteger : integer;

    if (t1.isPointer() && t2.isNumeric())
    {
        if (isIncompletePointer(t1))
        {
            report(ptrIncomplete);
            return error;
        }

        return t1;
    }

    if (op == "+" && t1.isNumeric() && t2.isPointer())
    {
        if (isIncompletePointer(t2))
        {
            report(ptrIncomplete);
            return error;
        }

        return t2;
    }

    if (op == "-" && t1.isPointer() && t2.isPointer())
        return longinteger;

    report(invalidBinary, op);
    return error;
}
Type checkAdd( const Type& left, const Type& right ) 
{ 
    return checkAdditive(left, right, "+");
}
Type checkSubtract( const Type& left, const Type& right ) 
{ 
    return checkAdditive(left, right, "-");
}


Type checkMultiplicative( const Type& left, const Type& right, const std::string& op ) 
{ 
    return left;
}
Type checkMultiply( const Type& left, const Type& right ) 
{ 
    return left;
}
Type checkDivide( const Type& left, const Type& right ) 
{ 
    return left;
}
Type checkRemainder( const Type& left, const Type& right ) 
{ 
    return left;
}


Type checkNegate( const Type& expr ) 
{ 
    if (expr.isError())
        return error;
        
    if (expr.isNumeric())
        return expr;

    report(invalidUnary, "-");
    return error;
}
Type checkNot( const Type& expr ) 
{ 
    if (expr.isScalar())
        return integer;

    report(invalidUnary, "!");
    return error;
}
Type checkAddress( const Type& expr, const bool& lvalue ) 
{
    if (expr.isError())
        return error;

    if (lvalue)
        return Type(expr.specifier(), expr.indirection() + 1);

    report(lvalueRequired);
    return error;
}
Type checkDereference( const Type& expr )
{
    if (expr.isError())
        return error;

    const Type t1 = expr.promote();

    if (t1.isPointer())
    {
        if (!isIncompletePointer(t1))
            return Type(t1.specifier());

        report(ptrIncomplete);
        return error;
    }

    report(invalidUnary, "*");
    return error;
}
Type checkSizeof( const Type& expr ) 
{ 
    if (expr.isError())
        return error;

    if (!expr.isFunction())
        return longinteger;

    report(invalidSizeof);
    return error;
}
Type checkTypeCast( const Type& left, const Type& right )
{
    if (left.isError() || right.isError())
        return error;

    const Type t1 = left.promote();
    const Type t2 = right.promote();

    if ((t1.isNumeric() && t2.isNumeric()) || (t1.isPointer() && t2.isPointer()))
        return t1;

    report(invalidCast);
    return error;
}


Type checkArray( const Type& left, const Type& right ) 
{ 
    if (left.isError() || right.isError())
        return error;

    const Type t1 = left.promote();

    if (t1.isPointer())
    {
        if (right.isNumeric())
        {
            if (!isIncompletePointer(t1))
                return Type(t1.specifier(), t1.indirection() - 1);

            report(ptrIncomplete);
            return error;
        }

        report(invalidBinary, "[]");
        return error;
    }

    report(invalidBinary, "[]");
    return error;
}
Type checkStructField( const Type& left, const Type& right ) 
{ 
    if (left.isError() || right.isError())
        return error;

    return left;
}
Type checkStructPointerField( const Type& left, const Type& right ) 
{ 
    if (left.isError() || right.isError())
        return error;

    return left;
}


Type checkFunction( const string& name, Parameters& args )
{
    // In this function I reverse my rule on the rest of the check...() functions where
    // I like to do only the "affirmative" logic and then return errors in the rest of
    // the conditions. I did it here because I realized this function is really
    // confusing and heavily nested otherwise. And because I didn't like the idea of
    // doing some affirmative and some negative logic in the same function, I committed
    // to the exact opposite of what you see in the rest of the check..() functions.
    //      D E A L     W I T H     I T

    Symbol* symbol = toplevel->lookup(name); 
    if (symbol == nullptr)
    {
        report(funcRequired);
        return error;
    }

    const Type t1 = symbol->type();
    if (!t1.isFunction())
    {
        report(funcRequired);
        return error;
    }

    for (unsigned i = 0; i < args.size(); ++i)
        args[i].promote();

    const Parameters* params = t1.parameters();
    if (params != nullptr)
    {
        if (params->size() != args.size())
        {
            report(invalidArgs);
            return error;
        }

        for (unsigned i = 0; i < args.size(); ++i)
        {
            const Type t1 = (*params)[i].promote();
            const Type t2 = args[i].promote();

            if (t1 != t2 && !coerceIntToLong(t1, t2))
            {
                report(invalidArgs);
                return error;
            }
        }
    }

    return Type(t1.specifier(), t1.indirection());
}
