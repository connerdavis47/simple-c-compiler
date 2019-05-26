/*
 * This file will not be run through your compiler.
 */

# include <stdio.h>

extern int a[];

void init_array(int n)
{
    int i;

    for (i = 0; i < n; i ++)
	a[i] = i;
}


void print_array(int n)
{
    int i;

    for (i = 0; i < n; i ++)
	printf("%d\n", a[i]);
}
