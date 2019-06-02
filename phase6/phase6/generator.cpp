/*
 * File:	generator.cpp
 *
 * Description:	This file contains the public and member function
 *		definitions for the code generator for Simple C.
 *
 *		Extra functionality:
 *		- putting all the global declarations at the end
 */

# include <cassert>
# include <iostream>
# include <sstream>
# include <vector>

# include "generator.h"
# include "Label.h"
# include "machine.h"
# include "Register.h"
# include "Tree.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::stringstream;
using std::vector;

static int offset, temp_offset;
static string funcname;
static Label* return_label; 
static vector<string> strings;


# define DEBUG_MODE 1


/* This needs to be zero for the next phase. */

# define SIMPLE_PROLOGUE 0


/* This shouid be set if we want to use the callee-saved registers. */

# define CALLEE_SAVED 0


/* The registers and their related functions */

typedef vector<Register *> Registers;

static Register* rax    = new Register("%rax", "%eax", "%al");
static Register* rbx    = new Register("%rbx", "%ebx", "%bl");
static Register* rcx    = new Register("%rcx", "%ecx", "%cl");
static Register* rdx    = new Register("%rdx", "%edx", "%dl");
static Register* rsi    = new Register("%rsi", "%esi", "%sil");
static Register* rdi    = new Register("%rdi", "%edi", "%dil");
static Register* r8     = new Register("%r8", "%r8d", "%r8b");
static Register* r9     = new Register("%r9", "%r9d", "%r9b");
static Register* r10    = new Register("%r10", "%r10d", "%r10b");
static Register* r11    = new Register("%r11", "%r11d", "%r11b");
static Register* r12    = new Register("%r12", "%r12d", "%r12b");
static Register* r13    = new Register("%r13", "%r13d", "%r13b");
static Register* r14    = new Register("%r14", "%r14d", "%r14b");
static Register* r15    = new Register("%r15", "%r15d", "%r15b");

static Registers registers;
static Registers parameters = { rdi, rsi, rdx, rcx, r8, r9 };
static Registers caller_saved = { r11, r10, r9, r8, rcx, rdx, rsi, rdi, rax };

# if CALLEE_SAVED
static Registers callee_saved = { rbx, r12, r13, r14, r15 };
# else
static Registers callee_saved = { };
# endif

static void assign(Expression* expr, Register* reg);
static void load(Expression* expr, Register* reg);

Register* get_reg()
{
    for (unsigned i = 0; i < registers.size(); ++ i)
        if (registers[i]->_node == nullptr)
            return registers[i];

    load(nullptr, registers[0]);
    return registers[0];
}


static void debug_open(string id)
{
    if (DEBUG_MODE)
        cout << "\t# === " << id << endl;
}


static void debug_close(string id)
{
    if (DEBUG_MODE)
        cout << "\t# --- " << id << endl;
}


/*
 * Function:	suffix (private)
 *
 * Description:	Return the suffix for an opcode based on the given size.
 */

static string suffix(unsigned long size)
{
    return size == 1 ? "b\t" : (size == 4 ? "l\t" : "q\t");
}


/*
 * Function:	suffix (private)
 *
 * Description:	Return the suffix for an opcode based on the size of the
 *		given expression.
 */

static string suffix(Expression* expr)
{
    return suffix(expr->type().size());
}


/*
 * Function:	align (private)
 *
 * Description:	Return the number of bytes necessary to align the given
 *		offset on the stack.
 */

static int align(int offset)
{
    if (offset % STACK_ALIGNMENT == 0)
	    return 0;

    return STACK_ALIGNMENT - (abs(offset) % STACK_ALIGNMENT);
}


/*
 * Function:	operator << (private)
 *
 * Description:	Write an expression as an operand to the specified stream.
 *		This function first checks to see if the expression is in a
 *		register, and if not then uses its offset.
 */

static ostream& operator << (ostream& ostr, Expression* expr)
{
    if (expr->_register != nullptr)
    {
        unsigned size = expr->type().size();

        ostr << expr->_register->name(size);
    }
    else
        expr->operand(ostr);

    return ostr;
}


