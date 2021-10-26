#include <stdio.h>
#include <stdlib.h>
#include "avl_tree.h"

/**
 * Supprimer un noeud et tous ses sous enfants
 * @param node
 */
static void __recursive_destroy_avl_node(AVLNode *node);

/**
 * Afficher les détails d'un arbre dans la console
 * @param node
 */
static void __recursive_print_avl_node(AVLNode node);

/**
 * Ajouter un enfant à un noeud
 * @param root_node
 * @param new_node
 */
static void __add_avl_node(AVLNode *root_node, AVLNode *new_node);

/**
 * Supprimer un noeud
 * @param node
 */
static void __remove_avl_node(AVLNode *node);

/**
 * Change le noeud root de l'arbre
 * @param tree
 * @param new_root
 */
static void __set_root(AVLTree *tree, AVLNode *new_root);

/**
 * Récupère le noeud le grand à partir du noeud passé en paramètre
 * @param from_node
 * @return
 */
static AVLNode __retrieve_max_node(AVLNode from_node);

/**
 * Créer une relation parent-enfant entre deux noeuds
 * @param parentNode
 * @param childNode
 */
static void __create_node_relation(AVLNode *parentNode, AVLNode *childNode, link_t link_type);

/**
 * Supprimer une relation entre deux noeuds
 * @param parentNode
 * @param childNode
 */
static void __delete_node_relation(AVLNode *parentNode, AVLNode *childNode);

/**
 * Met à jour les poids de chaque parent du noeud passé en paramètres
 * @param node
 * @param weight_propagation_type
 */
static void __recursive_weight_backpropagation(AVLNode *node, weight_propagation_t weight_propagation_type);

/**
 * Rééquilibre l'arbre à partir des poids, effectue des rotations si besoin
 * @param tree
 * @param from_node
 */
static void __tree_rebalance(AVLTree *tree, AVLNode *from_node);

/**
 * Effectue une rotation gauche ou droite autour du noeud passé en paramètre
 * @param tree
 * @param root_rotation_node
 * @param rotation_type
 *
 * Left rotation (with node3)
 * ----
 *        node(-2)                      node2(0)
 *           \                         /     \
 *         node2(-1)       to     node(0)   node3(0)
 *             \
 *            node3(0)
 *
 * Left rotation (without node3)
 * ----
 *       parent(2)                     parent(2)
 *         /                            /
 *      node(-1)         to          node2(1)
 *         \                          /
 *        node2(0)                node(0)
 *
 * Right rotation (with node3)
 * ----
 *       node(2)                    node2(0)
 *        /                         /     \
 *     node2(1)       to      node3(0)   node(0)
 *      /
 *   node3(0)
 *
 * Right rotation (without node3)
 * ----
 *      parent(-2)                  parent(-2)
 *          \                           \
 *        node(1)         to          node2(-1)
 *          /                             \
 *      node2(0)                        node(0)
 */
static void __apply_rotation(AVLTree *tree, AVLNode *root_rotation_node, rotation_t rotation_type);

/**
 * Lève une exception et quitte le code
 * @param exception_identifier
 * @param message
 */
static void exception(char* exception_identifier, char* message);

// ---------------------------------------------

static void exception(char* exception_identifier, char* message) {
    fprintf(stderr, "Error [%s]: %s", exception_identifier, message);
    exit(EXIT_FAILURE);
}

// ---------------------------------------------

AVLTree create_avl_tree() {
    AVLTree tree = malloc(sizeof(AVLTreeElement));

    if(tree == NULL) exception("E-CAT", "Malloc");

    tree->root = NULL;
    return tree;
}

Bool is_empty_avl_tree(AVLTree tree) {
    return tree->root == NULL;
}

Bool avl_tree_contain(AVLTree tree, int value) {
    if(is_empty_avl_tree(tree)) return false;

    AVLNode current = tree->root;

    while(current != NULL && current->value != value) {
        if(value < current->value) current = current->left;
        else current = current->right;
    }

    return current != NULL;
}

