struct node a, *b;		/* 'a' has incomplete type */

struct node {
    int data;
    struct node *next;
};

struct node x, *y;

struct node node;

int node;			/* conflicting types for 'node' */

int foo;

struct foo {
    struct foo *foo;
    int x, y;
};

struct foo {			/* redefinition of 'foo' */
    int x, y;
};

struct foo f();			/* pointer type required for 'f' */

struct foo *g(struct foo *p,
	struct foo q) { }	/* pointer type required for 'q' */
