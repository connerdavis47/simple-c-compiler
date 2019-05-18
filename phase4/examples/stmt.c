int **f(void)
{
    int *p;
    return p;			/* invalid return type */
}

int main(void)
{
    if (main) main();		/* invalid type for test expression */
}
