#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AvlTree AvlTree;

/**
 * 创建一个空的 AVL 树实例。
 *
 * @return 若分配成功则返回新树指针，失败返回 NULL。
 */
AvlTree* avl_tree_create(void);

/**
 * 释放树占用的全部内存。
 */
void avl_tree_destroy(AvlTree* tree);

/**
 * 插入一个值。若成功插入返回 1，若值已存在返回 0，若分配失败返回 -1。
 */
int avl_tree_insert(AvlTree* tree, int value);

/**
 * 删除指定值。找到并删除返回 1，未找到返回 0。
 */
int avl_tree_remove(AvlTree* tree, int value);

/**
 * 判断值是否存在，存在返回 1，否则返回 0。
 */
int avl_tree_contains(const AvlTree* tree, int value);

/**
 * 返回元素数量。
 */
size_t avl_tree_size(const AvlTree* tree);

/**
 * 判断是否为空树，空树返回 1，否则返回 0。
 */
int avl_tree_empty(const AvlTree* tree);

/**
 * 按中序遍历顺序写入 out_values，最多写入 max_len 个元素。
 *
 * @return 实际写入的数量。
 */
size_t avl_tree_in_order(const AvlTree* tree, int* out_values, size_t max_len);

typedef enum AvlTreeChildType {
    AVL_CHILD_ROOT = 0,
    AVL_CHILD_LEFT = 1,
    AVL_CHILD_RIGHT = 2
} AvlTreeChildType;

typedef void (*AvlTreeVisitFn)(int value, size_t depth, size_t position, AvlTreeChildType child_type,
                               void* user_data);

/**
 * 以前序遍历的方式回调每个节点，提供其深度与在当前层的相对序号，可用于自定义树形展示。
 */
void avl_tree_traverse_structure(const AvlTree* tree, AvlTreeVisitFn visitor, void* user_data);

#ifdef __cplusplus
}
#endif

#endif  // AVL_TREE_H
