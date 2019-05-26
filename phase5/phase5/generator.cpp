# include "generator.h"
# include "platform.h"

# include <iostream>
# include <list>
# include <map>
# include <set>
# include <sstream>
# include <queue>

using std::cerr;
using std::cout;
using std::endl;
using std::list;
using std::map;
using std::ostream;
using std::string;
using std::stringstream;
using std::queue;

static unsigned num_args;
static queue<string> chunks;
static const string registers[] = {
    "e15d", "e14d", "e13d", "e12d", "e11d", "e10"
};
static const string call_registers[] = {
    "edi", "esi", "edx", "ecx", "e8d", "e9d"
};

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

    for (unsigned i = 0; i < _args.size(); ++i)
    {
        _args[i]->generate();
        ss << "\tmovl\t" << _args[i]->_text << ", %" << call_registers[i] << endl;
    }

    ss << "\tmovl\t$0, %eax" << endl;
    ss << "\tcall\t" << _id->name() << endl;

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

    int offset = 0, tmp_offset = 0;

    allocate(offset);

    num_args = 0;
    tmp_offset = offset;
    _body->generate();
    offset = tmp_offset;

    offset -= num_args * SIZEOF_ARG;
    while ((offset - ARG_OFFSET) % STACK_ALIGNMENT)
        --offset;

    ss << _id->name() << ":" << endl;
    ss << "\tpushq\t%rbp" << endl;
    ss << "\tmovq\t%rsp, %rbp" << endl;

    if (offset != 0)
        ss << "\tsubq\t$" << _id->name() << ".size, %rsp" << endl;

    ss << _body->_text;

    ss << "\tmovl\t$0, %eax" << endl;
    ss << "\tleave" << endl;
    ss << "\tret" << endl << endl;
    
    if (offset != 0)
        ss << "\t.set\t" << _id->name() << ".size, " << -offset;

    chunks.push(ss.str());
}

void generate(const Symbols& symbols)
{
    cout << "\t.text" << endl;

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
            cout << "," << symbols[i]->type().size() << endl;
        }
    }

    while (chunks.size() > 0)
    {
        cout << chunks.front() << endl;
        chunks.pop();
    }
}
