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

Scope* openScope( );
Scope* closeScope( );

void openStruct( const std::string& name );
void closeStruct( const std::string& name );

Symbol* defineFunction( const std::string& name, const Type& type );
Symbol* declareFunction( const std::string& name, const Type& type );
Symbol* declareParameter( const std::string& name, const Type& type );
Symbol* declareVariable( const std::string& name, const Type& type );
Symbol* checkIdentifier( const std::string& name );

Type checkReturn( const Type& expr );
Type checkWhileLoop( const Type& expr );
Type checkIfElse( const Type& expr );
Type checkAssignment( const Type& left, const Type& right );

Type checkLogical( const Type& left, const Type& right, const std::string& op );
Type checkLogicalAnd( const Type& left, const Type& right );
Type checkLogicalOr( const Type& left, const Type& right );

Type checkEquality( const Type& left, const Type& right, const std::string& op );
Type checkEqual( const Type& left, const Type& right );
Type checkNotEqual( const Type& left, const Type& right );

Type checkRelational( const Type& left, const Type& right, const std::string& op );
Type checkLessOrEqual( const Type& left, const Type& right );
Type checkGreaterOrEqual( const Type& left, const Type& right );
Type checkLessThan( const Type& left, const Type& right );
Type checkGreaterThan( const Type& left, const Type& right );

Type checkAdditive( const Type& left, const Type& right, const std::string& op );
Type checkAdd( const Type& left, const Type& right );
Type checkSubtract( const Type& left, const Type& right );

Type checkMultiplicative( const Type& left, const Type& right, const std::string& op );
Type checkMultiply( const Type& left, const Type& right );
Type checkDivide( const Type& left, const Type& right );
Type checkRemainder( const Type& left, const Type& right );

Type checkNegate( const Type& operand );
Type checkNot( const Type& operand );
Type checkAddress( const Type& operand );
Type checkDereference( const Type& operand );
Type checkSizeof( const Type& operand );

Type checkArray( const Type& left, const Type& right );
Type checkStructField( const Type& left, const Type& right );
Type checkStructPointerField( const Type& left, const Type& right );

# endif /* CHECKER_H */
