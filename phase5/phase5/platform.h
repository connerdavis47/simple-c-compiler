# define SIZEOF_ARG 8
# define SIZEOF_INT 4
# define SIZEOF_LONG 8
# define SIZEOF_PTR 8

# define ARG_OFFSET 16

# if defined (__linux__)

# define STACK_ALIGNMENT 16
# define label_prefix ".L"

# else

# error "System architecture not supported"

# endif
