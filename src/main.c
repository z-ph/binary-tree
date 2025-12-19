#include "avl_tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct TreeNodeEntry {
    int value;
    size_t depth;
    size_t position;
} TreeNodeEntry;

typedef struct TreePrintContext {
    TreeNodeEntry* entries;
    size_t count;
    size_t capacity;
    size_t max_depth;
    int error;
} TreePrintContext;

static void print_menu(const AvlTree* tree);
static void wait_for_enter(const char* message);
static int read_int(const char* prompt, int* out_value);
static size_t read_int_list(const char* prompt, int* values, size_t max_count);
static void handle_insert(AvlTree* tree);
static void handle_remove(AvlTree* tree);
static void handle_search(const AvlTree* tree);
static void handle_tree_view(const AvlTree* tree);
static void handle_auto_demo(void);
static void tree_print_callback(int value, size_t depth, size_t position, AvlTreeChildType child_type,
                                void* user_data);
static void print_tree_structure(const AvlTree* tree);
static void tree_print_context_init(TreePrintContext* ctx);
static void tree_print_context_destroy(TreePrintContext* ctx);
static int tree_print_context_add(TreePrintContext* ctx, int value, size_t depth, size_t position);
static void tree_print_render_levels(const TreePrintContext* ctx);
static void print_spaces(size_t count);
static size_t tree_max_value_width(const TreePrintContext* ctx);
static void print_value_cell(const char* text, size_t width);
static void display_in_order(const AvlTree* tree);
static int random_in_range(int min, int max);
static const char* color_for_depth(size_t depth);

int main(void) {
    system("chcp 65001 > nul");  // Windows 下设置 UTF-8 编码页以支持中文输出
    AvlTree* tree = avl_tree_create();
    if (!tree) {
        fprintf(stderr, "创建 AVL 树失败，内存不足。\n");
        return EXIT_FAILURE;
    }

    int running = 1;
    while (running) {
        print_menu(tree);
        printf("请选择操作: ");
        char buffer[32];
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break;
        }
        int choice = atoi(buffer);
        switch (choice) {
            case 1:
                handle_insert(tree);
                break;
            case 2:
                handle_remove(tree);
                break;
            case 3:
                handle_search(tree);
                break;
            case 4:
                handle_tree_view(tree);
                break;
            case 5:
                handle_auto_demo();
                break;
            case 6:
                running = 0;
                continue;
            default:
                printf("无效选项，请重新输入。\n");
                break;
        }
        wait_for_enter(NULL);
    }

    avl_tree_destroy(tree);
    printf("已退出 CLI。\n");
    return EXIT_SUCCESS;
}

static void print_menu(const AvlTree* tree) {
    size_t current_size = tree ? avl_tree_size(tree) : 0U;
    printf("\n================ AVL CLI ================\n");
    printf("1. 插入节点\n");
    printf("2. 删除节点\n");
    printf("3. 查询节点\n");
    printf("4. 树形展示（含中序遍历）\n");
    printf("5. 批量自动测试\n");
    printf("6. 退出\n");
    printf("当前节点数量: %zu\n", current_size);
    printf("========================================\n");
}

static void wait_for_enter(const char* message) {
    if (message && *message) {
        printf("%s", message);
    } else {
        printf("按 Enter 执行下一步...");
    }
    int ch = 0;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

static int read_int(const char* prompt, int* out_value) {
    if (!out_value) {
        return 0;
    }
    printf("%s", prompt);
    char buffer[64];
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0;
    }
    char* endptr = NULL;
    long value = strtol(buffer, &endptr, 10);
    if (endptr == buffer) {
        printf("输入不是有效整数。\n");
        return 0;
    }
    if (value < -2147483648L || value > 2147483647L) {
        printf("整数超出范围。\n");
        return 0;
    }
    *out_value = (int)value;
    return 1;
}

static size_t read_int_list(const char* prompt, int* values, size_t max_count) {
    if (!values || max_count == 0U) {
        return 0U;
    }
    printf("%s", prompt);
    char buffer[256];
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return 0U;
    }
    size_t count = 0U;
    char* token = strtok(buffer, " \t\r\n");
    while (token && count < max_count) {
        char* endptr = NULL;
        long value = strtol(token, &endptr, 10);
        if (endptr != token) {
            if (value >= -2147483648L && value <= 2147483647L) {
                values[count++] = (int)value;
            } else {
                printf("警告: %s 超出范围，已忽略。\n", token);
            }
        }
        token = strtok(NULL, " \t\r\n");
    }
    return count;
}

static void handle_insert(AvlTree* tree) {
    int values[64];
    size_t count = read_int_list("请输入要插入的整数（空格分隔，可多个）: ", values, 64);
    if (count == 0U) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        int result = avl_tree_insert(tree, values[i]);
        if (result == 1) {
            printf("成功插入 %d。\n", values[i]);
        } else if (result == 0) {
            printf("值 %d 已存在，未重复插入。\n", values[i]);
        } else {
            printf("插入 %d 失败：内存不足。\n", values[i]);
        }
        print_tree_structure(tree);
    }
    printf("当前元素数量: %zu\n", avl_tree_size(tree));
}

