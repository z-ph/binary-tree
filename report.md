# 平衡二叉树（AVL）抽象数据类型实验报告

## 1. 设计任务、要求及实验环境
- **设计任务**：围绕教材第 3 章“抽象数据类型的实现”，选择难度系数 1.3 的平衡二叉树（AVL Tree）作为目标 ADT，完成类型定义、全部基本操作、交互式 CLI 以及自动化单元测试。
- **主要要求**
  1. 采用 C17 标准及 4 空格缩进实现，接口放置于 `include/avl_tree.h`，实现位于 `src/avl_tree.c`。
  2. 提供可交互的演示程序 `src/main.c`，具备插入、删除、查找、树形展示和自动演示等功能。
  3. 在 `tests/` 下实现覆盖插入、删除、遍历与结构回调的测试套件，所有更改须通过 `./run_tests.sh`。
  4. 输出规范的实验报告，总结设计、调试与思考。
- **软件与硬件环境**：macOS + zsh，GCC 13（`-std=c11 -Wall -Wextra -Wpedantic -Werror`），本地构建脚本 `run_cli.sh`、`run_tests.sh`，终端支持 ANSI 彩色输出。

## 2. 抽象数据类型定义
记 `ElemSet` 为整型集合，AVL 树维护严格递增序列，任意节点的左右子树高度差绝对值不超过 1。

```
ADT AvlTree {
    数据对象：D = { ai | ai ∈ ElemSet, 1 ≤ i ≤ n, ai < ai+1 }
    数据关系：若 aj 在节点 p 的左子树，则 aj < ap；若在右子树，则 aj > ap
    基本操作：
        AvlTree* Create()
        void Destroy(&T)
        int Insert(&T, value)
        int Remove(&T, value)
        int Contains(T, value)
        size_t Size(T)
        int Empty(T)
        size_t InOrder(T, out[], max_len)
        void TraverseStructure(T, visit, user_data)
}
```

各操作含义与教材线性表示例一致：`Create/Destroy` 负责生命周期管理，`Insert/Remove` 修改集合并维持平衡，`Contains/Size/Empty` 查询状态，`InOrder` 输出升序序列，`TraverseStructure` 面向 CLI 绘制树形结构。

## 3. 存储结构与基本算法

### 3.1 存储结构
- `Node`：包含 `int value`、`int height` 以及左右子指针。`height` 用于快速计算平衡因子。
- `struct AvlTree`：维护 `Node* root` 和 `size_t size`。该结构位于 `src/avl_tree.c` 内部，外部仅通过不透明指针操控。

### 3.2 基本操作实现
- **Create/Destroy**：`avl_tree_create` 通过 `malloc` 分配树对象并初始化指针；`avl_tree_destroy` 深度优先释放节点（后序遍历），最后释放树本身。
- **Height/Balancing**：辅助函数 `node_height`、`update_height`、`balance_factor`、`rotate_left/right` 组合使用。在插入或删除后更新当前节点高度并检查平衡；若左右子树高度相差 2，则执行 LL/LR/RR/RL 旋转。
- **Insert**：`node_insert` 递归地按 BST 规则定位插入位置，遇到重复值直接返回。插入成功后沿递归栈向上更新高度与平衡；若 `malloc` 失败通过状态码 `-1` 传递错误。
- **Remove**：`node_remove` 递归查找目标。若命中节点：
  1. 只有一个子树则将子指针上提。
  2. 有两个子树时以右子树最小节点（`find_min`）替换当前值，再删除后继节点。
  删除成功后同样更新高度与旋转保持平衡。
- **Contains/Size/Empty**：常规 BST 搜索与属性读取，时间复杂度 O(log n)。
- **InOrder**：递归中序遍历，写入调用者提供的缓冲区；当缓冲区满时提前结束，保证不会越界。
- **TraverseStructure**：以前序遍历回调每个节点，将节点深度、同层相对位置以及左右子标记传递给 CLI，从而渲染树形视图。
- **CLI 交互**：`src/main.c` 使用上述接口实现 5 个菜单项——插入、删除、查找、树形展示与批量自动演示。自动演示会生成随机样本并在每一步暂停，帮助观察平衡调整的过程。

