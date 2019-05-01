# include <cassert>

# include "Type.h"

using namespace std;

Type::Type( )
  :_kind(ERROR)
  {
  }

Type::Type( int specifier, unsigned indirection ) 
  :_specifier(specifier), _indirection(indirection), _kind(SIMPLE)
  {
  }

Type::Type( int specifier, unsigned indirection, unsigned long length )
  :_specifier(specifier), _indirection(indirection), _length(length), _kind(ARRAY)
  {
  }

Type::Type( int specifier, unsigned indirection, Parameters* parameters )
  :_specifier(specifier), _indirection(indirection), _parameters(parameters), _kind(FUNCTION)
  {
  }

int Type::specifier() const
{
  return _specifier;
}

unsigned Type::indirection() const
{
  return _indirection;
}
  
unsigned long Type::length() const
{
  assert(_kind == ARRAY);

  return _length;
}

Parameters* Type::parameters() const
{
  assert(_kind == FUNCTION);

  return _parameters;
}

bool Type::isArray() const
{
  return _kind == ARRAY;
}
  
bool Type::isFunction() const
{
  return _kind == FUNCTION;
}
  
bool Type::isSimple() const
{
  return _kind == SIMPLE;
}

bool Type::operator == (const Type& rhs) const
{
  if (_kind != rhs._kind)
    return false;

  if (_kind == ERROR)
    return true;

  if (_specifier != rhs._specifier)
    return false;

  if (_indirection != rhs._indirection)
    return false;

  if (_kind == SIMPLE)
    return true;

  if (_kind == ARRAY)
    return _length == rhs._length;

  if (_parameters == nullptr || rhs._parameters == nullptr)
    return true;

  return _parameters == rhs._parameters;
}

bool Type::operator != (const Type& rhs) const
{
  return !operator == (rhs);
}

std::ostream& operator << (std::ostream& ostr, const Type& t)
{
  if (t.isArray())
    ostr << "[array] ";
  else if (t.isFunction())
    ostr << "[func] ";
  else
    ostr << "[simple] ";

  ostr << "specifier: " << t.specifier();
  ostr << " indirection: " << t.indirection();

  if (t.isArray())
    ostr << " length: " << t.length();
  
  return ostr;
}
