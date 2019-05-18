/*
 * tree2.c, or fine, I'll do it the right way, but they're still overrated.
 */

struct node {
    int data;
    struct node *left, *right;
};

int printf();
struct node *malloc(), *null;

struct node *insert(struct node *root, int data)
{
    if (!root) {
	root = malloc(sizeof(*root));
	root->data = data;
	root->left = null;
	root->right = null;
    } else if (data < root->data) {
	root->left = insert(root->left, data);
    } else if (data > root->data)
	root->right = insert(root->right, data);

    return root;
}

int search(struct node *root, int data)
{
    if (!root)
	return 0;

    if (data < root->data)
	return search(root->left, data);

    if (data > root->data)
	return search(root->right, data);

    return 1;
}

int preorder(struct node *root)
{
    if (root) {
	printf("%d\n", root->data);
	preorder(root->left);
	preorder(root->right);
    }
}

int inorder(struct node *root)
{
    if (root) {
	inorder(root->left);
	printf("%d\n", root->data);
	inorder(root->right);
    }
}

int main(void)
{
    struct node *root;

    root = null;
    root = insert(root, 7);
    root = insert(root, 4);
    root = insert(root, 1);
    root = insert(root, 0);
    root = insert(root, 5);
    root = insert(root, 2);
    root = insert(root, 3);
    root = insert(root, 6);

    printf("preorder traversal:\n");
    preorder(root);
    printf("inorder traversal:\n");
    inorder(root);
}
