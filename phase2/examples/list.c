struct node {
    int data;
    struct node *link;
};

struct list {
    int count;
    struct node *head;
};

int *malloc();

int push(struct list *list, int x)
{
    struct node *node;

    node = (struct node *) malloc(sizeof(*node));
    node->link = (struct node *) 0;
    (*list).head = node;
    list->head->data = x;
}