static void handle_remove(AvlTree* tree) {
    int values[64];
    size_t count = read_int_list("请输入要删除的整数（空格分隔，可多个）: ", values, 64);
    if (count == 0U) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        int result = avl_tree_remove(tree, values[i]);
        if (result == 1) {
            printf("成功删除 %d。\n", values[i]);
        } else {
            printf("未找到 %d。\n", values[i]);
        }
        print_tree_structure(tree);
    }
    printf("当前元素数量: %zu\n", avl_tree_size(tree));
}

static void handle_search(const AvlTree* tree) {
    int value = 0;
    if (!read_int("请输入要查询的整数: ", &value)) {
        return;
    }
    if (avl_tree_contains(tree, value)) {
        printf("%d 存在于树中。\n", value);
    } else {
        printf("%d 不存在。\n", value);
    }
    print_tree_structure(tree);
}

static void handle_tree_view(const AvlTree* tree) {
    print_tree_structure(tree);
    display_in_order(tree);
}

static void handle_auto_demo(void) {
    printf("批量自动测试（随机数据，每一步需手动确认）\n");
    AvlTree* demo = avl_tree_create();
    if (!demo) {
        printf("无法创建临时树。\n");
        return;
    }

    const size_t max_ops = 32;
    size_t insert_count = (size_t)random_in_range(5, 10);
    if (insert_count > max_ops) {
        insert_count = max_ops;
    }
    int insert_values[32];
    for (size_t i = 0; i < insert_count; ++i) {
        insert_values[i] = random_in_range(-50, 50);
    }

    printf("步骤 1：依次插入随机节点，共 %zu 个。\n", insert_count);
    for (size_t i = 0; i < insert_count; ++i) {
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "  准备插入 %d，按 Enter 执行...", insert_values[i]);
        wait_for_enter(prompt);
        int status = avl_tree_insert(demo, insert_values[i]);
        printf("    插入 %d -> %s\n", insert_values[i], status == 1 ? "成功" : "失败/重复");
        print_tree_structure(demo);
    }
    printf("步骤 1 完成，当前数量: %zu\n", avl_tree_size(demo));

    printf("步骤 2：依次尝试删除随机节点。\n");
    int delete_values[32];
    size_t delete_count = 0U;
    size_t existing_to_delete =
        insert_count > 0 ? (size_t)random_in_range(1, (int)(insert_count < 5 ? insert_count : 5)) : 0U;
    for (size_t i = 0; i < existing_to_delete && delete_count < max_ops; ++i) {
        delete_values[delete_count++] = insert_values[i];
    }
    if (delete_count < max_ops) {
        delete_values[delete_count++] = random_in_range(-50, 50);
    }
    for (size_t i = 0; i < delete_count; ++i) {
        int value = delete_values[i];
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "  准备删除 %d，按 Enter 执行...", value);
        wait_for_enter(prompt);
        printf("    删除 %d -> %s\n", value, avl_tree_remove(demo, value) ? "成功" : "失败");
        print_tree_structure(demo);
    }
    printf("步骤 2 完成，当前数量: %zu\n", avl_tree_size(demo));

    wait_for_enter("步骤 3：随机查询若干节点，按 Enter 开始...");
    int search_targets[2];
    search_targets[0] = (insert_count > 0) ? insert_values[insert_count - 1U] : random_in_range(-50, 50);
    search_targets[1] = random_in_range(-50, 50);
    for (size_t i = 0; i < 2; ++i) {
        int value = search_targets[i];
        printf("  查询 %d -> %s\n", value, avl_tree_contains(demo, value) ? "存在" : "不存在");
    }
    print_tree_structure(demo);

    wait_for_enter("步骤 4：展示树形结构和中序遍历，按 Enter 开始...");
    print_tree_structure(demo);
    display_in_order(demo);

    avl_tree_destroy(demo);
    printf("自动测试流程结束。\n");
}

static void tree_print_callback(int value, size_t depth, size_t position, AvlTreeChildType child_type,
                                void* user_data) {
    (void)child_type;
    TreePrintContext* ctx = (TreePrintContext*)user_data;
    if (!ctx) {
        return;
    }
    if (!tree_print_context_add(ctx, value, depth, position)) {
        ctx->error = 1;
    }
}

static void print_tree_structure(const AvlTree* tree) {
    printf("树形结构：\n");
    if (!tree || avl_tree_empty(tree)) {
        printf("  (空)\n");
        return;
    }
    TreePrintContext ctx;
    tree_print_context_init(&ctx);
    avl_tree_traverse_structure(tree, tree_print_callback, &ctx);
    if (ctx.error || ctx.count == 0U) {
        printf("  (内存不足或无节点)\n");
    } else {
        tree_print_render_levels(&ctx);
    }
    tree_print_context_destroy(&ctx);
}