/*
 * Function:	Expression::operand
 *
 * Description:	Write an expression as an operand to the specified stream.
 */

void Expression::operand(ostream& ostr) const
{
    ostr << _offset << "(%rbp)";
}


/*
 * Function:	Identifier::operand
 *
 * Description:	Write an identifier as an operand to the specified stream.
 */

void Identifier::operand(ostream& ostr) const
{
    if (_symbol->_offset == 0)
	    ostr << global_prefix << _symbol->name() << global_suffix;

    else
	    ostr << _symbol->_offset << "(%rbp)";
}


/*
 * Function:	Number::operand
 *
 * Description:	Write a number as an operand to the specified stream.
 */

void Number::operand(ostream& ostr) const
{
    ostr << "$" << _value;
}


void String::operand(ostream& ostr) const
{
    Label label;

    ostr << label << global_suffix;

    stringstream ss;
    ss << label << ":\n\t.string " << _value << endl;
    strings.push_back(ss.str());
}


void Expression::test(const Label& label, bool ifTrue)
{
    generate();

    if (_register == nullptr)
        load(this, get_reg());

    cout << "\tcmp" << suffix(this) << "$0, " << this << endl;
    cout << (ifTrue ? "\tjne\t" : "\tje\t") << label << endl;

    assign(this, nullptr);
}


void LessThan::test(const Label& label, bool onTrue)
{
    _left->generate();
    _right->generate();

    if (_left->_register == nullptr)
        load(_left, get_reg());

    cout << "\tcmp" << suffix(_left);
    cout << _right << ", " << _left << endl;
    cout << (onTrue ? "\tjl\t" : "\tjge\t") << label << endl;

    assign(_left, nullptr);
    assign(_right, nullptr);
}


void GreaterThan::test(const Label& label, bool onTrue)
{
    _left->generate();
    _right->generate();

    if (_left->_register == nullptr)
        load(_left, get_reg());

    cout << "\tcmp" << suffix(_left);
    cout << _right << ", " << _left << endl;
    cout << (onTrue ? "\tjg\t" : "\tjle\t") << label << endl;

    assign(_left, nullptr);
    assign(_right, nullptr);
}


void LessOrEqual::test(const Label& label, bool onTrue)
{
    _left->generate();
    _right->generate();

    if (_left->_register == nullptr)
        load(_left, get_reg());

    cout << "\tcmp" << suffix(_left);
    cout << _right << ", " << _left << endl;
    cout << (onTrue ? "\tjle\t" : "\tjg\t") << label << endl;

    assign(_left, nullptr);
    assign(_right, nullptr);
}


void GreaterOrEqual::test(const Label& label, bool onTrue)
{
    _left->generate();
    _right->generate();

    if (_left->_register == nullptr)
        load(_left, get_reg());

    cout << "\tcmp" << suffix(_left);
    cout << _right << ", " << _left << endl;
    cout << (onTrue ? "\tjge\t" : "\tjl\t") << label << endl;

    assign(_left, nullptr);
    assign(_right, nullptr);
}


void Equal::test(const Label& label, bool onTrue)
{
    debug_open("EQL");

    _left->generate();
    _right->generate();

    if (_left->_register == nullptr)
        load(_left, get_reg());

    cout << "\tcmp" << suffix(_left);
    cout << _right << ", " << _left << endl;
    cout << (onTrue ? "\tje\t" : "\tjne\t") << label << endl;

    debug_close("EQL");
}


void NotEqual::test(const Label& label, bool onTrue)
{
    _left->generate();
    _right->generate();

    if (_left->_register == nullptr)
        load(_left, get_reg());

    cout << "\tcmp" << suffix(_left);
    cout << _right << ", " << _left << endl;
    cout << (onTrue ? "\tjne\t" : "\tje\t") << label << endl;
}


void LogicalOr::test(const Label& label, bool onTrue)
{
    debug_open("OR");

    Label test;

    _left->test(test, true);
    _right->test(label, false);

    cout << test << ":";

    debug_close("OR");
}


