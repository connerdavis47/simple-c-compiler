/*
 * File:	Tree.h
 *
 * Description:	This file contains the class definitions for abstract
 *		syntax trees in Simple C.
 *
 *		The base class Node cannot not be instantiated (the
 *		constructor is private).  It provides empty functions for
 *		storage allocation and code generation.
 *
 *		A Node is either a Function, representing a function
 *		definition, or a Statement, which also cannot be
 *		instantiated (again, the constructor is private).
 *
 *		Since the compiler has a very functional design (semantic
 *		checking, storage allocation, code generation), its design
 *		doesn't necessarily mesh well with a tree designed using
 *		object-orientation.  So, here is my compromise:
 *
 *		Tree.h - class definitions
 *		Tree.cpp - constructors and accessors
 *		allocator.cpp - member functions to do storage allocation
 *		generator.cpp - member functions to do code generation
 *		writer.cpp - member functions to write the tree of a stream
 */

# ifndef TREE_H
# define TREE_H

# include <string>
# include <vector>
# include <ostream>

# include "Label.h"
# include "Scope.h"
# include "Register.h"

typedef std::vector<class Statement *> Statements;
typedef std::vector<class Expression *> Expressions;


/* The base class */

class Node {
protected:
    typedef std::string string;
    typedef std::ostream ostream;
    Node() : _hasCall(false) {}

public:
    bool _hasCall;
    virtual ~Node() {}
    virtual void write(ostream &ostr) const = 0;
    virtual void allocate(int &offset) const {}
    virtual void generate() {}
};


/* Any type of statement: return, while, if, block, and expression */

class Statement : public Node {
protected:
    Statement() {}
};


/* An expression */

class Expression : public Node {
protected:
    Type _type;
    bool _lvalue;
    Expression(const Type &type);

public:
    int _offset;
    Register *_register;

    const Type &type() const;
    bool lvalue() const;
    virtual bool isNumber(unsigned long &value) const;
    virtual void operand(ostream &ostr) const;
    virtual void test(const Label& label, bool ifTrue);
};


/* A binary operator */

class Binary : public Expression {
protected:
    Expression *_left, *_right;
    Binary(Expression *left, Expression *right, const Type &type);
};


/* A unary operator */

class Unary : public Expression {
protected:
    Expression *_expr;
    Unary(Expression *expr, const Type &type);
};


/* A string literal (strings really are just not expressions syntactically) */

class String : public Expression {
    string _value;

public:
    String(const string &value);
    const string &value() const;
    virtual void write(ostream &ostr) const;
    virtual void operand(ostream &ostr) const;
};


/* An identifier expression */

class Identifier : public Expression {
    const Symbol *_symbol;

public:
    Identifier(const Symbol *symbol);
    const Symbol *symbol() const;
    virtual void write(ostream &ostr) const;
    virtual void operand(ostream &ostr) const;
};


/* A number (i.e., integer literal) */

class Number : public Expression {
    string _value;

public:
    Number(const string &value);
    Number(unsigned long value);
    const string &value() const;
    virtual bool isNumber(unsigned long &value) const;
    virtual void write(ostream &ostr) const;
    virtual void operand(ostream &ostr) const;
};


/* A function call expression: id ( args ) */

