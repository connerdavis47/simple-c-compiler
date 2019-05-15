/*
 * File:	Scope.cpp
 *
 * Description:	This file contains the member function definitions for
 *		scopes in Simple C.
 *
 *		We didn't allocate the symbols we're given, so we don't
 *		deallocate them.  That's the rule.  You have to do that
 *		yourself.  Besides, it's possible that they're hanging
 *		around other places, like abstract syntax trees.
 *
 *		Extra functionality:
 *		- retrieving the vector of symbols
 */

# include <cassert>
# include "Scope.h"


/*
 * Function:	Scope::Scope (constructor)
 *
 * Description:	Initialize this scope object.
 */

Scope::Scope(Scope *enclosing)
    : _enclosing(enclosing)
{
}


/*
 * Function:	Scope::insert
 *
 * Description:	Insert the given symbol into this scope.  It had better not
 *		already be inserted, or we fail big time.
 */


void Scope::insert(Symbol *symbol)
{
    assert(find(symbol->name()) == nullptr);
    _symbols.push_back(symbol);
}


/*
 * Function:	Scope::find
 *
 * Description:	Find and return the symbol with the given name in this
 *		scope.  If no such symbol is found, return a null pointer.
 */

Symbol *Scope::find(const string &name) const
{
    for (unsigned i = 0; i < _symbols.size(); i ++)
	if (name == _symbols[i]->name())
	    return _symbols[i];

    return nullptr;
}


/*
 * Function:	Scope::remove
 *
 * Description:	Remove the symbol with the given name from this scope.
 *		Yes, I know, I duplicated the search logic from above.
 *		And, yes, I still didn't use an iterator.  So sue me.
 */

void Scope::remove(const string &name)
{
    for (unsigned i = 0; i < _symbols.size(); i ++)
	if (name == _symbols[i]->name())
	    _symbols.erase(_symbols.begin() + i);
}


/*
 * Function:	Scope::lookup
 *
 * Description:	Find and return the nearest symbol with the given name,
 *		starting the search in the given scope and moving into the
 *		enclosing scopes.  If no such symbol is found, return a
 *		null pointer.
 */

Symbol *Scope::lookup(const string &name) const
{
    Symbol *symbol;


    if ((symbol = find(name)) != nullptr)
	return symbol;

    return _enclosing != nullptr ? _enclosing->lookup(name) : nullptr;
}


/*
 * Function:	Scope::enclosing (accessor)
 *
 * Description:	Return the enclosing scope of this scope.
 */

Scope *Scope::enclosing() const
{
    return _enclosing;
}


/*
 * Function:	Scope::symbols (accessor)
 *
 * Description:	Return the list of symbols in this scope.
 */

const Symbols &Scope::symbols() const
{
    return _symbols;
}
