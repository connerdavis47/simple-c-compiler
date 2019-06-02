/*
 * File:	Register.h
 *
 * Description:	This file contains the class definition for registers on
 *		the Intel 64-bit processor.  Each register has three
 *		operand names depending upon the access size.  Our
 *		assembler syntax refers to these as an 8-bit byte, a 32-bit
 *		long word (since a word is historically 16-bits), and a
 *		64-bit quad word.  By default, the 64-bit quad word name
 *		will be used.
 */

# ifndef REGISTER_H
# define REGISTER_H
# include <string>
# include <ostream>

class Register {
    typedef std::string string;
    string _qword;
    string _lword;
    string _byte;

public:
    class Expression *_node;

    Register(const string &qword, const string &lword, const string &byte);
    const string &name(unsigned size = 0) const;

    const string& as_qword() const;
    const string& as_lword() const;
    const string& as_byte() const;
};

std::ostream &operator <<(std::ostream &ostr, const Register *reg);

# endif /* REGISTER_H */
