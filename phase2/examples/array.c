int b[10], x;

int main(void)
{
    int ***p3, **p2, *p1, b[10], c[20], x;

    p3 = &p2;
    p2 = &p1;
    p1 = &x;

    p3[1 - 1][2 * 0][0 / 3] = b[4 > x] * c[5 % 6];
    return x >= 4 && *p1 < 1 + x * !! 3;
}
