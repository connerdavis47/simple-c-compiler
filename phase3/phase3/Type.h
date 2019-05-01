# ifndef TYPE_H
# define TYPE_H

# include <ostream>
# include <string>
# include <vector>

typedef std::vector<class Type> Parameters;

class Type 
{

  int           _specifier;
  unsigned      _indirection;
  unsigned long _length;      // optional: for arrays only
  Parameters*   _parameters;  // optional: for functions only

  enum {

    ARRAY, ERROR, FUNCTION, SIMPLE

  }             _kind;

public:

  Type();
  Type(int specifier, unsigned indirection = 0);
  Type(int specifier, unsigned indirection, unsigned long length);
  Type(int specifier, unsigned indirection, Parameters* parameters);

  int             specifier() const;
  unsigned        indirection() const;
  unsigned long   length() const;
  Parameters*     parameters() const;

  bool isArray() const;
  bool isFunction() const;
  bool isSimple() const;

  bool operator == (const Type& rhs) const;
  bool operator != (const Type& rhs) const;

};

std::ostream& operator << (std::ostream& ostr, const Type& t);

# endif /* TYPE_H */
