#include "avl_tree.h"

#include <stdlib.h>

typedef struct Node {
    int value;
    int height;
    struct Node* left;
    struct Node* right;
} Node;

struct AvlTree {
    Node* root;
    size_t size;
};

static Node* node_create(int value);
static void node_destroy(Node* node);
static int node_height(const Node* node);
static void update_height(Node* node);
static int balance_factor(const Node* node);
static Node* rotate_left(Node* node);
static Node* rotate_right(Node* node);
static Node* rebalance(Node* node);
static Node* node_insert(Node* node, int value, int* status);
static Node* node_remove(Node* node, int value, int* status);
static Node* find_min(Node* node);
static int node_contains(const Node* node, int value);
static void inorder_traverse(const Node* node, int* out_values, size_t max_len, size_t* count);
static void traverse_structure(const Node* node, size_t depth, size_t position,
                               AvlTreeChildType child_type, AvlTreeVisitFn visitor, void* user_data);

AvlTree* avl_tree_create(void) {
    AvlTree* tree = (AvlTree*)malloc(sizeof(AvlTree));
    if (!tree) {
        return NULL;
    }
    tree->root = NULL;
    tree->size = 0;
    return tree;
}

void avl_tree_destroy(AvlTree* tree) {
    if (!tree) {
        return;
    }
    node_destroy(tree->root);
    free(tree);
}

int avl_tree_insert(AvlTree* tree, int value) {
    if (!tree) {
        return -1;
    }

    int status = 0;
    Node* new_root = node_insert(tree->root, value, &status);
    if (status == -1) {
        return -1;
    }
    tree->root = new_root;
    if (status == 1) {
        tree->size += 1;
    }
    return status;
}

int avl_tree_remove(AvlTree* tree, int value) {
    if (!tree) {
        return 0;
    }

    int status = 0;
    Node* new_root = node_remove(tree->root, value, &status);
    tree->root = new_root;
    if (status == 1) {
        tree->size -= 1;
    }
    return status;
}

int avl_tree_contains(const AvlTree* tree, int value) {
    if (!tree) {
        return 0;
    }
    return node_contains(tree->root, value);
}

size_t avl_tree_size(const AvlTree* tree) {
    return tree ? tree->size : 0U;
}

int avl_tree_empty(const AvlTree* tree) {
    return (tree == NULL) || (tree->size == 0U);
}

size_t avl_tree_in_order(const AvlTree* tree, int* out_values, size_t max_len) {
    if (!tree || !out_values || max_len == 0U) {
        return 0U;
    }
    size_t count = 0U;
    inorder_traverse(tree->root, out_values, max_len, &count);
    return count;
}

void avl_tree_traverse_structure(const AvlTree* tree, AvlTreeVisitFn visitor, void* user_data) {
    if (!tree || !visitor || !tree->root) {
        return;
    }
    traverse_structure(tree->root, 0U, 0U, AVL_CHILD_ROOT, visitor, user_data);
}

static Node* node_create(int value) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) {
        return NULL;
    }
    node->value = value;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static void node_destroy(Node* node) {
    if (!node) {
        return;
    }
    node_destroy(node->left);
    node_destroy(node->right);
    free(node);
}

static int node_height(const Node* node) {
    return node ? node->height : 0;
}

static void update_height(Node* node) {
    if (!node) {
        return;
    }
    int left_height = node_height(node->left);
    int right_height = node_height(node->right);
    node->height = (left_height > right_height ? left_height : right_height) + 1;
}

static int balance_factor(const Node* node) {
    if (!node) {
        return 0;
    }
    return node_height(node->left) - node_height(node->right);
}

static Node* rotate_left(Node* node) {
    Node* new_root = node->right;
    Node* moved_subtree = new_root->left;
    new_root->left = node;
    node->right = moved_subtree;
    update_height(node);
    update_height(new_root);
    return new_root;
}

static Node* rotate_right(Node* node) {
    Node* new_root = node->left;
    Node* moved_subtree = new_root->right;
    new_root->right = node;
    node->left = moved_subtree;
    update_height(node);
    update_height(new_root);
    return new_root;
}

static Node* rebalance(Node* node) {
    int balance = balance_factor(node);
    if (balance > 1) {
        if (balance_factor(node->left) < 0) {
            node->left = rotate_left(node->left);
        }
        return rotate_right(node);
    }
    if (balance < -1) {
        if (balance_factor(node->right) > 0) {
            node->right = rotate_right(node->right);
        }
        return rotate_left(node);
    }
    return node;
}

static Node* node_insert(Node* node, int value, int* status) {
    if (!status) {
        return node;
    }

    if (!node) {
        Node* new_node = node_create(value);
        if (!new_node) {
            *status = -1;
            return NULL;
        }
        *status = 1;
        return new_node;
    }

    if (value == node->value) {
        *status = 0;
        return node;
    }

    if (value < node->value) {
        Node* original = node->left;
        Node* updated = node_insert(node->left, value, status);
        if (*status == -1) {
            node->left = original;
            return node;
        }
        node->left = updated;
    } else {
        Node* original = node->right;
        Node* updated = node_insert(node->right, value, status);
        if (*status == -1) {
            node->right = original;
            return node;
        }
        node->right = updated;
    }

    if (*status == 1) {
        update_height(node);
        node = rebalance(node);
    }

    return node;
}

static Node* node_remove(Node* node, int value, int* status) {
    if (!status) {
        return node;
    }
    if (!node) {
        *status = 0;
        return NULL;
    }

    if (value < node->value) {
        node->left = node_remove(node->left, value, status);
    } else if (value > node->value) {
        node->right = node_remove(node->right, value, status);
    } else {
        *status = 1;
        if (!node->left) {
            Node* right = node->right;
            free(node);
            return right;
        }
        if (!node->right) {
            Node* left = node->left;
            free(node);
            return left;
        }

        Node* successor = find_min(node->right);
        node->value = successor->value;
        int tmp_status = 0;
        node->right = node_remove(node->right, successor->value, &tmp_status);
        *status = 1;
    }

    update_height(node);
    return rebalance(node);
}

static Node* find_min(Node* node) {
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

static int node_contains(const Node* node, int value) {
    if (!node) {
        return 0;
    }
    if (value == node->value) {
        return 1;
    }
    if (value < node->value) {
        return node_contains(node->left, value);
    }
    return node_contains(node->right, value);
}

static void inorder_traverse(const Node* node, int* out_values, size_t max_len, size_t* count) {
    if (!node || !out_values || !count || *count >= max_len) {
        return;
    }
    inorder_traverse(node->left, out_values, max_len, count);
    if (*count < max_len) {
        out_values[*count] = node->value;
        *count += 1U;
    } else {
        return;
    }
    inorder_traverse(node->right, out_values, max_len, count);
}

static void traverse_structure(const Node* node, size_t depth, size_t position,
                               AvlTreeChildType child_type, AvlTreeVisitFn visitor, void* user_data) {
    if (!node || !visitor) {
        return;
    }
    visitor(node->value, depth, position, child_type, user_data);
    traverse_structure(node->left, depth + 1U, position * 2U, AVL_CHILD_LEFT, visitor, user_data);
    traverse_structure(node->right, depth + 1U, position * 2U + 1U, AVL_CHILD_RIGHT, visitor, user_data);
}
