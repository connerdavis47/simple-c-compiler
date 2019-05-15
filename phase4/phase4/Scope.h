/*
 * File:	Scope.h
 *
 * Description:	This file contains the class definition for scopes in
 *		Simple C.  A scope consists simply of a list of symbols.
 *		We use a vector rather than a map because we want to keep
 *		the symbols in insertion order, and we expect the number of
 *		symbols inserted to be small.
 *
 *		Each scope has a link to its enclosing scope.  By
 *		convention, a null scope is used if there is no enclosing
 *		scope.  The find function searches only the given scope,
 *		whereas the lookup function searches the given scope and
 *		all enclosing scopes.
 */

# ifndef SCOPE_H
# define SCOPE_H
# include "Symbol.h"
# include <string>
# include <vector>

typedef std::vector<Symbol *> Symbols;

class Scope {
    typedef std::string string;

    Scope *_enclosing;
    Symbols _symbols;

public:
    Scope(Scope *enclosing = nullptr);

    void insert(Symbol *symbol);
    void remove(const string &name);
    Symbol *find(const string &name) const;
    Symbol *lookup(const string &name) const;

    Scope *enclosing() const;
    const Symbols &symbols() const;
};

# endif /* SCOPE_H */
