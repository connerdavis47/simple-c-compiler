# include "math.h"

# include "Tree.h"

using std::min;

void Block::allocate(int& offset) const
{
    Symbols symbols = _decls->symbols();

    int offsetMark;

    for (unsigned i = 0; i < symbols.size(); ++i)
    {
        if (symbols[i]->_offset == 0)
        {
            offset -= symbols[i]->type().size();
            symbols[i]->_offset = offset;
        }
    }

    offsetMark = offset;

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
    Symbols symbols = _body->declarations()->symbols();

    offset = 8;

    for (unsigned i = 0; i < params->size(); ++i)
    {
        symbols[i]->_offset = offset;
        offset += 8;
    }

    offset = 0;
    _body->allocate(offset);
}
