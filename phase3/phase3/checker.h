# ifndef CHECKER_H
# define CHECKER_H

# include "Scope.h"
# include "Symbol.h"

Scope* openScope();
Scope* closeScope();

Symbol* defineFunction(const std::string& name, const Type& type);
Symbol* declareFunction(const std::string& name, const Type& type);

Symbol* declareVariable(const std::string& name, const Type& type);

Symbol* checkIdentifier(const std::string& name);
Symbol* checkFunction(const std::string& name);

# endif /* CHECKER_H */
