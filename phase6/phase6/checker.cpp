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
# include <cassert>
# include <iostream>
# include "lexer.h"
# include "checker.h"
# include "tokens.h"
# include "Symbol.h"
# include "Scope.h"
# include "Type.h"


using namespace std;

static map<string,Scope *> fields;
static Scope *outermost, *toplevel;
static const Type error, integer("int"), longInteger("long");

static string undeclared = "'%s' undeclared";
static string redefined = "redefinition of '%s'";
static string redeclared = "redeclaration of '%s'";
static string conflicting = "conflicting types for '%s'";
static string incomplete = "'%s' has incomplete type";
static string nonpointer = "pointer type required for '%s'";

static string invalid_return = "invalid return type";
static string invalid_test = "invalid type for test expression";
static string invalid_lvalue = "lvalue required in expression";
static string invalid_operands = "invalid operands to binary %s";
static string invalid_operand = "invalid operand to unary %s";
static string invalid_cast = "invalid operand in cast expression";
static string invalid_sizeof = "invalid operand in sizeof expression";
static string invalid_function = "called object is not a function";
static string invalid_arguments = "invalid arguments to called function";
static string incomplete_type = "using pointer to incomplete type";


/*
 * Function:	promote
 *
 * Description:	Perform type promotion on the given expression.  An array
 *		is promoted a pointer by explicitly inserting an address
 *		operator.
 */

static Type promote(Expression *&expr)
{
    if (expr->type().isArray())
	expr = new Address(expr, expr->type().promote());

    return expr->type();
}


/*
 * Function:	cast
 *
 * Description:	Cast the given expression to the given type by inserting a
 *		cast operation.  As an optimization, an integer can always
 *		be converted to a long integer without an explicit cast.
 */

static Expression *cast(Expression *expr, const Type &type)
{
    unsigned long value;


    if (expr->isNumber(value))
	if (expr->type() == integer && type == longInteger) {
	    delete expr;
	    return new Number(value);
	}

    return new Cast(expr, type);
}


/*
 * Function:	extend
 *
 * Description:	Attempt to extend the type of the given expression to the
 *		given type.  The type of the given expression is only
 *		extended, not trunctated.  Promotion is also performed, if
 *		necessary.
 */

static Type extend(Expression *&expr, const Type &type)
{
    if (expr->type() == integer && type == longInteger)
	expr = cast(expr, type);

    return promote(expr);
}


/*
 * Function:	convert
 *
 * Description:	Convert the given expression to the given type as if by
 *		assignment.  Promotion is also performed, if necessary.
 */

static Type convert(Expression *&expr, const Type &type)
{

    if (expr->type() != type && expr->type().isNumeric() && type.isNumeric())
	expr = cast(expr, type);

    return promote(expr);
}


/*
 * Function:	scale
 *
 * Description:	Scale the result of pointer arithmetic.
 */

static Expression *scale(Expression *expr, unsigned size)
{
    unsigned long value;


    if (expr->isNumber(value)) {
	delete expr;
	return new Number(value * size);
    }

    extend(expr, longInteger);
    return new Multiply(expr, new Number(size), longInteger);
}


/*
 * Function:	isIncompletePointer
 *
 * Description:	Check if the specified type is a pointer to an incomplete
 *		structure type.
 */

static bool isIncompletePointer(const Type &t)
{
    if (!t.isSimple() || t.indirection() != 1)
	return false;

    return t.isStruct() && fields.count(t.specifier()) == 0;
}


/*
 * Function:	checkIfComplete
 *
 * Description:	Check if the given type is complete.  A non-structure type
 *		is always complete.  A structure type is complete if its
 *		fields have been defined.
 */

static Type checkIfComplete(const string &name, const Type &type)
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

static Type checkIfStructure(const string &name, const Type &type)
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
 * Function:	getFields
 *
 * Description:	Return the fields associated with the specified structure.
 */

