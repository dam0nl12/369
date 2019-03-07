#include <stdio.h>
#include <stdlib.h>

/* A binary tree node. */
struct node {
    int data;
    struct node* left;
    struct node* right;
};


/* Allocate a new node with the given data and NULL left and right pointers. */
struct node* newNode(int data) {
    struct node* node = (struct node*)
    malloc(sizeof(struct node));
    
    node->data = data;
    node->left = NULL;
    node->right = NULL;
    
    return(node);
}


/* Given a binary tree, print its nodes in preorder. */
void printPreorder(struct node* node) {
    if (node == NULL)
        return;
    
    /* First print data of node. */
    printf("%d ", node->data);
    
    /* Recur on left sutree. */
    printPreorder(node->left);
    
    /* Recur on right subtree. */
    printPreorder(node->right);
}


int main() {
    // Level 1.
    struct node *root = newNode(1);
    
    // 2
    root->left = newNode(2);
    root->right = newNode(3);
    
    // 3
    root->left->left = newNode(4);
    root->left->right = newNode(5);
    root->right->left = newNode(6);
    root->right->right = newNode(7);
    
    // 4
    root->left->left->left = newNode(8);
    root->left->left->right = newNode(9);
    
    root->left->right->left = newNode(10);
    root->left->right->right = newNode(11);
    
    root->right->left->left = newNode(12);
    root->right->left->right = newNode(13);
    
    root->right->right->left = newNode(14);
    root->right->right->right = newNode(15);
    
    // 5
    root->left->left->left->left = newNode(16);
    root->left->left->left->right = newNode(17);
    
    root->left->left->right->left = newNode(18);
    root->left->left->right->right = newNode(19);
    
    root->left->right->left->left = newNode(20);
    root->left->right->left->right = newNode(21);
    
    root->left->right->right->left = newNode(22);
    root->left->right->right->right = newNode(23);
    
    root->right->left->left->left = newNode(24);
    root->right->left->left->right = newNode(25);
    
    root->right->left->right->left = newNode(26);
    root->right->left->right->right = newNode(27);
    
    root->right->right->left->left = newNode(28);
    root->right->right->left->right = newNode(29);
    
    root->right->right->right->left = newNode(30);
    root->right->right->right->right = newNode(31);
    
    printf("\nPreorder traversal of binary tree: \n");
    printPreorder(root);
    printf("\n");
    
    // 5
    free(root->right->right->right->right);
    free(root->right->right->right->left);
    free(root->right->right->left->right);
    free(root->right->right->left->left);
    free(root->right->left->right->right);
    free(root->right->left->right->left);
    free(root->right->left->left->right);
    free(root->right->left->left->left);
    free(root->left->right->right->right);
    free(root->left->right->right->left);
    free(root->left->right->left->right);
    free(root->left->right->left->left);
    free(root->left->left->right->right);
    free(root->left->left->right->left);
    free(root->left->left->left->right);
    free(root->left->left->left->left);
    
    // 4
    free(root->right->right->right);
    free(root->right->right->left);
    free(root->right->left->right);
    free(root->right->left->left);
    free(root->left->right->right);
    free(root->left->right->left);
    free(root->left->left->right);
    free(root->left->left->left);
    
    // 3
    free(root->right->right);
    free(root->right->left);
    free(root->left->right);
    free(root->left->left);
    
    // 2
    free(root->right);
    free(root->left);
    
    // 1
    free(root);
    
    return 0;
}
