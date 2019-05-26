# define SIZEOF_ARG 8
# define SIZEOF_INT 4
# define SIZEOF_LONG 8
# define SIZEOF_PTR 8

# define ARG_OFFSET 16

# if defined (__linux__) && (defined(__i386__) || defined(__x86_64__))

# define STACK_ALIGNMENT 4
# define global_prefix ""
# define label_prefix ".L"

# elif defined (__APPLE__) && (defined(__i386__) || defined(__x86_64__))

# define STACK_ALIGNMENT 16
# define global_prefix "_"
# define label_prefix "L"

# else

# error System architecture not supported

# endif