### 3.3 存储结构与操作源码（节选）
以下代码直接摘自 `src/avl_tree.c`，展示不带头结点的二叉链式表示及其基本操作实现：

```c
typedef struct Node {
    int value;          /* 节点存储的关键字 */
    int height;         /* 以本节点为根的子树高度 */
    struct Node* left;  /* 左子树指针 */
    struct Node* right; /* 右子树指针 */
} Node;

struct AvlTree {
    Node* root;   /* 根指针 */
    size_t size;  /* 树中元素个数 */
};

static Node* node_create(int value);      /* 生成新节点 */
static void node_destroy(Node* node);     /* 后序释放节点 */
static int node_height(const Node* node); /* 读取节点高度 */
static void update_height(Node* node);    /* 重新计算节点高度 */
static int balance_factor(const Node* node); /* 返回平衡因子 */
static Node* rotate_left(Node* node);     /* 左旋 */
static Node* rotate_right(Node* node);    /* 右旋 */
static Node* rebalance(Node* node);       /* 根据平衡因子旋转 */
static Node* node_insert(Node* node, int value, int* status); /* 递归插入 */
static Node* node_remove(Node* node, int value, int* status); /* 递归删除 */
static Node* find_min(Node* node);        /* 查找右子树最小节点 */
static int node_contains(const Node* node, int value); /* 查找元素 */
static void inorder_traverse(const Node* node, int* out_values, size_t max_len, size_t* count); /* 中序遍历 */
static void traverse_structure(const Node* node, size_t depth, size_t position,
                               AvlTreeChildType child_type, AvlTreeVisitFn visitor, void* user_data); /* 结构遍历 */

AvlTree* avl_tree_create(void) { /* 构造一棵空 AVL 树 */
    AvlTree* tree = (AvlTree*)malloc(sizeof(AvlTree));
    if (!tree) {
        return NULL;
    }
    tree->root = NULL;
    tree->size = 0;
    return tree;
}

void avl_tree_destroy(AvlTree* tree) { /* 销毁 AVL 树并释放内存 */
    if (!tree) {
        return;
    }
    node_destroy(tree->root);
    free(tree);
}

int avl_tree_insert(AvlTree* tree, int value) { /* 插入元素 */
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

int avl_tree_remove(AvlTree* tree, int value) { /* 删除元素 */
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

int avl_tree_contains(const AvlTree* tree, int value) { /* 判定元素是否存在 */
    if (!tree) {
        return 0;
    }
    return node_contains(tree->root, value);
}

size_t avl_tree_size(const AvlTree* tree) { /* 返回元素数量 */
    return tree ? tree->size : 0U;
}

int avl_tree_empty(const AvlTree* tree) { /* 判断树是否为空 */
    return (tree == NULL) || (tree->size == 0U);
}

size_t avl_tree_in_order(const AvlTree* tree, int* out_values, size_t max_len) { /* 中序遍历接口 */
    if (!tree || !out_values || max_len == 0U) {
        return 0U;
    }
    size_t count = 0U;
    inorder_traverse(tree->root, out_values, max_len, &count);
    return count;
}

void avl_tree_traverse_structure(const AvlTree* tree, AvlTreeVisitFn visitor, void* user_data) { /* 结构遍历 */
    if (!tree || !visitor || !tree->root) {
        return;
    }
    traverse_structure(tree->root, 0U, 0U, AVL_CHILD_ROOT, visitor, user_data);
}
```

上面展示了所有对外可见的 API、底层链式存储结构定义以及与 CLI/测试共用的遍历接口，完整反映了“所选择的存储结构及在其上实现的基本操作”。

