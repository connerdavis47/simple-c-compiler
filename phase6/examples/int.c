/* int.c */

int printf();

int main(void)
{
    int x, y, z;
    int a, b, c, d, e;

    x = 100;
    y = 30;
    z = 2;

    a = x + y + z;
    b = x - y - z;
    c = x * y * z;
    d = x / y + z;
    e = x % y - z;

    printf("%d\n", a);
    printf("%d\n", b);
    printf("%d\n", c);
    printf("%d\n", d);
    printf("%d\n", e);
}
