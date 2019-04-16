/* malloc.c */

int *malloc();

int main(void)
{
    int n;
    int *p;

    n = 10;
    p = malloc((int) sizeof(*p) * n);
}