class Call : public Expression {
    const Symbol *_id;
    Expressions _args;

public:
    Call(const Symbol *id, const Expressions &args, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* A field reference: expr . id */

class Field : public Expression {
    Expression *_expr;
    Symbol *_id;

public:
    Field(Expression *expr, Symbol *id, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* A logical negation expression: ! expr */

class Not : public Unary {
public:
    Not(Expression *expr, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* An arithmetic negation expression: - expr */

class Negate : public Unary {
public:
    Negate(Expression *expr, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* A dereference expression: * expr */

class Dereference : public Unary {
public:
    Dereference(Expression *expr, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* An address expression: & expr */

class Address : public Unary {
public:
    Address(Expression *expr, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* A cast expression: (type) expr */

class Cast : public Unary {
public:
    Cast(Expression *expr, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* A multiply expression: left * right */

class Multiply : public Binary {
public:
    Multiply(Expression *left, Expression *right, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* A divide expression: left / right */

class Divide : public Binary {
public:
    Divide(Expression *left, Expression *right, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* A remainder expression: left % right */

class Remainder : public Binary {
public:
    Remainder(Expression *left, Expression *right, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* An addition expression: left + right */

class Add : public Binary {
public:
    Add(Expression *left, Expression *right, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* A subtraction expression: left - right */

class Subtract : public Binary {
public:
    Subtract(Expression *left, Expression *right, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* A less-than expression: left < right */

class LessThan : public Binary {
public:
    LessThan(Expression *left, Expression *right, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void test(const Label& label, bool onTrue);
};


/* A greater-than expression: left > right */

class GreaterThan : public Binary {
public:
    GreaterThan(Expression *left, Expression *right, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void test(const Label& label, bool onTrue);
};


/* A less-than-or-equal expression: left <= right */

class LessOrEqual : public Binary {
public:
    LessOrEqual(Expression *left, Expression *right, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void test(const Label& label, bool onTrue);
};


/* A greater-than-or-equal expression: left >= right */

class GreaterOrEqual : public Binary {
public:
    GreaterOrEqual(Expression *left, Expression *right, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void test(const Label& label, bool onTrue);
};


/* An equality expression: left == right */

class Equal : public Binary {
public:
    Equal(Expression *left, Expression *right, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void test(const Label& label, bool onTrue);
};


/* An inequality expression: left != right */

class NotEqual : public Binary {
public:
    NotEqual(Expression *left, Expression *right, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void test(const Label& label, bool onTrue);
};


/* A logical-and expression: left && right */

class LogicalAnd: public Binary {
public:
    LogicalAnd(Expression *left, Expression *right, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void test(const Label& label, bool onTrue);
};


/* A logical-or expression: left || right */

class LogicalOr : public Binary {
public:
    LogicalOr(Expression *left, Expression *right, const Type &type);
    virtual void write(ostream &ostr) const;
    virtual void test(const Label& label, bool onTrue);
};


/* An assignment statement: left = right */

class Assignment : public Statement {
    Expression *_left, *_right;

public:
    Assignment(Expression *left, Expression *right);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* A return statement: return expr */

class Return : public Statement {
    Expression *_expr;

public:
    Return(Expression *expr);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* A block (compound) statement: { decls stmts } */

class Block : public Statement {
    Scope *_decls;
    Statements _stmts;

public:
    Block(Scope *decls, const Statements &stmts);
    Scope *declarations() const;
    virtual void write(ostream &ostr) const;
    virtual void allocate(int &offset) const;
    virtual void generate();
};


/* A while statement: while ( expr ) stmt */

class While : public Statement {
    Expression *_expr;
    Statement *_stmt;

public:
    While(Expression *expr, Statement *stmt);
    virtual void write(ostream &ostr) const;
    virtual void allocate(int &offset) const;
    virtual void generate();
};


/* An if-then or if-then-else statement: if ( expr ) thenStmt else elseStmt */

class If : public Statement {
    Expression *_expr;
    Statement *_thenStmt, *_elseStmt;

public:
    If(Expression *expr, Statement *thenStmt, Statement *elseStmt);
    virtual void write(ostream &ostr) const;
    virtual void allocate(int &offset) const;
    virtual void generate();
};


/* A simple (expression) statement */

class Simple : public Statement {
    Expression *_expr;

public:
    Simple(Expression *expr);
    virtual void write(ostream &ostr) const;
    virtual void generate();
};


/* A function definition: id() { body } */

class Function : public Node {
    const Symbol *_id;
    Block *_body;

public:
    Function(const Symbol *id, Block *body);
    virtual void write(ostream &ostr) const;
    virtual void allocate(int &offset) const;
    virtual void generate();
};

# endif /* TREE_H */