void print_avl_tree(AVLTree tree) {
    printf("\n--------  AVL Tree Info  --------\n");

    if(is_empty_avl_tree(tree)) {
        printf("\nEmpty AVL Tree");
    } else {
        if(tree->root->left != NULL)
            printf("\nMax Left Node: %d\n", __retrieve_max_node(tree->root->left)->value);

        printf("\nMax Right Node: %d\n", __retrieve_max_node(tree->root)->value);

        __recursive_print_avl_node(tree->root);
    }

    printf("\n\n------  End AVL Tree Info  ------\n\n");
}

static void __recursive_print_avl_node(AVLNode node) {
    printf("\n[node %d %s(weight: %d | factor: %d) #%d] -> ", node->value, (node->parent == NULL) ? "(root) " : "", node->weight, node->weight_factor, (node->parent != NULL) ? node->parent->value : -1);

    (node->left == NULL) ? printf("left: Empty ") : printf("left: [%d] ", node->left->value);
    (node->right == NULL) ? printf("right: Empty ") : printf("right: [%d] ", node->right->value);

    if(node->left != NULL) __recursive_print_avl_node(node->left);
    if(node->right != NULL) __recursive_print_avl_node(node->right);
}

void destroy_avl_tree(AVLTree *tree) {
    if(!is_empty_avl_tree((*tree)))
        __recursive_destroy_avl_node(&(*tree)->root);

    free(*tree);
    *tree = NULL;
}

static void __recursive_destroy_avl_node(AVLNode *node) {
    if((*node)->right != NULL) __recursive_destroy_avl_node(&(*node)->right);
    if((*node)->left != NULL) __recursive_destroy_avl_node(&(*node)->left);

    free(*node);
    *node = NULL;
}

// ---------------------------------------------

static void __set_root(AVLTree *tree, AVLNode *new_root) {
    (*tree)->root = *new_root;
    (*new_root)->parent = NULL;
    (*new_root)->weight_factor = 0;
}

// ---------------------------------------------

Bool avl_node_is_leaf(AVLNode node) {
    return (node != NULL) && (node->left == NULL) && (node->right == NULL);
}

Bool avl_node_is_root(AVLNode node) {
    return (node != NULL) && (node->parent == NULL);
}

AVLNode add_avl_node(AVLTree *tree, int value) {
    AVLNode node = malloc(sizeof(AVLNodeElement));

    node->value = value;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->weight = 0;
    node->weight_factor = 0;

    if((*tree)->root == NULL) {
        __set_root(tree, &node);
        return node;
    }

    __add_avl_node(&(*tree)->root, &node);
    __recursive_weight_backpropagation(&node, Propagate_Add_Node);
    __tree_rebalance(tree, &node);

    return node;
}

static void __add_avl_node(AVLNode *root_node, AVLNode *new_node) {
    if((*new_node)->value < (*root_node)->value) {
        if((*root_node)->left != NULL) {
            return __add_avl_node(&(*root_node)->left, new_node);
        } else {
            __create_node_relation(root_node, new_node, Left_Link);
        }
    } else {
        if((*root_node)->right != NULL) {
            return __add_avl_node(&(*root_node)->right, new_node);
        } else {
            __create_node_relation(root_node, new_node, Right_Link);
        }
    }
}

// ---------------------------------------------

