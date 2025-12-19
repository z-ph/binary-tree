#include <stddef.h>
#include <stdio.h>

#include "avl_tree.h"
#include "test_framework.h"

TEST_CASE(test_insert_and_traversal) {
    TEST_ARRANGE("创建空树并准备遍历缓存");
    AvlTree* tree = avl_tree_create();
    REQUIRE(tree != NULL);
    REQUIRE(avl_tree_empty(tree) == 1);
    int values[4] = {0};
    test_log_info("DATA", "插入序列: [10,10,5,15]，遍历缓冲区大小=4");

    TEST_ACTION("插入多个节点并执行中序遍历");
    int status_insert_10 = avl_tree_insert(tree, 10);
    int status_insert_dup = avl_tree_insert(tree, 10);  // 重复元素会被忽略
    int status_insert_5 = avl_tree_insert(tree, 5);
    int status_insert_15 = avl_tree_insert(tree, 15);
    size_t size_after_insert = avl_tree_size(tree);
    int contains_5 = avl_tree_contains(tree, 5);
    int contains_42 = avl_tree_contains(tree, 42);
    size_t traversal_count = avl_tree_in_order(tree, values, 4);

    TEST_ASSERT("验证插入状态、大小与遍历结果");
    REQUIRE_EQ(1, status_insert_10);
    REQUIRE_EQ(0, status_insert_dup);
    REQUIRE_EQ(1, status_insert_5);
    REQUIRE_EQ(1, status_insert_15);
    REQUIRE_EQ(3U, size_after_insert);
    REQUIRE_EQ(1, contains_5);
    REQUIRE_EQ(0, contains_42);
    REQUIRE_EQ(3U, traversal_count);
    REQUIRE(values[0] == 5);
    REQUIRE(values[1] == 10);
    REQUIRE(values[2] == 15);
    char result_summary[160];
    snprintf(result_summary, sizeof(result_summary),
             "结果: 插入状态=(%d,%d,%d,%d)，长度=%zu，中序输出=[%d,%d,%d]", status_insert_10, status_insert_dup,
             status_insert_5, status_insert_15, size_after_insert, values[0], values[1], values[2]);
    test_log_info("RESULT", result_summary);

    avl_tree_destroy(tree);
}

TEST_CASE(test_removals) {
    TEST_ARRANGE("创建树并插入多形态节点");
    AvlTree* tree = avl_tree_create();
    REQUIRE(tree != NULL);
    const int items[] = {30, 20, 40, 10, 25, 35, 50, 5, 15};
    const size_t item_count = sizeof(items) / sizeof(items[0]);
    for (size_t i = 0; i < item_count; ++i) {
        REQUIRE_EQ(1, avl_tree_insert(tree, items[i]));
    }
    REQUIRE_EQ(item_count, avl_tree_size(tree));
    test_log_info("DATA", "初始节点序列: [30,20,40,10,25,35,50,5,15]");

    TEST_ACTION("删除不同位置的节点并最终清空");
    int remove_20 = avl_tree_remove(tree, 20);  // 删除拥有两个子节点的节点
    int contains_20 = avl_tree_contains(tree, 20);
    size_t size_after_remove_20 = avl_tree_size(tree);

    int remove_leaf = avl_tree_remove(tree, 5);
    int remove_root = avl_tree_remove(tree, 30);
    int remove_missing = avl_tree_remove(tree, 999);

    int traversal_positive = 1;
    int removal_loop_success = 1;
    while (avl_tree_empty(tree) == 0) {
        int values[16] = {0};
        size_t count = avl_tree_in_order(tree, values, 16);
        if (count == 0U) {
            traversal_positive = 0;
            break;
        }
        if (avl_tree_remove(tree, values[0]) != 1) {
            removal_loop_success = 0;
            break;
        }
    }
    size_t final_size = avl_tree_size(tree);

    TEST_ASSERT("确认删除行为符合预期");
    REQUIRE_EQ(1, remove_20);
    REQUIRE_EQ(0, contains_20);
    REQUIRE_EQ(item_count - 1, size_after_remove_20);
    REQUIRE_EQ(1, remove_leaf);
    REQUIRE_EQ(1, remove_root);
    REQUIRE_EQ(0, remove_missing);
    REQUIRE(traversal_positive == 1);
    REQUIRE(removal_loop_success == 1);
    REQUIRE_EQ(0U, final_size);
    char removal_summary[256];
    snprintf(removal_summary, sizeof(removal_summary),
             "删除流程: remove20=%d(contains=%d,size=%zu), remove5=%d, remove30=%d, remove999=%d, 最终大小=%zu",
             remove_20, contains_20, size_after_remove_20, remove_leaf, remove_root, remove_missing, final_size);
    test_log_info("RESULT", removal_summary);

    avl_tree_destroy(tree);
}

