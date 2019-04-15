// sum.c

int printf();

int main(void)
{
    long i, n, sum;

    i = 0;
    sum = 0l;
    n = 10L;

    while (i <= n) {
	sum = sum + i;
	i = i + 1;
    }

    printf("%ld %ld\n", sum, n * (n + 1) / 2);
}
