/*
 * File:	allocator.cpp
 *
 * Description:	This file contains the member function definitions for
 *		functions dealing with storage allocation.  The actual
 *		classes are declared elsewhere, mainly in Tree.h.
 *
 *		Extra functionality:
 *		- maintaining minimum offset in nested blocks
 *		- allocation within while and if-then-else statements
 */

# include <map>
# include <cassert>
# include <iostream>

# include "checker.h"
# include "machine.h"
# include "tokens.h"
# include "Tree.h"

using namespace std;

static map<string, unsigned long> sizes;

/*
 * Function:	Type::size
 *
 * Description:	Return the size of a type in bytes.
 */

unsigned long Type::size() const
{
    assert(_kind != FUNCTION && _kind != ERROR);

    unsigned long count = (_kind == ARRAY ? _length : 1);

    if (_indirection > 0)
	    return count * SIZEOF_PTR;

    if (_specifier == "long")
	    return count * SIZEOF_LONG;

    if (_specifier == "int")
	    return count * SIZEOF_INT;

    if (sizes.count(_specifier) > 0)
	    return count * sizes[_specifier];

    /* The size of a structure is the size of all of its fields, but with
       each field aligned and the entire structure aligned as well.  Since
       this is rather expensive to compute, we cache the result. */

    unsigned align, size = 0;
    const Symbols& symbols = getFields(_specifier)->symbols();

    for (unsigned i = 0; i < symbols.size(); i ++) 
    {
        align = symbols[i]->type().alignment();

        if (size % align != 0)
            size += (align - size % align);

        symbols[i]->_offset = size;
        size += symbols[i]->type().size();
    }

    align = alignment();

    if (size % align != 0)
	    size += (align - size % align);

    sizes[_specifier] = size;
    return size;
}


/*
 * Function:	Type::alignment
 *
 * Description:	Return the alignment of a type in bytes.
 */

unsigned Type::alignment() const
{
    assert(_kind != FUNCTION && _kind != ERROR);

    if (_indirection > 0 || _specifier == "char")
	    return ALIGNOF_PTR;

    if (_specifier == "long")
	    return ALIGNOF_LONG;

    if (_specifier == "int")
	    return ALIGNOF_INT;

    /* The alignment of a structure is the maximum alignment of its fields. */

    unsigned align = 0;
    const Symbols& symbols = getFields(_specifier)->symbols();

    for (unsigned i = 0; i < symbols.size(); ++ i)
	    if (symbols[i]->type().alignment() > align)
	        align = symbols[i]->type().alignment();

    return align;
}


/*
 * Function:	Block::allocate
 *
 * Description:	Allocate storage for this block.  We assign decreasing
 *		offsets for all symbols declared within this block, and
 *		then for all symbols declared within any nested block.
 *		Only symbols that have not already been allocated an offset
 *		will be assigned one, since some parameters are already
 *		assigned special offsets.
 */

void Block::allocate(int& offset) const
{
    int temp, saved;
    unsigned i;
    Symbols symbols;

    symbols = _decls->symbols();

    for (i = 0; i < symbols.size(); ++ i)
        if (symbols[i]->_offset == 0) 
        {
            offset -= symbols[i]->type().size();
            symbols[i]->_offset = offset;
        }

    saved = offset;

    for (i = 0; i < _stmts.size(); ++ i) 
    {
        temp = saved;
        _stmts[i]->allocate(temp);
        offset = min(offset, temp);
    }
}


/*
 * Function:	While::allocate
 *
 * Description:	Allocate storage for this while statement, which
 *		essentially means allocating storage for variables declared
 *		as part of its statement.
 */

void While::allocate(int& offset) const
{
    _stmt->allocate(offset);
}


/*
 * Function:	If::allocate
 *
 * Description:	Allocate storage for this if-then or if-then-else
 *		statement, which essentially means allocating storage for
 *		variables declared as part of its statements.
 */

void If::allocate(int& offset) const
{
    int saved, temp;

    saved = offset;
    _thenStmt->allocate(offset);

    if (_elseStmt != nullptr) 
    {
        temp = saved;
        _elseStmt->allocate(temp);
        offset = min(offset, temp);
    }
}


/*
 * Function:	Function::allocate
 *
 * Description:	Allocate storage for this function and return the number of
 *		bytes required.  The parameters are allocated offsets as
 *		well, starting with the given offset.  This function is
 *		designed to work with both 32-bit and 64-bit Intel
 *		architectures, with or without callee-saved registers.
 *
 *		32-bit Intel/Linux:
 *		  SIZEOF_PARAM = 0 (each parameter has its own size)
 *		  NUM_PARAM_REGS = 0 (all parameters are on the stack)
 *
 *		64-bit Intel/Linux:
 *		  SIZEOF_PARAM = 8 (each parameter is always eight bytes)
 *		  NUM_PARAM_REGS = 6 (first six parameters are in registers)
 */

void Function::allocate(int& offset) const
{
    Parameters *params;
    Symbols symbols;

    params = _id->type().parameters();
    symbols = _body->declarations()->symbols();

    for (unsigned i = NUM_PARAM_REGS; i < params->size(); ++ i) 
    {
        symbols[i]->_offset = offset;
        offset += (SIZEOF_PARAM ? SIZEOF_PARAM : (*params)[i].promote().size());
    }

    offset = 0;

    for (unsigned i = 0; i < NUM_PARAM_REGS; ++ i)
        if (i < params->size()) 
        {
            offset -= (*params)[i].promote().size();
            symbols[i]->_offset = offset;
        }
        else
            break;

    _body->allocate(offset);
}
