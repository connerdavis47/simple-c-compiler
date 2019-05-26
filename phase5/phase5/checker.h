/*
 * File:	checker.h
 *
 * Description:	This file contains the public function declarations for the
 *		semantic checker for Simple C.
 */

# ifndef CHECKER_H
# define CHECKER_H
# include <string>
# include "Scope.h"
# include "Tree.h"

Scope *openScope();
Scope *closeScope();

void openStruct(const std::string &name);
void closeStruct(const std::string &name);
Scope *getFields(const std::string &name);

Symbol *defineFunction(const std::string &name, const Type &type);
Symbol *declareFunction(const std::string &name, const Type &type);
Symbol *declareParameter(const std::string &name, const Type &type);
Symbol *declareVariable(const std::string &name, const Type &type);
Symbol *checkIdentifier(const std::string &name);

Expression *checkCall(Symbol *id, Expressions &args);
Expression *checkArray(Expression *left, Expression *right);
Expression *checkDirectField(Expression *expr, const std::string &id);
Expression *checkIndirectField(Expression *expr, const std::string &id);
Expression *checkNot(Expression *expr);
Expression *checkNegate(Expression *expr);
Expression *checkDereference(Expression *expr);
Expression *checkAddress(Expression *expr);
Expression *checkSizeof(Expression *expr);
Expression *checkCast(const Type &type, Expression *expr);
Expression *checkMultiply(Expression *left, Expression *right);
Expression *checkDivide(Expression *left, Expression *right);
Expression *checkRemainder(Expression *left, Expression *right);
Expression *checkAdd(Expression *left, Expression *right);
Expression *checkSubtract(Expression *left, Expression *right);
Expression *checkLessThan(Expression *left, Expression *right);
Expression *checkGreaterThan(Expression *left, Expression *right);
Expression *checkLessOrEqual(Expression *left, Expression *right);
Expression *checkGreaterOrEqual(Expression *left, Expression *right);
Expression *checkEqual(Expression *left, Expression *right);
Expression *checkNotEqual(Expression *left, Expression *right);
Expression *checkLogicalAnd(Expression *left, Expression *right);
Expression *checkLogicalOr(Expression *left, Expression *right);
Statement *checkAssignment(Expression *left, Expression *right);

void checkReturn(Expression *&expr, const Type &type);
void checkTest(Expression *&expr);

# endif /* CHECKER_H */
