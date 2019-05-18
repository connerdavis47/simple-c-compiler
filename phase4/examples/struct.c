struct foo {
    int x, *p, a[10];
};

struct bar {
    int x, *p, a[10];
};

struct foo sf, *psf, asf[10];
struct bar sb, *psb;
struct node *np, **npp;

int main(void) {
    int x, y;

    sf.x = 0;
    sf.a = 0;                   /* lvalue required in expression */
    sf.bar = 0;                 /* invalid operands to binary .  */

    x.x = 0;                    /* invalid operands to binary .  */

    psf->x = 0;
    psf->a = 0;                 /* lvalue required in expression */
    psf->foo = 0;               /* invalid operands to binary -> */
    asf->p = &x;

    (*asf).p = &x;
    (*psf).a[x] = x;

    sf = sb;                    /* invalid operands to binary =  */
    psb = (struct bar *) &sf;

    np + 1;			/* using pointer to incomplete type */
    npp + 1;
    *np;			/* using pointer to incomplete type */
}
