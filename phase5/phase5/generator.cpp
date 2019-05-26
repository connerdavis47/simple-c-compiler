# include "generator.h"
# include "platform.h"

# include <iostream>
# include <sstream>

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::stringstream;

static unsigned num_args;

ostream& operator << (ostream& ostr, Expression* expr)
{
    return ostr << expr->_operand;
}

void Identifier::generate()
{
    stringstream ss;

    if (_symbol->_offset != 0)
        ss << _symbol->_offset << "(%rbp)";

    else
        ss << global_prefix << _symbol->name();

    _operand = ss.str();
}

void Number::generate()
{
    stringstream ss;

    ss << "$" << _value;

    _operand = ss.str();
}

# if defined (__linux__)

void Call::generate()
{
    unsigned num_bytes = 0;

    for (int i = _args.size() - 1; i >= 0; --i)
    {
        _args[i]->generate();
        cout << "\tpushq\t" << _args[i] << endl;
        num_bytes += _args[i]->type().size();
    }

    cout << "\tcall\t" << _id->name() << endl;

    if (num_bytes > 0)
        cout << "\taddq\t$" << num_bytes << ", %rsp" << endl;
}

# elif defined (__APPLE__)

void Call::generate()
{
    if (_args.size() > num_args)
        num_args = _args.size();

    for (int i = _args.size() - 1; i >= 0; --i)
    {
        _args[i]->generate();

        cout << "\tmovq\t" << _args[i] << ", %rax" << endl;
        cout << "\tmovq\t%rax, " << (i * SIZEOF_ARG) << "(%rsp)" << endl;
    }

    cout << "\tcall\t" << global_prefix << _id->name() << endl;
}

# endif

void Assignment::generate()
{
    _left->generate();
    _right->generate();

    cout << "\tmovl\t" << _right << ", %rax" << endl;
    cout << "\tmovl\t%rax, " << _left << endl;
}

void Block::generate()
{
    for (unsigned i = 0; i < _stmts.size(); ++i)
        _stmts[i]->generate();
}

void Function::generate()
{
    int offset = 0;

    allocate(offset);

    cout << global_prefix << _id->name() << ":" << endl;
    cout << "\tpushq\t%rbp" << endl;
    cout << "\tmovq\t%rsp, %rbp" << endl;
    cout << "\tsubq\t$" << _id->name() << ".size, %rsp" << endl;

    num_args = 0;
    _body->generate();

    offset -= num_args * SIZEOF_ARG;
    while ((offset - ARG_OFFSET) % STACK_ALIGNMENT)
        --offset;

    cout << "\tmovq\t%rbp, %rsp" << endl;
    cout << "\tpopq\t%rbp" << endl;
    cout << "\tret" << endl << endl;
    
    cout << "\t.globl\t" << global_prefix << _id->name() << endl;
    cout << "\t.set\t" << _id->name() << ".size, " << -offset << endl << endl;
}

void generate(const Symbols& symbols)
{
    if (symbols.size() > 0)
        cout << "\t.data" << endl;

    for (unsigned i = 0; i < symbols.size(); ++i)
    {
        cout << "\t.comm\t" << global_prefix << symbols[i]->name();
        cout << ", " << symbols[i]->type().size();
        cout << ", " << symbols[i]->type().alignment() << endl;
    }
}
