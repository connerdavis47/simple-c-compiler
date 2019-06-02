/*
 * File:	Type.cpp
 *
 * Description:	This file contains the member function definitions for
 *		types in Simple C.  A type is either a simple type, an
 *		array type, or a function type.
 *
 *		Note that we simply don't like putting function definitions
 *		in the header file.  The header file is for the interface.
 *		Actually, we prefer opaque pointer types in C where you
 *		don't even get to see what's inside, much less touch it.
 *		But, C++ lets us have value types with access control
 *		instead of just always using pointer types.
 *
 *		Extra functionality:
 *		- equality and inequality operators
 *		- predicate functions such as isArray()
 *		- stream operator
 *		- the error type
 */

# include <cassert>
# include "Type.h"

using namespace std;


/*
 * Function:	Type::Type (constructor)
 *
 * Description:	Initialize this type as an error type.
 */

Type::Type()
    : _specifier("error"), _kind(ERROR)
{
}


/*
 * Function:	Type::Type (constructor)
 *
 * Description:	Initialize this type object as a simple type.
 */

Type::Type(const string &specifier, unsigned indirection)
    : _specifier(specifier), _indirection(indirection), _kind(SIMPLE)
{
}


/*
 * Function:	Type::Type (constructor)
 *
 * Description:	Initialize this type object as an array type.
 */

Type::Type(const string &specifier, unsigned indirection, unsigned long length)
    : _specifier(specifier), _indirection(indirection), _length(length)
{
    _kind = ARRAY;
}


/*
 * Function:	Type::Type (constructor)
 *
 * Description:	Initialize this type object as a function type.
 */

Type::Type(const string &specifier, unsigned indirection, Parameters *parameters)
    : _specifier(specifier), _indirection(indirection), _parameters(parameters)
{
    _kind = FUNCTION;
}


/*
 * Function:	Type::operator ==
 *
 * Description:	Return whether another type is equal to this type.  The
 *		parameter lists are checked for function types, which C++
 *		makes so easy.  (At least, it makes something easy!)
 */

bool Type::operator ==(const Type &rhs) const
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

    if (!_parameters || !rhs._parameters)
	return true;

    return *_parameters == *rhs._parameters;
}


/*
 * Function:	Type::operator !=
 *
 * Description:	Well, what do you think it does?  Why can't the language
 *		generate this function for us?  Because they think we want
 *		it to do something else?  Yeah, like that'd be a good idea.
 */

bool Type::operator !=(const Type &rhs) const
{
    return !operator ==(rhs);
}


/*
 * Function:	Type::isArray
 *
 * Description:	Return whether this type is an array type.
 */

bool Type::isArray() const
{
    return _kind == ARRAY;
}


/*
 * Function:	Type::isError
 *
 * Description:	Return whether this type is an error type.
 */

bool Type::isError() const
{
    return _kind == ERROR;
}


/*
 * Function:	Type::isFunction
 *
 * Description:	Return whether this type is a function type.
 */

bool Type::isFunction() const
{
    return _kind == FUNCTION;
}


/*
 * Function:	Type::isSimple
 *
 * Description:	Return whether this type is a simple type.
 */

bool Type::isSimple() const
{
    return _kind == SIMPLE;
}


/*
 * Function:	Type::specifier (accessor)
 *
 * Description:	Return the specifier of this type.
 */

const string &Type::specifier() const
{
    return _specifier;
}


/*
 * Function:	Type::indirection (accessor)
 *
 * Description:	Return the number of levels of indirection of this type.
 */

unsigned Type::indirection() const
{
    return _indirection;
}


/*
 * Function:	Type::length (accessor)
 *
 * Description:	Return the length of this type, which must be an array
 *		type.  Is there a better way than calling assert?  There
 *		certainly isn't an easier way.
 */

unsigned long Type::length() const
{
    assert(_kind == ARRAY);
    return _length;
}


/*
 * Function:	Type::parameters (accessor)
 *
 * Description:	Return the parameters of this type, which must be a
 *		function type.
 */

Parameters *Type::parameters() const
{
    assert(_kind == FUNCTION);
    return _parameters;
}


/*
 * Function:	Type::isStruct
 *
 * Description:	Return whether this type has a struct specifier.
 */

bool Type::isStruct() const
{
    return _kind != ERROR && _specifier != "int" && _specifier != "long";
}


/*
 * Function:	operator <<
 *
 * Description:	Write a type to the specified output stream.  At least C++
 *		let's us do some cool things.
 */

ostream &operator <<(ostream &ostr, const Type &type)
{
    ostr << type.specifier();

    if (type.indirection() > 0)
	ostr << " " << string(type.indirection(), '*');

    if (type.isArray())
	ostr << "[" << type.length() << "]";

    else if (type.isFunction())
	ostr << "()";

    return ostr;
}


/*
 * Function:	Type::isScalar
 *
 * Description:	Check if this type is a scalar type after any promotion.
 *		In Simple C, a scalar type is either a numeric or pointer
 *		type.
 */

bool Type::isScalar() const
{
    return isNumeric() || isPointer();
}


/*
 * Function:	Type::isNumeric
 *
 * Description:	Check if this type is a numeric type.
 */

bool Type::isNumeric() const
{
    if (_kind != SIMPLE || _indirection > 0)
	return false;

    return _specifier == "int" || _specifier == "long";
}


/*
 * Function:	Type::isPointer
 *
 * Description:	Check is this type is a pointer type after any promotion.
 */

bool Type::isPointer() const
{
    return (_kind == SIMPLE && _indirection > 0) || _kind == ARRAY;
}


/*
 * Function:	Type::isCompatibleWith
 *
 * Description:	Check if this type is compatible with the other given type.
 *		In Simple C, two types are compatible if both are numeric
 *		or if both are identical scalar types.
 */

bool Type::isCompatibleWith(const Type &that) const
{
    if (isNumeric() && that.isNumeric())
	return true;

    return isScalar() && promote() == that.promote();
}


/*
 * Function:	Type::promote
 *
 * Description:	Return the result of performing type promotion on this
 *		type.  In Simple C, an array is promoted to a pointer.
 */

Type Type::promote() const
{
    return _kind == ARRAY ? Type(_specifier, _indirection + 1) : *this;
}


/*
 * Function:	Type::deref
 *
 * Description:	Return the result of dereferencing this type, which must be
 *		a pointer type.
 */

Type Type::deref() const
{
    assert(isPointer());
    return Type(_specifier, _kind == ARRAY ? _indirection : _indirection - 1);
}
