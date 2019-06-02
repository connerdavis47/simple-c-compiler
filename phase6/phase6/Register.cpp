/*
 * File:	Register.cpp
 *
 * Description:	This file contains the member function definitions for
 *		registers on the Intel 64-bit processor.
 */

# include "Tree.h"
# include "Register.h"

using namespace std;


/*
 * Function:	Register::Register (constructor)
 *
 * Description:	Initialize this register with its correct operand names.
 */

Register::Register(const string &qword, const string &lword, const string &byte)
    : _qword(qword), _lword(lword), _byte(byte), _node(nullptr)
{
}


/*
 * Function:	Register::name
 *
 * Description:	Return the correct operand name given an access size.  The
 *		default is to return the 64-bit operand name.
 */

const string &Register::name(unsigned size) const
{
    return size == 1 ? _byte : (size == 4 ? _lword : _qword);
}


const string& Register::as_qword() const
{
    return _qword;
}


const string& Register::as_lword() const
{
    return _lword;
}
    

const string& Register::as_byte() const
{
    return _byte;
}


/*
 * Function:	operator <<
 *
 * Description:	Write a register to a stream.  The operand name is
 *		determined by the type of the associated expression if
 *		present.  Otherwise, the default name will be used.
 */

ostream &operator <<(ostream &ostr, const Register *reg)
{
    if (reg->_node != nullptr)
	return ostr << reg->name(reg->_node->type().size());

    return ostr << reg->name();
}