void LogicalAnd::test(const Label& label, bool onTrue)
{

}


/*
 * Function:	Call::generate
 *
 * Description:	Generate code for a function call expression.
 */

void Call::generate()
{
    debug_open("CALL");

    unsigned long value, size, bytesPushed = 0;


    /* Generate any arguments with function calls first. */

    for (int i = _args.size() - 1; i >= 0; -- i)
	    if (_args[i]->_hasCall)
	        _args[i]->generate();


    /* Adjust the stack if necessary. */

    if (_args.size() > NUM_PARAM_REGS) 
    {
        bytesPushed = align((_args.size() - NUM_PARAM_REGS) * SIZEOF_PARAM);

        if (bytesPushed > 0)
            cout << "\tsubq\t$" << bytesPushed << ", %rsp" << endl;
    }


    /* Move the arguments into the correct registers or memory locations. */

    for (int i = _args.size() - 1; i >= 0; -- i) 
    {
        debug_open("ARG");

        size = _args[i]->type().size();

        if (!_args[i]->_hasCall)
            _args[i]->generate();

        if (i < NUM_PARAM_REGS)
            load(_args[i], parameters[i]);

        else 
        {
            bytesPushed += SIZEOF_PARAM;

            if (_args[i]->_register)
                cout << "\tpushq\t" << _args[i]->_register->name() << endl;

            else if (_args[i]->isNumber(value) || size == SIZEOF_PARAM)
                cout << "\tpushq\t" << _args[i] << endl;
            
            else 
            {
                load(_args[i], rax);
                cout << "\tpushq\t%rax" << endl;
            }
        }

        assign(_args[i], nullptr);

        debug_close("ARG");
    }


    /* Spill any caller-saved registers still in use. */

    for (unsigned i = 0; i < caller_saved.size(); ++ i)
	    load(nullptr, caller_saved[i]);


    /* Call the function.  Technically, we only need to assign the number
       of floating point arguments to %eax if the function being called
       takes a variable number of arguments.  But, it never hurts. */

    if (_id->type().parameters() == nullptr)
	    cout << "\tmovl\t$0, %eax" << endl;

    cout << "\tcall\t" << global_prefix << _id->name() << endl;


    /* Reclaim the space of any arguments pushed on the stack. */

    if (bytesPushed > 0)
	    cout << "\taddq\t$" << bytesPushed << ", %rsp" << endl;

    assign(this, rax);

    debug_close("CALL");
}


/*
 * Function:	Block::generate
 *
 * Description:	Generate code for this block, which simply means we
 *		generate code for each statement within the block.
 */

void Block::generate()
{
    for (unsigned i = 0; i < _stmts.size(); i ++)
	    _stmts[i]->generate();
}


/*
 * Function:	Simple::generate
 *
 * Description:	Generate code for a simple (expression) statement, which
 *		means simply generating code for the expression.
 */

void Simple::generate()
{
    _expr->generate();
    assign(_expr, nullptr);
}


/*
 * Function:	Function::generate
 *
 * Description:	Generate code for this function, which entails allocating
 *		space for local variables, then emitting our prologue, the
 *		body of the function, and the epilogue.
 *
 *		The stack must be aligned at the point at which a function
 *		begins execution.  Since the call instruction pushes the
 *		return address on the stack and each function is expected
 *		to push its base pointer, we adjust our offset by that
 *		amount and then perform the alignment.
 *
 *		On a 32-bit Intel platform, 8 bytes are pushed (4 for the
 *		return address and 4 for the base pointer).  Since Linux
 *		requires a 4-byte alignment, all we need to do is ensure
 *		the stack size is a multiple of 4, which will usually
 *		already be the case.  However, since OS X requires a
 *		16-byte alignment (thanks, Apple, for inventing your own
 *		standards), we will often see an extra amount of stack
 *		space allocated.
 *
 *		On a 64-bit Intel platform, 16 bytes are pushed (8 for the
 *		return address and 8 for the base pointer).  Both Linux and
 *		OS X require 16-byte alignment.
 */

