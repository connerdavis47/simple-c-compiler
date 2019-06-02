/*
 * tree.c, or I've always thought structures were overrated, didn't you?
 *
 * Structures?  We ain't got no structures.  We don't need no structures.
 * I don't have to show you any stinkin' structures!
 *
 * Believe it or not, your compiler will be able to generate assembly code
 * for this program.  Scary, huh?
 */

int printf(), *malloc(), *null;

int **insert(int **root, int *data)
{
    if (!root) {
	root = (int **) malloc(sizeof(*root) * 3);
	root[0] = data;
	root[1] = null;
	root[2] = null;
    } else if (data < root[0]) {
	root[1] = (int *) insert((int **) root[1], data);
    } else if (data > root[0])
	root[2] = (int *) insert((int **) root[2], data);

    return root;
}

int search(int **root, int *data)
{
    if (!root)
	return 0;

    if (data < root[0])
	return search((int **) root[1], data);

    if (data > root[0])
	return search((int **) root[2], data);

    return 1;
}

int preorder(int **root)
{
    if (root) {
	printf("%d\n", *root[0]);
	preorder((int **) root[1]);
	preorder((int **) root[2]);
    }
}

int inorder(int **root)
{
    if (root) {
	inorder((int **) root[1]);
	printf("%d\n", *root[0]);
	inorder((int **) root[2]);
    }
}

int main(void)
{
    int **root;
    int a[10], i;

    i = 0;

    while (i < 8) {
	a[i] = i;
	i = i + 1;
    }

    root = (int **) null;
    root = insert(root, &a[7]);
    root = insert(root, &a[4]);
    root = insert(root, &a[1]);
    root = insert(root, &a[0]);
    root = insert(root, &a[5]);
    root = insert(root, &a[2]);
    root = insert(root, &a[3]);
    root = insert(root, &a[6]);
    printf("preorder traversal:\n");
    preorder(root);
    printf("inorder traversal:\n");
    inorder(root);
}
