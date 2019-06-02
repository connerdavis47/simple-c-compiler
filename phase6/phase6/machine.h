/*
 * File:	machine.h
 *
 * Description:	This file contains the values of various parameters for the
 *		target machine architecture.
 */

# define SIZEOF_INT 4
# define SIZEOF_LONG 8
# define SIZEOF_PTR 8
# define SIZEOF_REG 8

# define ALIGNOF_INT 4
# define ALIGNOF_LONG 8
# define ALIGNOF_PTR 8

# define SIZEOF_PARAM 8
# define PARAM_OFFSET 16
# define NUM_PARAM_REGS 6
# define STACK_ALIGNMENT 16

# if defined (__linux__) && defined(__x86_64__)

# define global_prefix ""
# define global_suffix "(%rip)"
# define label_prefix ".L"

# elif defined (__APPLE__) && defined(__x86_64__)

# define global_prefix "_"
# define global_suffix "(%rip)"
# define label_prefix "L"

# else

# error Unsupport architecture
# endif