void Function::generate()
{
    unsigned size;
    int param_offset;
    Parameters* params = _id->type().parameters();
    const Symbols& symbols = _body->declarations()->symbols();


    return_label = new Label();


    /* Assign offsets to all symbols within the scope of the function. */

    param_offset = PARAM_OFFSET + SIZEOF_REG * callee_saved.size();
    offset = param_offset;
    allocate(offset);


    /* Generate the prologue. */

    funcname = _id->name();

    cout << global_prefix << funcname << ":" << endl;

    debug_open("PROLOGUE");

    cout << "\tpushq\t%rbp" << endl;

    for (unsigned i = 0; i < callee_saved.size(); i ++)
	    cout << "\tpushq\t" << callee_saved[i] << endl;

    cout << "\tmovq\t%rsp, %rbp" << endl;

    if (SIMPLE_PROLOGUE) 
    {
	    offset -= align(offset - param_offset);
	    cout << "\tsubq\t$" << -offset << ", %rsp" << endl;
    } 
    else 
    {
	    cout << "\tmovl\t$" << funcname << ".size, %eax" << endl;
	    cout << "\tsubq\t%rax, %rsp" << endl;
    }

    debug_close("PROLOGUE");


    /* Spill any parameters. */

    for (unsigned i = 0; i < NUM_PARAM_REGS; ++ i)
        if (i < params->size()) 
        {
            size = symbols[i]->type().size();
            cout << "\tmov" << suffix(size) << parameters[i]->name(size);
            cout << ", " << symbols[i]->_offset << "(%rbp)" << endl;
        } 
        else
            break;

    /* Generate the body and epilogue. */

    registers = (_hasCall && callee_saved.size() ? callee_saved : caller_saved);
    temp_offset = offset;
    _body->generate();
    offset = temp_offset;

    cout << *return_label << ":" << endl;
    cout << endl << global_prefix << funcname << ".exit:" << endl;
    cout << "\tmovq\t%rbp, %rsp" << endl;

    for (int i = callee_saved.size() - 1; i >= 0; -- i)
	    cout << "\tpopq\t" << callee_saved[i] << endl;

    cout << "\tpopq\t%rbp" << endl;
    cout << "\tret" << endl << endl;

    /* Finish aligning the stack. */

    if (!SIMPLE_PROLOGUE) 
    {
	    offset -= align(offset - param_offset);
	    cout << "\t.set\t" << funcname << ".size, " << -offset << endl;
    }

    cout << "\t.globl\t" << global_prefix << funcname << endl;
    cout << "\t.type\t" << global_prefix << funcname << ", @function" << endl << endl;
}


/*
 * Function:	generateGlobals
 *
 * Description:	Generate code for any global variable declarations.
 */

void generateGlobals(Scope* scope)
{
    const Symbols& symbols = scope->symbols();

    for (unsigned i = 0; i < strings.size(); ++ i)
        cout << strings[i] << endl;

    for (unsigned i = 0; i < symbols.size(); ++ i)
        if (!symbols[i]->type().isFunction()) 
        {
            cout << "\t.comm\t" << global_prefix << symbols[i]->name() << ", ";
            cout << symbols[i]->type().size() << endl;
        }
}

/*
 * Function:	Assignment::generate
 *
 * Description:	Generate code for an assignment statement.
 *
 *		NOT FINISHED: Only works if the right-hand side is an
 *		integer literal and the left-hand side is an integer
 *		scalar.
 */

void Assignment::generate()
{
    debug_open("ASSIGN");

    _left->generate();
    _right->generate();

    // if left = scalar && right = number
    cout << "\tmov" << suffix(_left) << _right << ", " << _left << endl;

    debug_close("ASSIGN"); 
}

void Return::generate()
{
    debug_open("RET"); 

    _expr->generate();

    cout << "\tmovl\t" << _expr << ", %eax";
    cout << "\tjmp\t" << *return_label << endl;

    debug_close("RET");
}

void While::generate()
{
    debug_open("WHILE");

    Label loop, exit;

    cout << loop << ":";

    _expr->test(exit, false);
    _stmt->generate();

    cout << "\tjmp\t" << loop << endl;

    debug_close("WHILE");
}

