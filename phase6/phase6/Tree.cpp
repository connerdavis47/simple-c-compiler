/*
 * File:	Tree.cpp
 *
 * Description:	This file contains the constructor and accessor member
 *		function definitions for abstract syntax trees in Simple C.
 *
 *		The tree is actually built during semantic checking, where
 *		type information is readily available.  Any simplifications
 *		or canonicalizations of the tree are performed when it is
 *		constructed.
 *
 *		The functions here are just the constructors and accessors.
 *		The storage allocation and code generation functions are
 *		located elsewhere.  Most of these functions don't do
 *		anything interesting, and could easily be put in the header
 *		file, but we don't like to do that.
 *
 *		Extra functionality:
 *		- everything (it is optional to construct an AST)
 */

# include "Tree.h"
# include <sstream>
# include <cstdlib>

using namespace std;


/*
 * Function:	Expression::Expression (constructor)
 *
 * Description:	Initialize the expression object to not be an lvalue and to
 *		have the specified type.
 */

Expression::Expression(const Type &type)
    : _type(type), _lvalue(false), _offset(0), _register(nullptr)
{
}


/*
 * Function:	Expression::type (accessor)
 *
 * Description:	Return the type of this expression.
 */

const Type &Expression::type() const
{
    return _type;
}


/*
 * Function:	Expression::lvalue (accessor)
 *
 * Description:	Return whether this expression is an lvalue.
 */

bool Expression::lvalue() const
{
    return _lvalue;
}


/*
 * Function:	Binary::Binary (constructor)
 *
 * Description:	Initialize this expression as a binary operator with the
 *		specified children.
 */

Binary::Binary(Expression *left, Expression *right, const Type &type)
    : Expression(type), _left(left), _right(right)
{
    _hasCall = left->_hasCall | right->_hasCall;
}


/*
 * Function:	Unary::Unary (constructor)
 *
 * Description:	Initialize this expression as a unary operator with the
 *		specified child.
 */

Unary::Unary(Expression *expr, const Type &type)
    : Expression(type), _expr(expr)
{
    _hasCall = expr->_hasCall;
}


/*
 * Function:	String::String (constructor)
 *
 * Description:	Initialize this string literal.  We don't care about the
 *		length of the string, only that it is an array.
 */

String::String(const string &value)
    : Expression(Type("char", 0, 1)), _value(value)
{
}


/*
 * Function:	String::value (accessor)
 *
 * Description:	Return the value of this string.
 */

const string &String::value() const
{
    return _value;
}


/*
 * Function:	Identifier::Identifier (constructor)
 *
 * Description:	Initialize this identifier object.  An identifier is an
 *		lvalue if its type is a simple type.
 */

Identifier::Identifier(const Symbol *symbol)
    : Expression(symbol->type()), _symbol(symbol)
{
    _lvalue = symbol->type().isSimple();
}


/*
 * Function:	Identifier::symbol (accessor)
 *
 * Description:	Return the symbol of this identifier.
 */

const Symbol *Identifier::symbol() const
{
    return _symbol;
}


/*
 * Function:	Number::Number (constructor)
 *
 * Description:	Initialize a number, which has type int or long.
 */

Number::Number(const string &value)
    : Expression(Type("int")), _value(value)
{
    unsigned long val;
    char *ptr;


    val = strtoul(value.c_str(), &ptr, 0);

    if (*ptr == 'l' || *ptr == 'L' || (unsigned) val != val)
	_type = Type("long");

    if (*ptr == 'l' || *ptr == 'L')
	_value.erase(_value.size() - 1);
}


/*
 * Function:	Number::Number (constructor)
 *
 * Description:	Initialize a number, which always has type long.
 */

Number::Number(unsigned long value)
    : Expression(Type("long"))
{
    stringstream ss;

    ss << value;
    _value = ss.str();
}


/*
 * Function:	Number::value (accessor)
 *
 * Description:	Return the value of this number.
 */

const string &Number::value() const
{
    return _value;
}


/*
 * Function:	Call::Call (constructor)
 *
 * Description:	Initialize a function call expression.
 */

Call::Call(const Symbol *id, const Expressions &args, const Type &type)
    : Expression(type), _id(id), _args(args)
{
    _hasCall = true;
}


/*
 * Function:	Field::Field (constructor)
 *
 * Description:	Initialize a field reference expression.
 */

Field::Field(Expression *expr, Symbol *id, const Type &type)
    : Expression(type), _expr(expr), _id(id)
{
    _lvalue = expr->lvalue() && !id->type().isArray();
}


/*
 * Function:	Not::Not (constructor)
 *
 * Description:	Initialize a logical negation expression.
 */

Not::Not(Expression *expr, const Type &type)
    : Unary(expr, type)
{
}


/*
 * Function:	Negate::Negate (constructor)
 *
 * Description:	Initialize an arithmetic negation expression.
 */

Negate::Negate(Expression *expr, const Type &type)
    : Unary(expr, type)
{
}


/*
 * Function:	Dereference::Dereference (constructor)
 *
 * Description:	Initialize a dereference expression.
 */

Dereference::Dereference(Expression *expr, const Type &type)
    : Unary(expr, type)
{
    _lvalue = true;
}


/*
 * Function:	Address::Address (constructor)
 *
 * Description:	Initialize an address expression.
 */

Address::Address(Expression *expr, const Type &type)
    : Unary(expr, type)
{
}


/*
 * Function:	Cast::Cast (constructor)
 *
 * Description:	Initialize a cast expression.
 */

Cast::Cast(Expression *expr, const Type &type)
    : Unary(expr, type)
{
}


/*
 * Function:	Multiply::Multiply (constructor)
 *
 * Description:	Initialize a multiplication expression.
 */

