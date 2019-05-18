int x, y, *p;
long m, n, *q;

int main(void)
{
    x || y && m;
    q && m || x;
    q || n && p;

    x == y != n;
    10 != n == y;

    x < y > m;
    1 > n < x;
    x <= n >= x;
    p >= (int *) q <= x;

    q + x - n;
    x - n + p;

    m * x / n;
    x / y * n;
    x % n * m;

    p + x - m;
    m + p - 10;
    p + q;			/* invalid operands to binary + */

    p - q;			/* invalid operands to binary - */
    y - p + x;			/* invalid operands to binary - */
    p - (int *) q + x;

    p - (int *) q;
    1 + (long ***) p;
    10 + (int) q;		/* invalid operand in cast expression */

    - x;
    - m;
    - p;			/* invalid operand to unary - */

    ! x;
    ! m;
    ! p;
    ! x * m;
    ! p * n;

    &p + x;
    &y - x;

    x + sizeof(p);
    x + sizeof(sizeof(10));
}
