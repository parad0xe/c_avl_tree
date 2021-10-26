
#include "../types.h"

#ifndef DATA_STRUCT_AVL_TREE_H
#define DATA_STRUCT_AVL_TREE_H

typedef enum _link_t {
    Left_Link,
    Right_Link,
    Use_Factor
} link_t;

typedef enum _rotation_t {
    Rotate_R,
    Rotate_L
} rotation_t;

typedef enum _weight_propagation_t {
    Propagate_Add_Node,
    Propagate_Delete_Node
} weight_propagation_t;

typedef struct _avl_node {
    int value;
    struct _avl_node *left;
    struct _avl_node *right;
    struct _avl_node *parent;
    int weight;
    int weight_factor;
} AVLNodeElement, *AVLNode;

typedef struct _avl_tree {
    struct _avl_node *root;
} AVLTreeElement, *AVLTree;

// ---------------------------------------------

AVLTree create_avl_tree();
Bool is_empty_avl_tree(AVLTree tree);
Bool avl_tree_contain(AVLTree tree, int value);
void print_avl_tree(AVLTree tree);
void destroy_avl_tree(AVLTree *tree);

// ---------------------------------------------

Bool avl_node_is_leaf(AVLNode node);
Bool avl_node_is_root(AVLNode node);
AVLNode add_avl_node(AVLTree *tree, int value);
void remove_avl_node(AVLTree *tree, AVLNode *node);

#endif //DATA_STRUCT_AVL_TREE_H