Scope *getFields(const string &name)
{
    assert(fields.count(name) > 0);
    return fields[name];
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


/*
 * Function:	checkCall
 *
 * Description:	Check a function call expression: symbol (args).  The
 *		symbol must have a function type, and the number and type
 *		of the arguments must agree.
 */

Expression *checkCall(Symbol *id, Expressions &args)
{
    const Type &t = id->type();
    Parameters *params;
    Type arg, result = error;


    if (t != error) {
	if (t.isFunction()) {
	    params = t.parameters();
	    result = Type(t.specifier(), t.indirection());

	    if (params == nullptr)
		for (unsigned i = 0; i < args.size(); i ++) {
		    arg = promote(args[i]);

		    if (arg != error && !arg.isScalar()) {
			report(invalid_arguments);
			result = error;
			break;
		    }
		}

	    else if (params->size() != args.size())
		report(invalid_arguments);

	    else
		for (unsigned i = 0; i < args.size(); i ++) {
		    arg = convert(args[i], (*params)[i]);

		    if (!arg.isCompatibleWith((*params)[i])) {
			report(invalid_arguments);
			result = error;
			break;
		    }
		}

	} else
	    report(invalid_function);
    }

    return new Call(id, args, result);
}


/*
 * Function:	checkArray
 *
 * Description:	Check an array expression: left [right].  The left operand
 *		must have type "pointer to T" after promotion and the right
 *		operand must have a numeric type.  The result has type T.
 *		The pointer type must be complete.
 */

Expression *checkArray(Expression *left, Expression *right)
{
    Type t1 = left->type();
    Type t2 = right->type();
    Type result = error;


    if (t1 != error && t2 != error) {
	if (isIncompletePointer(t1))
	    report(incomplete_type);

	else if (t1.isPointer() && t2.isNumeric()) {
	    t1 = promote(left);
	    right = scale(right, t1.deref().size());
	    result = t1.deref();

	} else
	    report(invalid_operands, "[]");
    }

    return new Dereference(new Add(left, right, t1), result);
}


/*
 * Function:	checkDirectField
 *
 * Description:	Check a direct field reference expression: expr . id.  The
 *		expression must have a structure type, the identifier must
 *		be a field of that structure, and the result has the type
 *		of the field.  If the identifier is not a member of the
 *		structure, we go ahead and declare it and give it the error
 *		type, so we only get the error once.
 */

Expression *checkDirectField(Expression *expr, const std::string &id)
{
    const Type &t = expr->type();
    Symbol *symbol = nullptr;


    if (t != error) {
	if (t.isStruct() && t.indirection() == 0) {
	    symbol = fields[t.specifier()]->find(id);

	    if (symbol == nullptr) {
		symbol = new Symbol(id, error);
		fields[t.specifier()]->insert(symbol);
		report(invalid_operands, ".");
	    }

	} else
	    report(invalid_operands, ".");
    }

    if (symbol == nullptr)
	symbol = new Symbol("-unknown-", error);

    return new Field(expr, symbol, symbol->type());
}


/*
 * Function:	checkIndirectField
 *
 * Description:	Check an indirect field reference expression: expr -> id.
 *		The expression must have type pointer(T), where T is a
 *		complete structure type, the identifier must be a field of
 *		that structure, and the result has the type of the field.
 *		Once again, if the identifier is not a member of the
 *		structure, we go ahead and declare it and give it the error
 *		type, so we only get the error once.
 */

Expression *checkIndirectField(Expression *expr, const std::string &id)
{
    Type t = promote(expr);
    Symbol *symbol = nullptr;


    if (t != error) {
	if (isIncompletePointer(t))
	    report(incomplete_type);

	else if (t.isStruct() && t.indirection() == 1) {
	    symbol = fields[t.specifier()]->find(id);
	    t = t.deref();

	    if (symbol == nullptr) {
		symbol = new Symbol(id, error);
		fields[t.specifier()]->insert(symbol);
		report(invalid_operands, "->");
	    }

	} else
	    report(invalid_operands, "->");
    }

    if (symbol == nullptr)
	symbol = new Symbol("-unknown-", error);

    return new Field(new Dereference(expr, t), symbol, symbol->type());
}


/*
 * Function:	checkNot
 *
 * Description:	Check a logical negation expression: ! expr.  The operand
 *		must have a scalar type, and the result has type int.
 */

Expression *checkNot(Expression *expr)
{
    const Type &t = promote(expr);
    Type result = error;


    if (t != error) {
	if (t.isScalar())
	    result = integer;
	else
	    report(invalid_operand, "!");
    }

    return new Not(expr, result);
}


/*
 * Function:	checkNegate
 *
 * Description:	Check an arithmetic negation expression: - expr.  The
 *		operand must have a numeric type, and the result has the
 *		same type.
 */

Expression *checkNegate(Expression *expr)
{
    const Type &t = expr->type();
    Type result = error;


    if (t != error) {
	if (t.isNumeric())
	    result = t;
	else
	    report(invalid_operand, "-");
    }

    return new Negate(expr, result);
}


/*
 * Function:	checkDereference
 *
 * Description:	Check a dereference expression: * expr.  The operand must
 *		have type "pointer to T," where T is complete, and the
 *		result has type T.
 */

Expression *checkDereference(Expression *expr)
{
    const Type &t = promote(expr);
    Type result = error;


    if (t != error) {
	if (isIncompletePointer(t))
	    report(incomplete_type);
	else if (t.isPointer())
	    result = t.deref();
	else
	    report(invalid_operand, "*");
    }

    return new Dereference(expr, result);
}


/*
 * Function:	checkAddress
 *
 * Description:	Check an address expression: & expr.  The operand must be
 *		an lvalue, and if it has type T, then the result has type
 *		"pointer to T."
 */

Expression *checkAddress(Expression *expr)
{
    const Type &t = expr->type();
    Type result = error;


    if (t != error) {
	if (expr->lvalue())
	    result = Type(t.specifier(), t.indirection() + 1);
	else
	    report(invalid_lvalue);
    }

    return new Address(expr, result);
}


/*
 * Function:	checkSizeof
 *
 * Description:	Check a sizeof expression: sizeof expr.  The operand cannot
 *		be a function type.
 */

Expression *checkSizeof(Expression *expr)
{
    const Type &t = expr->type();


    if (t != error && !t.isFunction())
	return new Number(expr->type().size());

    report(invalid_sizeof);
    return new Number(0);
}


/*
 * Function:	checkCast
 *
 * Description:	Check a cast expression: (type) expr.  The result and
 *		operand must either both have numeric types or both have
 *		pointer types after any promotion.
 */

Expression *checkCast(const Type &type, Expression *expr)
{
    const Type &t = promote(expr);


    if (t == error || type == t)
	return expr;

    if (type.isNumeric() && t.isNumeric())
	return new Cast(expr, type);

    if (type.isPointer() && t.isPointer())
	return new Cast(expr, type);

    report(invalid_cast);
    return new Cast(expr, error);
}


/*
 * Function:	checkMultiplicative
 *
 * Description:	Check a multiplication, division, or remainder expression:
 *		both operands must have numeric types, and the result has
 *		type long if either operand has type long, and has type int
 *		otherwise.
 */

static Type checkMultiplicative(Expression *&left, Expression *&right,
	const string &op)
{
    const Type &t1 = extend(left, right->type());
    const Type &t2 = extend(right, left->type());
    Type result = error;


    if (t1 != error && t2 != error) {
	if (t1.isNumeric() && t2.isNumeric())
	    result = t1;
	else
	    report(invalid_operands, op);
    }

    return result;
}


/*
 * Function:	checkMultiply
 *
 * Description:	Check a multiplication expression: left * right.
 */

Expression *checkMultiply(Expression *left, Expression *right)
{
    Type t = checkMultiplicative(left, right, "*");
    return new Multiply(left, right, t);
}


/*
 * Function:	checkDivide
 *
 * Description:	Check a division expression: left / right.
 */

Expression *checkDivide(Expression *left, Expression *right)
{
    Type t = checkMultiplicative(left, right, "/");
    return new Divide(left, right, t);
}

/*
 * Function:	checkRemainder
 *
 * Description:	Check a remainder expression: left % right.
 */

Expression *checkRemainder(Expression *left, Expression *right)
{
    Type t = checkMultiplicative(left, right, "%");
    return new Remainder(left, right, t);
}


/*
 * Function:	checkAdd
 *
 * Description:	Check an addition expression: left + right.  If both
 *		operands have numeric types, then the result has type long
 *		is either has type long, and has type int otherwise; if one
 *		operand has a pointer type and the other has a numeric
 *		type, then the result has that pointer type.  Any pointer
 *		type must be complete.
 */

Expression *checkAdd(Expression *left, Expression *right)
{
    Type t1 = left->type();
    Type t2 = right->type();
    Type result = error;


    if (t1 != error && t2 != error) {
	if (isIncompletePointer(t1) || isIncompletePointer(t2))
	    report(incomplete_type);

	else if (t1.isNumeric() && t2.isNumeric()) {
	    t1 = extend(left, t2);
	    t2 = extend(right, t1);
	    result = t1;

	} else if (t1.isPointer() && t2.isNumeric()) {
	    t1 = promote(left);
	    right = scale(right, t1.deref().size());
	    result = t1;

	} else if (t1.isNumeric() && t2.isPointer()) {
	    t2 = promote(right);
	    left = scale(left, t2.deref().size());
	    result = t2;

	} else
	    report(invalid_operands, "+");
    }

    return new Add(left, right, result);
}


/*
 * Function:	checkSubtract
 *
 * Description:	Check a subtraction expression: left - right.  If both
 *		operands have numeric types, then the result has type long
 *		if either has type long, and has type int otherwise; if the
 *		left operand has a pointer type and the right operand has a
 *		numeric type, then the result has that pointer type.  If
 *		both operands have identical pointer types, then the result
 *		has type long.  Any pointer type must be complete.
 */

Expression *checkSubtract(Expression *left, Expression *right)
{
    Expression *tree;
    Type t1 = left->type();
    Type t2 = right->type();
    Type result = error;


    if (t1 != error && t2 != error) {
	if (isIncompletePointer(t1) || isIncompletePointer(t2))
	    report(incomplete_type);

	else if (t1.isNumeric() && t2.isNumeric()) {
	    t1 = extend(left, t2);
	    t2 = extend(right, t1);
	    result = t1;

	} else if (t1.isPointer() && t2.isNumeric()) {
	    t1 = promote(left);
	    right = scale(right, t1.deref().size());
	    result = t1;

	} else if (t1.isPointer() && t1.promote() == t2.promote()) {
	    t1 = promote(left);
	    t2 = promote(right);
	    result = longInteger;

	} else
	    report(invalid_operands, "-");
    }

    tree = new Subtract(left, right, result);

    if (t1.isPointer() && t1 == t2)
	tree = new Divide(tree, new Number(t1.deref().size()), longInteger);

    return tree;
}


/*
 * Function:	checkComparative
 *
 * Description:	Check an equality or relational expression: the types of
 *		both operands must be compatible, and the result has type
 *		int.
 */

static Type checkComparative(Expression *&left, Expression *&right,
	const string &op)
{
    const Type &t1 = extend(left, right->type());
    const Type &t2 = extend(right, left->type());
    Type result = error;


    if (t1 != error && t2 != error) {
	if (t1.isCompatibleWith(t2))
	    result = integer;
	else
	    report(invalid_operands, op);
    }

    return result;
}


/*
 * Function:	checkEqual
 *
 * Description:	Check an equality expression: left == right.
 */

Expression *checkEqual(Expression *left, Expression *right)
{
    Type t = checkComparative(left, right, "==");
    return new Equal(left, right, t);
}


/*
 * Function:	checkNotEqual
 *
 * Description:	Check an inequality expression: left != right.
 */

Expression *checkNotEqual(Expression *left, Expression *right)
{
    Type t = checkComparative(left, right, "!=");
    return new NotEqual(left, right, t);
}


/*
 * Function:	checkLessThan
 *
 * Description:	Check a less-than expression: left < right.
 */

Expression *checkLessThan(Expression *left, Expression *right)
{
    Type t = checkComparative(left, right, "<");
    return new LessThan(left, right, t);
}


/*
 * Function:	checkGreaterThan
 *
 * Description:	Check a greater-than expression: left > right.
 */

Expression *checkGreaterThan(Expression *left, Expression *right)
{
    Type t = checkComparative(left, right, ">");
    return new GreaterThan(left, right, t);
}


/*
 * Function:	checkLessOrEqual
 *
 * Description:	Check a less-than-or-equal expression: left <= right.
 */

Expression *checkLessOrEqual(Expression *left, Expression *right)
{
    Type t = checkComparative(left, right, "<=");
    return new LessOrEqual(left, right, t);
}


/*
 * Function:	checkGreaterOrEqual
 *
 * Description:	Check a greater-than-or-equal expression: left >= right.
 */

Expression *checkGreaterOrEqual(Expression *left, Expression *right)
{
    Type t = checkComparative(left, right, ">=");
    return new GreaterOrEqual(left, right, t);
}


/*
 * Function:	checkLogical
 *
 * Description:	Check a logical-or or a logical-and expression: the types
 *		of both operands must be scalar types and the result has
 *		type int.
 */

static Type checkLogical(Expression *&left, Expression *&right,
	const string &op)
{
    const Type &t1 = promote(left);
    const Type &t2 = promote(right);
    Type result = error;


    if (t1 != error && t2 != error) {
	if (t1.isScalar() && t2.isScalar())
	    result = integer;
	else
	    report(invalid_operands, op);
    }

    return result;
}


/*
 * Function:	checkLogicalAnd
 *
 * Description:	Check a logical-and expression: left && right.
 */

Expression *checkLogicalAnd(Expression *left, Expression *right)
{
    Type t = checkLogical(left, right, "&&");
    return new LogicalAnd(left, right, t);
}


/*
 * Function:	checkLogicalOr
 *
 * Description:	Check a logical-or expression: left || right.
 */

Expression *checkLogicalOr(Expression *left, Expression *right)
{
    Type t = checkLogical(left, right, "||");
    return new LogicalOr(left, right, t);
}


/*
 * Function:	checkAssignment
 *
 * Description:	Check an assignment statement: the left operand must be an
 *		lvalue and the types of the operands must be compatible.
 */

Statement *checkAssignment(Expression *left, Expression *right)
{
    const Type &t1 = left->type();
    const Type &t2 = convert(right, left->type());


    if (t1 != error && t2 != error) {
	if (!left->lvalue())
	    report(invalid_lvalue);

	else if (!t1.isCompatibleWith(t2))
	    report(invalid_operands, "=");
    }

    return new Assignment(left, right);
}


/*
 * Function:	checkReturn
 *
 * Description:	Check a return statement: the type of the expression must
 *		be compatible with the given type, which should be the
 *		return type of the enclosing function.
 */

void checkReturn(Expression *&expr, const Type &type)
{
    const Type &t = convert(expr, type);


    if (t != error && !t.isCompatibleWith(type))
	report(invalid_return);
}


/*
 * Function:	checkTest
 *
 * Description:	Check if the type of the expression is a legal type in a
 *		test expression in a while, if-then, or if-then-else
 *		statement: the type must be a scalar type.
 */

void checkTest(Expression *&expr)
{
    const Type &t = promote(expr);


    if (t != error && !t.isScalar())
	report(invalid_test);
}
