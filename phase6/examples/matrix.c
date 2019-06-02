/* matrix.c */

int free(), *malloc(), printf(), scanf();

int **allocate(int n)
{
    int i;
    int **a;

    i = 0;
    a = (int **) malloc(n * sizeof(*a));

    while (i < n) {
	a[i] = malloc(n * sizeof(**a));
	i = i + 1;
    }

    return a;
}

int initialize(int **a, int n)
{
    int i, j;


    i = 0;

    while (i < n) {
	j = 0;

	while (j < n) {
	    a[i][j] = i + j;
	    /**(*(a + i) + j) = i + j;*/
	    j = j + 1;
	}

	i = i + 1;
    }
}

int display(int **a, int n)
{
    int i, j;
    int *p;

    i = 0;

    while (i < n) {
	j = 0;

	while (j < n) {
	    p = a[i];
	    printf("%d ", p[j]);
	    j = j + 1;
	}

	i = i + 1;
	printf("\n");
    }
}

int deallocate(int **a, int n)
{
    int i;

    i = 0;

    while (i < n) {
	free(a[i]);
	i = i + 1;
    }

    free(a);
}

int main(void)
{
    int **a;
    int n;

    scanf("%d", &n);
    a = allocate(n);
    initialize(a, n);
    display(a, n);
    deallocate(a, n);
}