## 4. 程序清单与测试结果
- `include/avl_tree.h`：公开 API、节点访问回调定义。
- `src/avl_tree.c`：AVL 树核心逻辑，包含旋转、插入、删除与遍历。
- `src/main.c`：终端 CLI，实现批量插入/删除、树形展示（带颜色和层级对齐）、中序输出以及随机自动脚本。
- `tests/test_framework.*`：轻量级测试框架，提供 `TEST_CASE`/`REQUIRE` 宏与彩色日志。
- `tests/avl_tree_tests.c`：四个测试用例覆盖插入与遍历、删除多种节点形态、缓冲区上限以及结构遍历回调。

**自动化测试**

```
$ ./run_tests.sh
DESCR  AVL 树基础行为
RUN    test_insert_and_traversal ... OK
RUN    test_removals              ... OK
RUN    test_inorder_buffer_limit  ... OK
RUN    test_structure_traversal   ... OK
SUMMARY 全部描述合计：4 通过，0 失败
```

所有测试均通过，验证了库与 CLI 共享的核心逻辑。

**CLI 运行要点**
1. 执行 `./run_cli.sh` 编译后进入交互菜单。
2. `树形展示` 会绘制多行结构并附带中序结果，方便人工检查平衡。
3. `批量自动测试` 通过 `random_in_range` 生成数据，每个动作需要按 Enter 确认，适合课堂演示或调试。

## 5. 基本操作时间复杂度

| 操作 | 平均/最坏复杂度 | 说明 |
| --- | --- | --- |
| `Create` / `Destroy` | O(1) / O(n) | 创建仅初始化指针；销毁需遍历释放全部节点 |
| `Insert` | O(log n) | AVL 保持高度 O(log n)，插入后至多一次双旋 |
| `Remove` | O(log n) | 同插入，删除后需要更新高度并可能旋转 |
| `Contains` | O(log n) | 标准 BST 查找 |
| `Size` / `Empty` | O(1) | 直接读取 `size` 字段 |
| `InOrder` | O(min(n, max_len)) | 受调用方缓冲区上限约束 |
| `TraverseStructure` | O(n) | 前序遍历所有节点，为 CLI 提供可视化数据 |

与顺序存储或普通链式结构相比，AVL 在保持序列有序的同时显著降低了查找与更新的最坏复杂度，代价是每次更新需要维护高度并执行旋转。

## 6. 调试过程与经验
1. **高度维护**：早期版本在删除操作中遗漏了 `update_height`，导致 `balance_factor` 计算过期。通过在每次递归返回前统一调用 `update_height` + `rebalance` 解决。
2. **遍历缓冲区保护**：`avl_tree_in_order` 需要当 `max_len` 为 0 时直接返回，避免用户传入空缓冲区导致访问越界。测试 `test_inorder_buffer_limit` 覆盖了该路径。
3. **CLI 渲染**：由于树形打印需要按层对齐，设计了 `TreePrintContext` 动态数组保存 `(value, depth, position)`，再统一渲染。调试过程中在插入大量节点时触发了 `realloc` 失败分支，因而在渲染函数中增加了内存检查与报错信息。
4. **自动演示**：为了避免全自动造成界面刷屏，所有随机插入/删除均要求按 Enter 继续，方便逐步观察旋转效果。

## 7. 思考与总结
本实验通过实现 AVL 树完整 ADT，从接口设计、存储结构、平衡算法到交互与测试的串联，验证了“抽象类型 + 多表示 + 自动化验证”的工程流程。与线性表示例相比，AVL 的难点在于同时维护排序与高度信息，需要在每个递归返回点保持节点一致性；而 CLI 与测试框架则提供了观察与回归的抓手。

后续可考虑：
1. 为 CLI 增加批量导入/导出、可视化导出（DOT 图）等功能；
2. 扩展 ADT，增加 `lower_bound`/`upper_bound` 等序列操作；
3. 引入性能测试，比较与红黑树或跳表的差异。

通过本次实验，对抽象数据类型的精确定义、存储结构选择以及代码与文档并重的流程有了更系统的理解。
