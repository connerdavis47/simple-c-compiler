# include <iostream>

# include "machine.h"
# include "Label.h"

using std::ostream;

unsigned Label::_counter = 0;

Label::Label()
{
    _number = _counter ++;
}

unsigned Label::number() const
{
    return _number;
}

ostream& operator << (ostream& ostr, const Label& label)
{
    return ostr << label_prefix << label.number();
}
