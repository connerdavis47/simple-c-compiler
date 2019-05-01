# ifndef CHECKER_H
# define CHECKER_H

# include "Scope.h"
# include "Symbol.h"

Scope* openScope( );
Scope* closeScope( );

Symbol* defineFunction( const std::string& name, const Type& type );
Symbol* defineStruct( const std::string& name );

Symbol* declareFunction( const std::string& name, const Type& type );
Symbol* declareStruct( const std::string& name, const Type& type, const std::string& structName );
Symbol* declareVariable( const std::string& name, const Type& type );

Symbol* checkIdentifier( const std::string& name );
Symbol* checkStruct( const std::string& name, const std::string& structName );

# endif /* CHECKER_H */
