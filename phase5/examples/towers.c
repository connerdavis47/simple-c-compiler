int call_towers(), print_move(), print();

int towers(int n, int from, int to, int spare)
{
    call_towers(n, from, spare, to);
    print_move(from, to);
    call_towers(n, spare, to, from);
}

int main(void)
{
    int n;

    n = 3;
    print(n);
    towers(n, 1, 2, 3);
}
