/* sum.c */

int printf();

int main(void)
{
    long i, n, sum;

    i = 0;
    sum = 0;
    n = 10;

    while (i <= n) {
	sum = sum + i;
	i = i + 1;
    }

    printf("%ld %ld\n", sum, n * (n + 1) / 2);
}
