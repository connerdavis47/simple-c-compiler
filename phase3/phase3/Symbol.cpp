# include "Symbol.h"

using std::string;

typedef std::string string;

Symbol::Symbol(const string& name, const Type& type)
  : _name(name), _type(type), _defined(false)
{
}

const string& Symbol::name() const
{
  return _name;
}

const Type& Symbol::type() const
{
  return _type;
}
