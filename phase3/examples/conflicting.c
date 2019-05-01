int x;
int x;

int a[10];
int a[5];			/* conflicting types for 'a' */

long *x;			/* conflicting types for 'x' */

int g();
int *g();			/* conflicting types for 'g' */

int *f();

int *f(int z)
{
}

int f;				/* conflicting types for 'f' */

int h;
int h();			/* conflicting types for 'h' */
