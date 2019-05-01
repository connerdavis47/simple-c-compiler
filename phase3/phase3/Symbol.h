# ifndef SYMBOL_H
# define SYMBOL_H

# include <string>

# include "Type.h"

class Symbol
{

  typedef std::string string;

  string  _name;
  Type    _type;

public:

  bool _defined;

  Symbol(const string& name, const Type& type);

  const string& name() const;
  const Type&   type() const;

};

# endif /* SYMBOL_H */
