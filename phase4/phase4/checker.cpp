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

static string invalid_return = "invalid return type";                    // E1
static string invalid_test = "invalid type for test expression";         // E2
static string expected_lvalue = "lvalue required in expression";         // E3
static string invalid_binary = "invalid operands to binary %s";          // E4
static string invalid_unary = "invalid operand to unary %s";             // E5
static string invalid_cast = "invalid operand in cast expression";       // E6
static string invalid_sizeof = "invalid operand in sizeof expression";   // E7
static string expected_func = "called object is not a function";         // E8
static string invalid_args = "invalid arguments to called function";     // E9
static string invalid_ptr = "using pointer to incomplete type";       // E10

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

Symbol* defineFunction( const string& name, const Type& type )
{
    Symbol* symbol = outermost->find(name);

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

Symbol* declareFunction( const string& name, const Type& type )
{
    Symbol* symbol = outermost->find(name);

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

Symbol* declareParameter( const string& name, const Type& type )
{
    return declareVariable(name, checkIfStructure(name, type));
}


/*
 * Function:	declareVariable
 *
 * Description:	Declare a variable with the specified NAME and TYPE.  Any
 *		redeclaration is discarded.
 */

Symbol* declareVariable( const string& name, const Type& type )
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

Symbol* checkIdentifier( const string& name )
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


/**
 * The result has type long if either operand has type long, and has type 
 * int otherwise.
 */
static Type coerceIntToLong( const Type& left, const Type& right )
{
    return (left.specifier() == "long" || right.specifier() == "long") 
        ? longinteger : integer;
}


/**
 * A pointer is complete if it refers to a structure that has previously been
 * defined. Otherwise, the pointer is considered incomplete.
 */
static bool isCompletePointer( const Type& t )
{
  return !(t.isStruct() && t.indirection() == 1 && fields.count(t.specifier()) == 0);
}


Type checkReturn( const Type& expr, const Type& type ) 
{ 
    if (expr.isError() || type.isError())
        return error;

    // The type of the expression in a return statement must be compatible with the 
    // return type of the enclosing function
    if (expr.isCompatibleWith(type))
        return expr;

    // [E1]
    report(invalid_return);
    return error;
}
Type checkTest( const Type& expr )
{
    if (expr.isError())
        return error;

    // The type of an expression in a while or if statement must be a scalar type
    if (expr.isScalar())
        return expr;

    // [E2]
    report(invalid_test);
    return error;
}
Type checkAssignment( const Type& left, const Type& right, const bool& lvalue ) 
{ 
    if (left.isError() || right.isError())
        return error;

    // In an assignment statement, the left-hand side must be an lvalue
    if (lvalue)
    {
        // and the types of the left-hand and right-hand sides must be compatible 
        if (left.isCompatibleWith(right))
            return left;

        // [E4]
        report(invalid_binary, "=");
        return error;
    }

    // [E3]
    report(expected_lvalue);
    return error;
}


Type checkLogical( const Type& left, const Type& right, const std::string& op ) 
{ 
    if (left.isError() || right.isError())
        return error;
        
    // The type of each operand must be a scalar type
    // The types of the two operands need not be compatible.
    if (left.isScalar() && right.isScalar())
        return integer; /* The result has type int and is not an lvalue */

    // [E4]
    report(invalid_binary, op);
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
        
    // The types of the left and right operands must be compatible
    if (left.isCompatibleWith(right))
        return integer; /* The result has type int and is not an lvalue */

    // [E4]
    report(invalid_binary, op);
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
        
    // The types of the left and right operands must be compatible
    if (left.isCompatibleWith(right))
        return integer; /* The result has type int and is not an lvalue */

    // [E4]
    report(invalid_binary, op);
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
    // If the types of both operands are numeric, then the result has type long 
    // if either operand has type long, and has type int otherwise.
    if (left.isNumeric() && right.isNumeric())
        return coerceIntToLong(left, right);

    // If the left operand has type “pointer to T” and the right operand has a 
    // numeric type
    if (left.isPointer() && right.isNumeric())
    {
        // Additionally, if any operand has type “pointer to T” then T must be 
        // complete.
        if (isCompletePointer(left))
            return left; /* then the result has type “pointer to T.” */

        // [E10]
        report(invalid_ptr);
        return error;
    }

    // [E4]
    report(invalid_binary, op);
    return error;
}
Type checkAdd( const Type& left, const Type& right ) 
{
    if (left.isError() || right.isError())
        return error;

    // In all cases, operands undergo type promotion
    const Type t1 = left.promote();
    const Type t2 = right.promote();

    // For addition only, if the left operand has a numeric type and the right 
    // operand has type “pointer to T”
    if (t1.isNumeric() && t2.isPointer())
    {
        // Additionally, if any operand has type “pointer to T” then T must be 
        // complete.
        if (isCompletePointer(t2))
            return t2; /* then the result has type “pointer to T.” */

        // [E10]
        report(invalid_ptr);
        return error;
    }

    return checkAdditive(t1, t2, "+");
}
Type checkSubtract( const Type& left, const Type& right ) 
{ 
    if (left.isError() || right.isError())
        return error;

    // In all cases, operands undergo type promotion
    const Type t1 = left.promote();
    const Type t2 = right.promote();

    // For subtraction only, if both operands have type “pointer to T”
    if (t1.isPointer() && t2.isPointer())
    {
        // Additionally, if any operand has type “pointer to T” then T must be 
        // complete.
        if (isCompletePointer(t1) && isCompletePointer(t2))
        {

            if (t1.isCompatibleWith(t2))
                return longinteger; /* then the result has type long. */

            // [E4]
            report(invalid_binary, "-");
            return error;
        }

        // [E10]
        report(invalid_ptr);
        return error;
    }

    return checkAdditive(t1, t2, "-");
}


Type checkMultiplicative( const Type& left, const Type& right, const std::string& op ) 
{ 
    if (left.isError() || right.isError())
        return error;

    // The types of both operands must be numeric
    // If either operand has type long, then the result has type long. Otherwise, 
    // the result has type int.
    if (left.isNumeric() && right.isNumeric())
        return coerceIntToLong(left, right);

    // [E4]
    report(invalid_binary, op);
    return error;
}
Type checkMultiply( const Type& left, const Type& right ) 
{ 
    return checkMultiplicative(left, right, "*");
}
Type checkDivide( const Type& left, const Type& right ) 
{ 
    return checkMultiplicative(left, right, "/");
}
Type checkRemainder( const Type& left, const Type& right ) 
{ 
    return checkMultiplicative(left, right, "%");
}


Type checkNegate( const Type& expr ) 
{ 
    if (expr.isError())
        return error;
        
    // The operand in a unary - expression must have a numeric type
    if (expr.isNumeric())
        return expr; /* and the result has the same type */

    // [E5]
    report(invalid_unary, "-");
    return error;
}
Type checkNot( const Type& expr ) 
{ 
    if (expr.isError())
        return error;
        
    // The operand in a ! expression must have a scalar type
    if (expr.isScalar())
        return integer; /* and the result has type int */

    // [E5]
    report(invalid_unary, "!");
    return error;
}
Type checkAddress( const Type& expr, const bool& lvalue ) 
{
    if (expr.isError())
        return error;

    // The operand in a unary & expression must be an lvalue
    // If the operand has type T, then the result has type “pointer to T” and is 
    // not an lvalue.
    if (lvalue)
        return Type(expr.specifier(), expr.indirection() + 1);

    // [E3]
    report(expected_lvalue);
    return error;
}
Type checkDereference( const Type& expr )
{
    if (expr.isError())
        return error;

    // after any promotion
    const Type t1 = expr.promote();

    // The operand in a unary * expression must have type “pointer to T” 
    if (t1.isPointer())
    {
        // and T must be complete
        if (isCompletePointer(t1))
            return Type(t1.specifier()); /* The result has type T and is an lvalue. */

        // [E10]
        report(invalid_ptr);
        return error;
    }

    // [E5]
    report(invalid_unary, "*");
    return error;
}
Type checkSizeof( const Type& expr ) 
{ 
    if (expr.isError())
        return error;

    // The operand of a sizeof expression must not have a function type
    if (!expr.isFunction())
        return longinteger; /* The result of the expression has type long */

    // [E7]
    report(invalid_sizeof);
    return error;
}
Type checkTypeCast( const Type& left, const Type& right )
{
    if (left.isError() || right.isError())
        return error;

    // (after any promotion) 
    const Type t1 = left.promote();
    const Type t2 = right.promote();

    // The types of the result and operand must either both be numeric 
    if ((t1.isNumeric() && t2.isNumeric()))
        return t1;
        
    // or both be pointer types
    if (t1.isPointer() && t2.isPointer())
    {
        if (isCompletePointer(t1) && isCompletePointer(t2))
            return t1; /* The result is not an lvalue. */

        // [E10]
        report(invalid_ptr);
        return error;
    }

    // [E6]
    report(invalid_cast);
    return error;
}


Type checkArray( const Type& left, const Type& right ) 
{ 
    if (left.isError() || right.isError())
        return error;

    // after any promotion
    const Type t1 = left.promote();

    // The left operand in an array reference expression must have type 
    // “pointer to T”
    if (t1.isPointer())
    {
        // the expression must have a numeric type
        if (right.isNumeric())
        {
            // and T must be complete
            // The result has type T and is an lvalue.
            if (isCompletePointer(t1))
                return Type(t1.specifier(), t1.indirection() - 1);

            // [E10]
            report(invalid_ptr);
            return error;
        }

        // [E4]
        report(invalid_binary, "[]");
        return error;
    }

    // [E4]
    report(invalid_binary, "[]");
    return error;
}
Type checkStructField( const Type& type, const string field ) 
{ 
    if (type.isError())
        return error;

    // The operand in a direct structure field reference must be a structure type
    if (type.isStruct())
    {
        // and the identifier must be a field of the structure
        if (fields.find(type.specifier()) != fields.end())
        {
            // in which case the type of the expression is the type of the identifier.
            const Symbols typeFields = fields.find(type.specifier())->second->symbols();
            for (unsigned i = 0; i < typeFields.size(); ++i)
                if (typeFields[i]->name() == field)
                    return typeFields[i]->type();

            // [E4]
            report(invalid_binary, ".");
            return error;
        }

        // [E4]
        report(invalid_binary, ".");
        return error;
    }

    // [E4]
    report(invalid_binary, ".");
    return error;
}
Type checkStructPointerField( const Type& type, const string field ) 
{ 
    if (type.isError())
        return error;

    // (after any promotion)
    const Type t1 = type.promote();

    // The operand in an indirect structure field reference must be a pointer to a 
    // structure type
    if (t1.isPointer() && t1.isStruct())
    {
        // the structure type must be complete
        if (isCompletePointer(t1))
        {
            // and the identifier must be a field of the structure
            if (fields.find(t1.specifier()) != fields.end())
            {
                // in which case the type of the expression is the type of the identifier.
                const Symbols typeFields = fields.find(t1.specifier())->second->symbols();
                for (unsigned i = 0; i < typeFields.size(); ++i)
                    if (typeFields[i]->name() == field)
                        return typeFields[i]->type();
            }

            // [E4]
            report(invalid_binary, "->");
            return error;
        }

        // [E10]
        report(invalid_ptr);
        return error;
    }

    // [E4]
    report(invalid_binary, "->");
    return error;
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
        report(expected_func);
        return error;
    }

    // The identifier in a function call expression must have type “function returning T"
    Type t1 = symbol->type();
    if (!t1.isFunction())
    {
        // [E8]
        report(expected_func);
        return error;
    }

    // if the parameters have been specified 
    Parameters* params = t1.parameters();
    if (params != nullptr)
    {
        // the number of parameters and arguments must agree 
        if (params->size() != args.size())
        {
            report(invalid_args);
            return error;
        }

        for (unsigned i = 0; i < args.size(); ++i)
        {
            // Arguments always undergo type promotion.
            const Type type1 = (*params)[i];
            const Type type2 = args[i].promote();

            // the arguments must all have scalar types,
            // and their types must be compatible
            if (!type1.isCompatibleWith(type2) || !type1.isScalar() || !type2.isScalar())
            {
                // [E9]
                report(invalid_args);
                return error;
            }
        }
    }

    // the result has type T
    return Type(t1.specifier(), t1.indirection());
}
