/* params.c */

int foo(), bar();

int foo(int x, int y)
{
    return -x + !y * 10 != 3;
}