void remove_avl_node(AVLTree *tree, AVLNode *node_to_remove) {
    if(avl_node_is_leaf(*node_to_remove)) {
        if(avl_node_is_root(*node_to_remove)) {
            __remove_avl_node(node_to_remove);
            (*tree)->root = NULL;
            return;
        } else {
            AVLNode parent = (*node_to_remove)->parent;

            if((*node_to_remove)->weight_factor == 1 && (*node_to_remove)->parent->right != NULL) {
                (*node_to_remove)->parent->weight--;
                __delete_node_relation(&(*node_to_remove)->parent, node_to_remove);
                __remove_avl_node(node_to_remove);
            } else if((*node_to_remove)->weight_factor == -1 && (*node_to_remove)->parent->left != NULL) {
                (*node_to_remove)->parent->weight++;
                __delete_node_relation(&(*node_to_remove)->parent, node_to_remove);
                __remove_avl_node(node_to_remove);
            } else {
                __recursive_weight_backpropagation(node_to_remove, Propagate_Delete_Node);
                __delete_node_relation(&(*node_to_remove)->parent, node_to_remove);
                __remove_avl_node(node_to_remove);
            }

            __tree_rebalance(tree, &parent);
            return;
        }
    } else {
        if((*node_to_remove)->left != NULL) {
            AVLNode max_node = __retrieve_max_node((*node_to_remove)->left);

            if(max_node->parent == *node_to_remove) {
                if(avl_node_is_root(*node_to_remove)) {
                    __recursive_weight_backpropagation(&max_node, Propagate_Delete_Node);
                    max_node->weight = (*node_to_remove)->weight;
                    __set_root(tree, &max_node);
                } else {
                    __create_node_relation(&(*node_to_remove)->parent, &max_node, Left_Link);
                    __recursive_weight_backpropagation(&max_node, Propagate_Delete_Node);
                }

                if((*node_to_remove)->right != NULL)
                    __create_node_relation(&max_node, &(*node_to_remove)->right, Right_Link);

                __remove_avl_node(node_to_remove);
                return;
            } else {
                if(max_node->left != NULL) {
                    __create_node_relation(&max_node->parent, &max_node->left, Right_Link);
                    __recursive_weight_backpropagation(&max_node->left, Propagate_Delete_Node);
                } else {
                    if(max_node->parent->left != NULL)
                        max_node->parent->weight++;
                    else
                        __recursive_weight_backpropagation(&max_node, Propagate_Delete_Node);

                    max_node->right = NULL;
                }

                __create_node_relation(&max_node, &(*node_to_remove)->left, Left_Link);

                if((*node_to_remove)->right != NULL)
                    __create_node_relation(&max_node, &(*node_to_remove)->right, Right_Link);

                max_node->weight_factor = (*node_to_remove)->weight_factor;
                max_node->weight = (*node_to_remove)->weight;

                AVLNode old_max_node_parent = max_node->parent;

                if(avl_node_is_root(*node_to_remove))
                    __set_root(tree, &max_node);
                else
                    __create_node_relation(&(*node_to_remove)->parent, &max_node, Use_Factor);

                __tree_rebalance(tree, &old_max_node_parent);
                __remove_avl_node(node_to_remove);
                return;
            }
        } else {
            AVLNode max_node = (*node_to_remove)->right;

            __recursive_weight_backpropagation(&max_node, Propagate_Delete_Node);

            max_node->weight_factor = (*node_to_remove)->weight_factor;

            if(avl_node_is_root(*node_to_remove))
                __set_root(tree, &max_node);
            else
                __create_node_relation(&(*node_to_remove)->parent, &max_node, Use_Factor);

            __tree_rebalance(tree, &max_node);
            __remove_avl_node(node_to_remove);
            return;
        }
    }
}

static void __remove_avl_node(AVLNode *node) {
    (*node)->left = NULL;
    (*node)->right = NULL;
    (*node)->parent = NULL;
    free(*node);
    *node = NULL;
}

// ---------------------------------------------

static AVLNode __retrieve_max_node(AVLNode from_node) {
    if(from_node == NULL) exception("E-RMN", "Node parameter is NULL");

    AVLNode current = from_node;
    while(current->right != NULL) current = current->right;
    return current;
}

static void __create_node_relation(AVLNode *parentNode, AVLNode *childNode, link_t link_type) {
    if((*parentNode) == NULL) exception("E-CNC", "Null parent pointer");

    if((*childNode) == NULL) exception("E-CNC", "Null child node pointer");

    if(link_type == Use_Factor)
        link_type = ((*childNode)->weight_factor == 1) ? Left_Link : Right_Link;

    if(link_type == Left_Link) {
        (*parentNode)->left = (*childNode);
        (*childNode)->weight_factor = 1;
    } else if(link_type == Right_Link) {
        (*parentNode)->right = (*childNode);
        (*childNode)->weight_factor = -1;
    } else exception("E-CNC", "Unknown link type");

    (*childNode)->parent = (*parentNode);
}

