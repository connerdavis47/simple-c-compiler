# include "generator.h"
# include "platform.h"

# include <iostream>
# include <sstream>
# include <queue>

using std::cerr;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::stringstream;
using std::queue;

static unsigned num_args;
static queue<string> chunks;

void Simple::generate()
{
    _expr->generate();

    _text = _expr->_text;
}

void Identifier::generate()
{
    stringstream ss;

    if (_symbol->_offset != 0)
        ss << _symbol->_offset << "(%rbp)";

    else
        ss << _symbol->name() << "(%rip)";

    _text = ss.str();
}

void Number::generate()
{
    stringstream ss;

    ss << "$" << _value;

    _text = ss.str();
}

void Call::generate()
{
    stringstream ss;

    unsigned num_bytes = 0;

    for (int i = _args.size() - 1; i >= 0; --i)
    {
        _args[i]->generate();
        ss << "\tpmovl\t" << _args[i]->_text<< endl;
        num_bytes += _args[i]->type().size();
    }

    ss << "\tcall\t" << _id->name() << endl;

    if (num_bytes > 0)
        ss << "\taddq\t$" << num_bytes << ", %rsp" << endl;

    _text = ss.str();
}

void Assignment::generate()
{
    stringstream ss;

    _left->generate();
    _right->generate();

    ss << "\tmovl\t" << _right->_text << ", " << _left->_text << endl;

    _text = ss.str();
}

void Block::generate()
{
    stringstream ss;
    
    for (unsigned i = 0; i < _stmts.size(); ++i)
    {
        _stmts[i]->generate();
        ss << _stmts[i]->_text;
    }

    _text = ss.str();
}

void Function::generate()
{
    stringstream ss;

    int offset = 0;

    allocate(offset);

    num_args = 0;
    _body->generate();

    offset -= num_args * SIZEOF_ARG;
    while ((offset - ARG_OFFSET) % STACK_ALIGNMENT)
        --offset;

    ss << _id->name() << ":" << endl;
    ss << "\tpushq\t%rbp" << endl;
    ss << "\tmovq\t%rsp, %rbp" << endl;

    if (offset != 0)
        ss << "\tsubq\t$" << _id->name() << ".size, %rsp" << endl;

    ss << _body->_text;

    ss << "\tpopq\t%rbp" << endl;
    ss << "\tret" << endl << endl;
    
    if (offset != 0)
        ss << "\t.set\t" << _id->name() << ".size, " << -offset;

    chunks.push(ss.str());
}

void generate(const Symbols& symbols)
{
    for (unsigned i = 0; i < symbols.size(); ++i)
    {
        if (symbols[i]->type().isFunction())
        {
            cout << "\t.globl\t" << symbols[i]->name() << endl;
            cout << "\t.type\t" << symbols[i]->name() << ", @function" << endl;
        }
        else
        {
            cout << "\t.comm\t" << symbols[i]->name();
            cout << "," << symbols[i]->type().size();
            cout << "," << symbols[i]->type().alignment() << endl;
        }
    }

    while (chunks.size() > 0)
    {
        cout << chunks.front() << endl;
        chunks.pop();
    }
}