void If::generate()
{
    debug_open("IF");

    Label skip;

    _expr->test(skip, true);

    cout << "\tcmp\t$0, " << _expr << endl;
    cout << "\tje\t" << skip << endl;

    _thenStmt->generate();

    cout << skip << ":";

    debug_close("IF");
}

void Subtract::generate()
{
    debug_open("SUB");
    
    _left->generate();
    _right->generate();

    if (_left->_register == nullptr)
        load(_left, get_reg());

    cout << "\tsub" << suffix(_left);
    cout << _right << ", " << _left << endl;

    assign(_right, nullptr);
    assign(this, _left->_register);

    debug_close("SUB"); 
}

void Add::generate()
{
    debug_open("ADD");
    
    _left->generate();
    _right->generate();

    if (_left->_register == nullptr)
        load(_left, get_reg());

    cout << "\tadd" << suffix(_left);
    cout << _right << ", " << _left << endl;

    assign(_right, nullptr);
    assign(this, _left->_register);

    debug_close("ADD"); 
}

void Remainder::generate()
{
    debug_open("REM");

    _left->generate();
    _right->generate();

    cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\tcltd" << endl;
    cout << "\tidiv" << suffix(_right) << _right << endl;

    assign(this, rdx);

    debug_close("REM");
}

void Divide::generate()
{
    debug_open("DIV");

    _left->generate();
    _right->generate();

    cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\tcltd" << endl;
    cout << "\tidiv" << suffix(_right) << _right << endl;

    assign(this, rax);

    debug_close("DIV");
}

void Multiply::generate()
{
    debug_open("MUL");

    _left->generate();
    _right->generate();

    if (_left->_register == nullptr)
        load(_left, get_reg());

    cout << "\timul" << suffix(_right) << _right << ", " << _left << endl;

    assign(_right, nullptr);
    assign(this, _left->_register);

    debug_close("MUL");
}

void Cast::generate()
{

}

void Address::generate()
{
    debug_open("ADDR");

    _expr->generate();

    cout << "\tleaq\t" << _expr << ", ";
    assign(_expr, get_reg());
    cout << _expr->_register->as_qword() << endl;

    assign(this, _expr->_register);
    assign(_expr, nullptr);

    debug_close("ADDR");
}

void Dereference::generate()
{
}

void Negate::generate()
{
    debug_open("NEG");

    _expr->generate();

    if (_expr->_register == nullptr)
        load(_expr, get_reg());

    cout << "\tneg" << suffix(_expr) << "\t" << _expr << endl;

    debug_close("NEG");
}

void Not::generate()
{
    debug_open("NOT");

    debug_close("NOT");
}

void Field::generate()
{

}

/*
 * Function:	assign (private)
 *
 * Description:	Assign the given expression to the given register.  No
 *		assembly code is generated here as only the pointers are
 *		updated.
 */

static void assign(Expression* expr, Register* reg)
{
    if (expr != nullptr)
    {
        if (expr->_register != nullptr)
            expr->_register->_node = nullptr;

        expr->_register = reg;
    }

    if (reg != nullptr)
    {
        if (reg->_node != nullptr)
            reg->_node->_register = nullptr;

        reg->_node = expr;
    }
}


/*
 * Function:	load (private)
 *
 * Description:	Load the given expression into the given register.
 */

static void load(Expression* expr, Register* reg)
{
    if (reg->_node != expr)
    {
        if (reg->_node != nullptr)
        {
            unsigned size = reg->_node->type().size();

            offset -= size;
            reg->_node->_offset = offset;

            cout << "\tmov" << suffix(reg->_node);
            cout << reg->name(size) << ", ";
            cout << offset << "(%rbp)" << endl;
        }
        
        if (expr != nullptr)
        {
            unsigned size = expr->type().size();

            cout << "\tmov" << suffix(expr) << expr;
            cout << ", " << reg->name(size) << endl;
        }

        assign(expr, reg);
    }
}
