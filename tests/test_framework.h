#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stddef.h>

typedef void (*TestFunc)(void);

typedef struct TestCase {
    const char* name;
    TestFunc func;
} TestCase;

typedef struct DescribeCase {
    const char* name;
    const TestCase* tests;
    size_t test_count;
} DescribeCase;

int test_framework_run_all(const TestCase* cases, size_t count);
int test_framework_run_suites(const DescribeCase* suites, size_t count);
void test_framework_fail(const char* expr, const char* file, int line);
void test_log_success(const char* label, const char* message);
void test_log_error(const char* label, const char* message);
void test_log_info(const char* label, const char* message);
void test_log_label(const char* label, const char* message);
void test_log_divider(void);
int test_assert_true(int condition, const char* expr, const char* file, int line);
int test_assert_equal_ll(long long expected, long long actual, const char* expr, const char* file, int line);

#define TEST_CASE(name) static void name(void)

#define TEST_ENTRY(fn) \
    { #fn, fn }

#define DESCRIBE_ENTRY(desc_name, test_array) \
    { desc_name, test_array, sizeof(test_array) / sizeof((test_array)[0]) }

#define TEST_ARRANGE(message) test_log_info("ARRANGE", message)
#define TEST_ACTION(message)  test_log_info("ACTION", message)
#define TEST_ASSERT(message)  test_log_info("ASSERT", message)

#define REQUIRE(condition)                                                       \
    do {                                                                         \
        if (!test_assert_true((condition), #condition, __FILE__, __LINE__)) {    \
            return;                                                              \
        }                                                                        \
    } while (0)

#define REQUIRE_EQ(expected, actual)                                                       \
    do {                                                                                   \
        long long _expected_value = (long long)(expected);                                  \
        long long _actual_value = (long long)(actual);                                      \
        if (!test_assert_equal_ll(_expected_value, _actual_value,                           \
                                  #actual " == " #expected, __FILE__, __LINE__)) {          \
            return;                                                                         \
        }                                                                                   \
    } while (0)

#endif  // TEST_FRAMEWORK_H