static void __delete_node_relation(AVLNode *parentNode, AVLNode *childNode) {
    if((*parentNode) == NULL) exception("E-DNR", "Null parent pointer");

    if((*childNode) == NULL) exception("E-DNR", "Null child node poiniter");

    if((*childNode)->parent != (*parentNode)) exception("E-DNR", "Parent node is not the parent of child node");

    if((*childNode)->weight_factor == 1) {
        if((*parentNode)->left != (*childNode))
            exception("E-DNR", "Child node is not the left child of the parent node");

        (*parentNode)->left = NULL;
    } else if((*childNode)->weight_factor == -1) {
        if((*parentNode)->right != (*childNode))
            exception("E-DNR", "Child node is not the right child of the parent node");

        (*parentNode)->right = NULL;
    } else exception("E-DNR", "Unknown weight factor");

    (*childNode)->parent = NULL;
}

// ---------------------------------------------

static void __tree_rebalance(AVLTree *tree, AVLNode *from_node) {
    AVLNode current = (*from_node);

    while(current != NULL && -1 <= current->weight && current->weight <= 1)
        current = current->parent;

    if(current == NULL) return;

    if(current->weight == 2) {
        if(current->left->weight == 1) {
            __apply_rotation(tree, &current, Rotate_R);
        } else if(current->left->weight == -1) {
            __apply_rotation(tree, &current->left, Rotate_L);
            __apply_rotation(tree, &current, Rotate_R);
        } else exception("E-TR", "Invalid weight");
    } else if(current->weight == -2) {
        if(current->right->weight == -1) {
            __apply_rotation(tree, &current, Rotate_L);
        }  else if(current->right->weight == 1) {
            __apply_rotation(tree, &current->right, Rotate_R);
            __apply_rotation(tree, &current, Rotate_L);
        } else exception("E-TR", "Invalid weight");
    }

    if(current->parent != NULL)
        __tree_rebalance(tree, &current->parent);
}

static void __apply_rotation(AVLTree *tree, AVLNode *root_rotation_node, rotation_t rotation_type) {
    AVLNode node = (*root_rotation_node);
    AVLNode node2 = (rotation_type == Rotate_L) ? node->right : node->left;

    if(node2 == NULL) exception("E-AR", "Null pointer");

    AVLNode node3 = (rotation_type == Rotate_L) ? node->right->right : node->left->left;

    if(node3 == NULL) {
        if((*tree)->root == node) exception("E-AR", "Root node cannot rotate in this configuration");

        if(node->parent == NULL) exception("E-AR", "Null parent pointer");

        int tmp_weight = node2->weight;
        node2->weight = -node->weight;
        node->weight = tmp_weight;

        // update parent connection
        if(rotation_type == Rotate_L) {
            __create_node_relation(&node->parent, &node2, Left_Link);
            __create_node_relation(&node2, &node, Left_Link);

            node2->left = node;
            node->right = NULL;
        } else if(rotation_type == Rotate_R) {
            __create_node_relation(&node->parent, &node2, Right_Link);
            __create_node_relation(&node2, &node, Right_Link);

            node2->right = node;
            node->left = NULL;
        } else exception("E-AR", "Unknown rotation type");

        return;
    }

    __recursive_weight_backpropagation(&node3, Propagate_Delete_Node);

    // update node2 parent
    if((*tree)->root == node) {
        __set_root(tree, &node2);
    } else {
        if(node->parent == NULL) exception("E-AR", "Null parent pointer");

        __create_node_relation(&node->parent, &node2, (rotation_type == Rotate_L) ? Right_Link : Left_Link);
    }

    if(rotation_type == Rotate_L) {
        // update node connection
        if(node2->left != NULL)
            __create_node_relation(&node, &node2->left, Right_Link);
        else
            node->right = NULL;

        __create_node_relation(&node2, &node, Left_Link);
    } else if(rotation_type == Rotate_R) {
        // update node connection
        if(node2->right != NULL)
            __create_node_relation(&node, &node2->right, Left_Link);
        else
            node->left = NULL;

        __create_node_relation(&node2, &node, Right_Link);
    } else exception("E-AR", "Unknown rotation type");

    // update node weight
    node->weight = 0;
}

// ---------------------------------------------

static void __recursive_weight_backpropagation(AVLNode *node, weight_propagation_t weight_propagation_type) {
    if((*node)->parent != NULL) {
        (*node)->parent->weight += (weight_propagation_type == Propagate_Add_Node) ? (*node)->weight_factor : -(*node)->weight_factor;
        __recursive_weight_backpropagation(&(*node)->parent, weight_propagation_type);
    }
}