TEST_CASE(test_inorder_buffer_limit) {
    TEST_ARRANGE("创建空树准备写入大量顺序节点");
    AvlTree* tree = avl_tree_create();
    REQUIRE(tree != NULL);
    test_log_info("DATA", "插入升序范围 [0,9]，遍历缓冲区大小=4");

    TEST_ACTION("插入升序数据并读取有限缓冲区");
    int insert_status = 1;
    for (int i = 0; i < 10; ++i) {
        if (avl_tree_insert(tree, i) != 1) {
            insert_status = 0;
            break;
        }
    }
    int buffer[4] = {0};
    size_t count = avl_tree_in_order(tree, buffer, 4);

    TEST_ASSERT("只写入缓冲区可承载的元素并保持顺序");
    REQUIRE(insert_status == 1);
    REQUIRE_EQ(4U, count);
    for (size_t i = 0; i < count; ++i) {
        REQUIRE(buffer[i] == (int)i);
    }
    char buffer_summary[128];
    snprintf(buffer_summary, sizeof(buffer_summary), "读取了 %zu 个元素，缓冲区内容=[%d,%d,%d,%d]", count, buffer[0], buffer[1],
             buffer[2], buffer[3]);
    test_log_info("RESULT", buffer_summary);

    avl_tree_destroy(tree);
}

typedef struct {
    int value;
    size_t depth;
    size_t position;
    AvlTreeChildType type;
} NodeCapture;

typedef struct {
    NodeCapture* entries;
    size_t capacity;
    size_t count;
} CaptureContext;

static void structure_collect_callback(int value, size_t depth, size_t position, AvlTreeChildType type,
                                       void* user_data) {
    CaptureContext* ctx = (CaptureContext*)user_data;
    if (!ctx || !ctx->entries || ctx->count >= ctx->capacity) {
        return;
    }
    NodeCapture* slot = &ctx->entries[ctx->count++];
    slot->value = value;
    slot->depth = depth;
    slot->position = position;
    slot->type = type;
}

TEST_CASE(test_structure_traversal) {
    AvlTree* tree = avl_tree_create();
    REQUIRE(tree != NULL);
    const int values[] = {8, 4, 12, 2, 6, 10, 14};
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
        REQUIRE_EQ(1, avl_tree_insert(tree, values[i]));
    }
    test_log_info("DATA", "插入序列: [8,4,12,2,6,10,14]，验证结构遍历输出");
    NodeCapture captures[16] = {0};
    CaptureContext ctx = {
        .entries = captures,
        .capacity = 16,
        .count = 0U,
    };
    avl_tree_traverse_structure(tree, structure_collect_callback, &ctx);
    REQUIRE(ctx.count >= 5U);
    REQUIRE(captures[0].value == 8);
    REQUIRE(captures[0].depth == 0U);
    REQUIRE(captures[0].type == AVL_CHILD_ROOT);
    REQUIRE(captures[0].position == 0U);
    REQUIRE(captures[1].value == 4);
    REQUIRE(captures[1].depth == 1U);
    REQUIRE(captures[1].position == 0U);
    REQUIRE(captures[1].type == AVL_CHILD_LEFT);
    REQUIRE(captures[2].value == 2);
    REQUIRE(captures[2].depth == 2U);
    REQUIRE(captures[2].position == 0U);
    REQUIRE(captures[3].value == 6);
    REQUIRE(captures[3].position == 1U);
    REQUIRE(captures[4].value == 12);
    REQUIRE(captures[4].type == AVL_CHILD_RIGHT);
    REQUIRE(captures[4].position == 1U);
    char structure_summary[200];
    snprintf(structure_summary, sizeof(structure_summary),
             "捕获节点数=%zu，前五项=[(v=%d,d=%zu,pos=%zu),(v=%d,d=%zu,pos=%zu),(v=%d,d=%zu,pos=%zu),"
             "(v=%d,d=%zu,pos=%zu),(v=%d,d=%zu,pos=%zu)]",
             ctx.count, captures[0].value, captures[0].depth, captures[0].position, captures[1].value,
             captures[1].depth, captures[1].position, captures[2].value, captures[2].depth, captures[2].position,
             captures[3].value, captures[3].depth, captures[3].position, captures[4].value, captures[4].depth,
             captures[4].position);
    test_log_info("RESULT", structure_summary);
    avl_tree_destroy(tree);
}

int main(void) {
    static const TestCase avl_basic_cases[] = {
        TEST_ENTRY(test_insert_and_traversal),
        TEST_ENTRY(test_removals),
        TEST_ENTRY(test_inorder_buffer_limit),
        TEST_ENTRY(test_structure_traversal),
    };

    static const DescribeCase suites[] = {
        DESCRIBE_ENTRY("AVL 树基础行为", avl_basic_cases),
    };

    return test_framework_run_suites(suites, sizeof(suites) / sizeof(suites[0]));
}