Multiply::Multiply(Expression *left, Expression *right, const Type &type)
    : Binary(left, right, type)
{
}


/*
 * Function:	Divide::Divide (constructor)
 *
 * Description:	Initialize a division expression.
 */

Divide::Divide(Expression *left, Expression *right, const Type &type)
    : Binary(left, right, type)
{
}


/*
 * Function:	Remainder::Remainder (constructor)
 *
 * Description:	Initialize a remainder expression.
 */

Remainder::Remainder(Expression *left, Expression *right, const Type &type)
    : Binary(left, right, type)
{
}


/*
 * Function:	Add::Add (constructor)
 *
 * Description:	Initialize an addition expression.
 */

Add::Add(Expression *left, Expression *right, const Type &type)
    : Binary(left, right, type)
{
}


/*
 * Function:	Subtract::Subtract (constructor)
 *
 * Description:	Initialize a subtraction expression.
 */

Subtract::Subtract(Expression *left, Expression *right, const Type &type)
    : Binary(left, right, type)
{
}


/*
 * Function:	LessThan::LessThan (constructor)
 *
 * Description:	Initialize a less-than expression.
 */

LessThan::LessThan(Expression *left, Expression *right, const Type &type)
    : Binary(left, right, type)
{
}


/*
 * Function:	GreaterThan
 *
 * Description:	Initialize a greater-than expression.
 */

GreaterThan::GreaterThan(Expression *left, Expression *right, const Type &type)
    : Binary(left, right, type)
{
}


/*
 * Function:	LessOrEqual
 *
 * Description:	Initialize a less-than-or-equal expression.
 */

LessOrEqual::LessOrEqual(Expression *left, Expression *right, const Type &type)
    : Binary(left, right, type)
{
}


/*
 * Function:	GreaterOrEqual
 *
 * Description:	Initialize a greater-than-or-equal expression.
 */

GreaterOrEqual::GreaterOrEqual(Expression *left, Expression *right, const Type &type)
    : Binary(left, right, type)
{
}


/*
 * Function:	Equal::Equal (constructor)
 *
 * Description:	Initialize an equality expression.
 */

Equal::Equal(Expression *left, Expression *right, const Type &type)
    : Binary(left, right, type)
{
}


/*
 * Function:	NotEqual::NotEqual (constructor)
 *
 * Description:	Initialize an inequality expression.
 */

NotEqual::NotEqual(Expression *left, Expression *right, const Type &type)
    : Binary(left, right, type)
{
}


/*
 * Function:	LogicalAnd::LogicalAnd (constructor)
 *
 * Description:	Initialize a logical-and expression.
 */

LogicalAnd::LogicalAnd(Expression *left, Expression *right, const Type &type)
    : Binary(left, right, type)
{
}


/*
 * Function:	LogicalOr::LogicalOr (constructor)
 *
 * Description:	Initialize a logical-or expression.
 */

LogicalOr::LogicalOr(Expression *left, Expression *right, const Type &type)
    : Binary(left, right, type)
{
}


/*
 * Function:	Assignment::Assignment (constructor)
 *
 * Description:	Initialize an assignment statement.
 */

Assignment::Assignment(Expression *left, Expression *right)
    : _left(left), _right(right)
{
    _hasCall = left->_hasCall | right->_hasCall;
}


/*
 * Function:	Return::Return (constructor)
 *
 * Description:	Initialize a return statement.
 */

Return::Return(Expression *expr)
    : _expr(expr)
{
    _hasCall = expr->_hasCall;
}


/*
 * Function:	Block::Block (constructor)
 *
 * Description:	Initialize a block statement.
 */

Block::Block(Scope *decls, const Statements &stmts)
    : _decls(decls), _stmts(stmts)
{
    for (unsigned i = 0; i < stmts.size(); i ++)
	if (stmts[i]->_hasCall) {
	    _hasCall = true;
	    break;
	}
}


/*
 * Function:	Block::declarations (accessor)
 *
 * Description:	Return the declarations of this block.
 */

Scope *Block::declarations() const
{
    return _decls;
}


/*
 * Function:	While::While (constructor)
 *
 * Description:	Initialize a while statement.
 */

While::While(Expression *expr, Statement *stmt)
    : _expr(expr), _stmt(stmt)
{
    _hasCall = expr->_hasCall | stmt->_hasCall;
}


/*
 * Function:	If::If (constructor)
 *
 * Description:	Initialize an if-then or if-then-else statement.
 */

If::If(Expression *expr, Statement *thenStmt, Statement *elseStmt)
    : _expr(expr), _thenStmt(thenStmt), _elseStmt(elseStmt)
{
    _hasCall = expr->_hasCall | thenStmt->_hasCall;

    if (elseStmt != nullptr && elseStmt->_hasCall)
	_hasCall = true;
}


/*
 * Function:	Simple::Simple (constructor)
 *
 * Description:	Initialize a simple (expression) statement.
 */

Simple::Simple(Expression *expr)
    : _expr(expr)
{
    _hasCall = expr->_hasCall;
}


/*
 * Function:	Function::Function (constructor)
 *
 * Description:	Initialize a function object.
 */

Function::Function(const Symbol *id, Block *body)
    : _id(id), _body(body)
{
    _hasCall = body->_hasCall;
}


/*
 * Function:	Expression::isNumber (accessor)
 *
 * Description:	Return false since most expressions are not numbers.
 */

bool Expression::isNumber(unsigned long &value) const
{
    return false;
}


/*
 * Function:	Number::isNumber (accessor)
 *
 * Description:	Return true since a number is in fact a number.
 */

bool Number::isNumber(unsigned long &value) const
{
    value = strtoul(_value.c_str(), NULL, 0);
    return true;
}
