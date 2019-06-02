/*
 * File:	Type.h
 *
 * Description:	This file contains the class definition for types in Simple
 *		C.  A type is either a simple type, an array type, or a
 *		function type.  These types include a specifier and the
 *		number of levels of indirection.  Array types also have a
 *		length, and function types also have a parameter list.
 *		An error type is also supported for use in undeclared
 *		identifiers and the results of type checking.
 *
 *		By convention, a null parameter list represents an
 *		unspecified parameter list.  An empty parameter list is
 *		represented by an empty vector.
 *
 *		No subclassing is used to avoid the problem of object
 *		slicing (since we'll be treating types as value types) and
 *		the proliferation of small member functions.
 *
 *		As we've designed them, types are essentially immutable,
 *		since we haven't included any mutators.  In practice, we'll
 *		be creating new types rather than changing existing types.
 */

# ifndef TYPE_H
# define TYPE_H
# include <vector>
# include <string>
# include <ostream>

typedef std::vector<class Type> Parameters;

class Type {
    typedef std::string string;

    string _specifier;
    unsigned _indirection;
    unsigned long _length;
    Parameters *_parameters;

    enum { ARRAY, ERROR, FUNCTION, SIMPLE } _kind;

public:
    Type();
    Type(const string &specifier, unsigned indirection = 0);
    Type(const string &specifier, unsigned indirection, unsigned long length);
    Type(const string &specifier, unsigned indirection, Parameters *parameters);

    bool operator ==(const Type &rhs) const;
    bool operator !=(const Type &rhs) const;

    bool isArray() const;
    bool isError() const;
    bool isFunction() const;
    bool isSimple() const;

    const string &specifier() const;
    unsigned indirection() const;
    unsigned long length() const;
    Parameters *parameters() const;

    bool isStruct() const;
    bool isScalar() const;
    bool isNumeric() const;
    bool isPointer() const;
    bool isCompatibleWith(const Type &that) const;

    Type promote() const;
    Type deref() const;

    unsigned long size() const;
    unsigned alignment() const;
};

std::ostream &operator <<(std::ostream &ostr, const Type &type);

# endif /* TYPE_H */
