# include <cassert>

# include "Scope.h"

Scope::Scope(Scope* enclosing)
  : _enclosing(enclosing)
  {
  }

Symbol* Scope::find(const string& name) const
{
  for (unsigned it = 0; it < _symbols.size(); ++it)
    if (name == _symbols[it]->name())
      return _symbols[it];

  return NULL;
}

Symbol* Scope::lookup(const string& name) const
{
  Symbol* symbol;

  if ((symbol = find(name)) != nullptr)
    return symbol;

  return _enclosing != nullptr 
          ? _enclosing->lookup(name) : nullptr;
}

void Scope::insert(Symbol* symbol)
{
  assert(find(symbol->name()) == nullptr);

  _symbols.push_back(symbol);
}

void Scope::remove(const string& name)
{
  for (unsigned it = 0; it < _symbols.size(); ++it)
  {
    if (name == _symbols[it]->name())
    {
      _symbols.erase(_symbols.begin() + it);
      break;
    }
  }
}


Scope* Scope::enclosing() const
{
  return _enclosing;
}

const Symbols& Scope::symbols() const
{
  return _symbols;
}
