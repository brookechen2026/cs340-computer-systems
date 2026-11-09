// inserts 10,000 random values into an unbalanced BST; no removals

#include "testharness.h"

typedef struct bst_node_t {
  struct bst_node_t *left, *right;
  int val;
} bst_node;

static void delete_tree(allocator *a, bst_node *n) {
  if (!n) return;
  delete_tree(a, n->left);
  delete_tree(a, n->right);
  a->free(n);
}

static bst_node *insert_or_else(allocator *a, bst_node *root, int value) {
  if (!root) {
    bst_node *n = a->malloc(sizeof(bst_node));
    n->val = value;
    n->left = n->right = NULL;
    return n;
  }
  if (value == root->val) { // delete duplicate nodes and their subtree
    delete_tree(a, root);
    return NULL;
  } else if (value < root->val) root->left = insert_or_else(a, root->left, value);
  else root->right = insert_or_else(a, root->right, value);
  return root;
}

static int validate(bst_node *root, int lower_bound, int upper_bound) {
  if (!root) return 1;
  if (root->val < lower_bound || root->val > upper_bound) return 0;
  return validate(root->left, lower_bound, root->val)
      && validate(root->right, root->val, upper_bound);
}

const char *mytest(allocator *a) {
  int lfg_state[10] = {124,128,173,225,222,340,357,361,374,421};
  int lfg_index = 9;
  bst_node *root = NULL;
  
  for(int i=0; i<1000; i+=1) {
    lfg_index += 1; lfg_index %= 10;
    int val = lfg_state[lfg_index] + lfg_state[(lfg_index+3)%10];
    val &= 0xFF;
    lfg_state[lfg_index] = val;
    root = insert_or_else(a, root, val);
  }
  if (!validate(root, 0x80000000, 0x7fffffff)) return "BST property violated";
  return NULL;
}
