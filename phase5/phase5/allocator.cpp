# include <iostream>
# include <math.h>

# include "platform.h"
# include "Tree.h"

using std::cout;
using std::endl;
using std::min;

void Block::allocate(int& offset) const
{
    Symbols symbols = _decls->symbols();

    for (unsigned i = 0; i < symbols.size(); ++i)
    {
        if (symbols[i]->_offset == 0)
        {
            offset -= symbols[i]->type().size();
            symbols[i]->_offset = offset;
        }
    }

    int offsetMark = offset;

    for (unsigned i = 0; i < _stmts.size(); ++i)
    {
        int tmp = offsetMark;
        _stmts[i]->allocate(tmp);
        offset = min(offset, tmp);
    }
}

void Function::allocate(int& offset) const
{
    const Parameters* params = _id->type().parameters();
    const Symbols& symbols = _body->declarations()->symbols();

    for (unsigned i = 0; i < symbols.size(); ++i)
    {
        if (i < 6 || i >= params->size())
        {
            offset -= symbols[i]->type().size();
            symbols[i]->_offset = offset;
        }
        else
            symbols[i]->_offset = STACK_ALIGNMENT + SIZEOF_ARG * (i - 6);        
    }

    _body->allocate(offset);
}