static void display_in_order(const AvlTree* tree) {
    if (!tree) {
        printf("无法输出中序遍历：树不存在。\n");
        return;
    }
    size_t size = avl_tree_size(tree);
    int* values = (int*)calloc(size ? size : 1U, sizeof(int));
    if (!values) {
        printf("内存不足，无法输出中序遍历。\n");
        return;
    }
    size_t count = avl_tree_in_order(tree, values, size);
    printf("中序遍历结果 (%zu 个): ", count);
    for (size_t i = 0; i < count; ++i) {
        printf("%d ", values[i]);
    }
    printf("\n");
    free(values);
}

static void tree_print_context_init(TreePrintContext* ctx) {
    if (!ctx) {
        return;
    }
    ctx->entries = NULL;
    ctx->count = 0U;
    ctx->capacity = 0U;
    ctx->max_depth = 0U;
    ctx->error = 0;
}

static void tree_print_context_destroy(TreePrintContext* ctx) {
    if (!ctx) {
        return;
    }
    free(ctx->entries);
    ctx->entries = NULL;
    ctx->count = 0U;
    ctx->capacity = 0U;
    ctx->max_depth = 0U;
    ctx->error = 0;
}

static int tree_print_context_add(TreePrintContext* ctx, int value, size_t depth, size_t position) {
    if (!ctx) {
        return 0;
    }
    if (ctx->count == ctx->capacity) {
        size_t new_cap = ctx->capacity == 0U ? 16U : ctx->capacity * 2U;
        TreeNodeEntry* new_entries = (TreeNodeEntry*)realloc(ctx->entries, new_cap * sizeof(TreeNodeEntry));
        if (!new_entries) {
            return 0;
        }
        ctx->entries = new_entries;
        ctx->capacity = new_cap;
    }
    ctx->entries[ctx->count].value = value;
    ctx->entries[ctx->count].depth = depth;
    ctx->entries[ctx->count].position = position;
    ctx->count += 1U;
    if (depth > ctx->max_depth) {
        ctx->max_depth = depth;
    }
    return 1;
}

static void tree_print_render_levels(const TreePrintContext* ctx) {
    if (!ctx || ctx->count == 0U) {
        printf("  (空)\n");
        return;
    }
    size_t value_width = tree_max_value_width(ctx);
    size_t cell_width = value_width + 2U;
    size_t unit_count = (size_t)1 << (ctx->max_depth + 1U);
    for (size_t depth = 0; depth <= ctx->max_depth; ++depth) {
        int* unit_values = (int*)calloc(unit_count, sizeof(int));
        unsigned char* unit_present = (unsigned char*)calloc(unit_count, sizeof(unsigned char));
        if (!unit_values || !unit_present) {
            printf("  (内存不足)\n");
            free(unit_values);
            free(unit_present);
            return;
        }
        for (size_t i = 0; i < ctx->count; ++i) {
            if (ctx->entries[i].depth != depth) {
                continue;
            }
            size_t shift = ctx->max_depth - depth;
            size_t unit = ((ctx->entries[i].position * 2U) + 1U) << shift;
            if (unit == 0U) {
                continue;
            }
            size_t idx = unit - 1U;
            if (idx >= unit_count) {
                continue;
            }
            unit_values[idx] = ctx->entries[i].value;
            unit_present[idx] = 1U;
        }
        for (size_t idx = 0; idx < unit_count; ++idx) {
            if (unit_present[idx]) {
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%d", unit_values[idx]);
                const char* color = color_for_depth(depth);
                printf("%s", color);
                print_value_cell(buffer, cell_width);
                printf("\033[0m");
            } else {
                print_spaces(cell_width);
            }
        }
        printf("\n");
        free(unit_values);
        free(unit_present);
    }
}

static void print_spaces(size_t count) {
    for (size_t i = 0; i < count; ++i) {
        putchar(' ');
    }
}

static size_t tree_max_value_width(const TreePrintContext* ctx) {
    if (!ctx || ctx->count == 0U) {
        return 1U;
    }
    size_t max_width = 1U;
    for (size_t i = 0; i < ctx->count; ++i) {
        char buffer[32];
        int len = snprintf(buffer, sizeof(buffer), "%d", ctx->entries[i].value);
        if (len > 0 && (size_t)len > max_width) {
            max_width = (size_t)len;
        }
    }
    return max_width;
}

static void print_value_cell(const char* text, size_t width) {
    if (!text) {
        print_spaces(width);
        return;
    }
    size_t len = strlen(text);
    if (len >= width) {
        printf("%s", text);
        return;
    }
    size_t padding = width - len;
    size_t left = padding / 2U;
    size_t right = padding - left;
    print_spaces(left);
    printf("%s", text);
    print_spaces(right);
}

static int random_in_range(int min, int max) {
    if (max <= min) {
        return min;
    }
    static int seeded = 0;
    if (!seeded) {
        seeded = 1;
        srand((unsigned)time(NULL));
    }
    return min + rand() % (max - min + 1);
}

static const char* color_for_depth(size_t depth) {
    return (depth % 2U == 0U) ? "\033[36m" : "\033[32m";
}
