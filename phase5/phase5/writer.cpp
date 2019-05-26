/*
 * File:	writer.cpp
 *
 * Description:	This file contains the member function definitions for
 *		writing the abstract syntax tree to an output stream.  The
 *		tree is written in a LISP-like syntax, but C-style
 *		operators are used.
 *
 *		This functionality has no end purpose in the actual
 *		compiler.  However, it is useful in understanding the
 *		structure of the abstract syntax tree, and is also a
 *		refresher on how to write and use polymorphic virtual
 *		functions in C++.
 */

# include "Tree.h"

using namespace std;


/*
 * Function:	operator << (private)
 *
 * Description:	Convenience function for printing a tree node using the
 *		output stream operator.  We did not make this function
 *		publicly available as someone else (i.e., the code
 *		generator) might with to overload the operator to do
 *		something else and we don't want ours to get in the way.
 */

static ostream &operator <<(ostream &ostr, const Node *node)
{
    node->write(ostr);
    return ostr;
}


/*
 * From this point on are the member functions for printing the tree, one
 * for each type of tree node that can be instantiated.  If you really,
 * really want a function comment header for each one, write them yourself.
 */

void String::write(ostream &ostr) const
{
    ostr << _value;
}

void Identifier::write(ostream &ostr) const
{
    ostr << _symbol->name();
}

void Number::write(ostream &ostr) const
{
    ostr << _value << (_type.specifier() == "long" ? "L" : "");
}

void Call::write(ostream &ostr) const
{
    ostr << "(" << _id->name();

    for (unsigned i = 0; i < _args.size(); i ++)
	ostr << " " << _args[i];

    ostr << ")";
}

void Field::write(ostream &ostr) const
{
    ostr << "(. " << _expr << " " << _id->name() << ")";
}

void Not::write(ostream &ostr) const
{
    ostr << "(! " << _expr << ")";
}

void Negate::write(ostream &ostr) const
{
    ostr << "(- " << _expr << ")";
}

void Dereference::write(ostream &ostr) const
{
    ostr << "(* " << _expr << ")";
}

void Address::write(ostream &ostr) const
{
    ostr << "(& " << _expr << ")";
}

void Cast::write(ostream &ostr) const
{
    ostr << "(" << _type << " " << _expr << ")";
}

void Multiply::write(ostream &ostr) const
{
    ostr << "(* " << _left << " " << _right << ")";
}

void Divide::write(ostream &ostr) const
{
    ostr << "(/ " << _left << " " << _right << ")";
}

void Remainder::write(ostream &ostr) const
{
    ostr << "(% " << _left << " " << _right << ")";
}

void Add::write(ostream &ostr) const
{
    ostr << "(+ " << _left << " " << _right << ")";
}

void Subtract::write(ostream &ostr) const
{
    ostr << "(- " << _left << " " << _right << ")";
}

void LessThan::write(ostream &ostr) const
{
    ostr << "(< " << _left << " " << _right << ")";
}

void GreaterThan::write(ostream &ostr) const
{
    ostr << "(> " << _left << " " << _right << ")";
}

void LessOrEqual::write(ostream &ostr) const
{
    ostr << "(<= " << _left << " " << _right << ")";
}

void GreaterOrEqual::write(ostream &ostr) const
{
    ostr << "(>= " << _left << " " << _right << ")";
}

void Equal::write(ostream &ostr) const
{
    ostr << "(== " << _left << " " << _right << ")";
}

void NotEqual::write(ostream &ostr) const
{
    ostr << "(!= " << _left << " " << _right << ")";
}

void LogicalAnd::write(ostream &ostr) const
{
    ostr << "(&& " << _left << " " << _right << ")";
}

void LogicalOr::write(ostream &ostr) const
{
    ostr << "(|| " << _left << " " << _right << ")";
}

void Assignment::write(ostream &ostr) const
{
    ostr << "(= " << _left << " " << _right << ")";
}

void Return::write(ostream &ostr) const
{
    ostr << "(return " << _expr << ")";
}

void Block::write(ostream &ostr) const
{
    ostr << "(begin";

    for (unsigned i = 0; i < _stmts.size(); i ++)
	ostr << " " << _stmts[i];

    ostr << ")";
}

void While::write(ostream &ostr) const
{
    ostr << "(while " << _expr << " " << _stmt << ")";
}

void If::write(ostream &ostr) const
{
    ostr << "(if " << _expr << " " << _thenStmt;

    if (_elseStmt != nullptr)
	ostr << " " << _elseStmt;

    ostr << ")";
}

void Simple::write(ostream &ostr) const
{
    ostr << _expr;
}

void Function::write(ostream &ostr) const
{
    unsigned num = _id->type().parameters()->size();
    const Symbols &symbols = _body->declarations()->symbols();


    ostr << "(define " << (num > 0 ? "(" : "") << _id->name();

    for (unsigned i = 0; i < num; i ++)
	ostr << " " << symbols[i]->name();

    ostr << (num > 0 ? ") " : " ") << _body << ")";
}
