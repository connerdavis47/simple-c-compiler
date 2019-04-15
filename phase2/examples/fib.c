/* fib.c */

/*
 * return the nth fibonacci number
 */

int printf(), scanf();

int fib(int n)
{
    if (n == 0 || n == 1) return 1;
    return fib(n - 1) + fib(n - 2);
}


int main(void)
{
    int n;

    scanf("%d", &n);
    printf("%d\n", fib(n));
}
