int a;

int f(int x, int z, int z)	/* redeclaration of 'z' */
{
    int x, y;			/* redeclaration of 'x' */

    {
	int y, z;
    }

    {
	int x, z;
    }
}

int a;

int g(int x) {
}

int g(int y) {			/* redefinition of 'g' */
}
