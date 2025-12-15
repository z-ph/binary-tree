#include "test_framework.h"

#include <stdio.h>
#include <string.h>

static int current_case_failed = 0;

static const char* COLOR_RESET = "\033[0m";
static const char* COLOR_GREEN = "\033[32m";
static const char* COLOR_RED = "\033[31m";
static const char* COLOR_CYAN = "\033[36m";
static const char* COLOR_MAGENTA = "\033[35m";
static const char* COLOR_GRAY = "\033[90m";

static void log_with_color(FILE* stream, const char* color, const char* label, const char* message);
static void run_suite(const DescribeCase* suite, size_t* total_passed, size_t* total_failed);

void test_framework_fail(const char* expr, const char* file, int line) {
    current_case_failed = 1;
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s (%s:%d)", expr ? expr : "", file ? file : "?", line);
    test_log_error("ASSERT", buffer);
}

void test_log_success(const char* label, const char* message) {
    log_with_color(stdout, COLOR_GREEN, label, message);
}

void test_log_error(const char* label, const char* message) {
    log_with_color(stderr, COLOR_RED, label, message);
}

void test_log_info(const char* label, const char* message) {
    log_with_color(stdout, COLOR_CYAN, label, message);
}

void test_log_label(const char* label, const char* message) {
    log_with_color(stdout, COLOR_MAGENTA, label, message);
}

void test_log_divider(void) {
    printf("%s------------------------------------------------------------%s\n", COLOR_GRAY, COLOR_RESET);
}

int test_framework_run_all(const TestCase* cases, size_t count) {
    const DescribeCase suite = {
        "默认套件",
        cases,
        count,
    };
    return test_framework_run_suites(&suite, 1);
}

int test_framework_run_suites(const DescribeCase* suites, size_t count) {
    if (!suites || count == 0U) {
        test_log_error("FRAME", "未提供任何描述或测试。");
        return 1;
    }

    size_t total_passed = 0U;
    size_t total_failed = 0U;

    for (size_t i = 0; i < count; ++i) {
        run_suite(&suites[i], &total_passed, &total_failed);
    }

    char summary[128];
    snprintf(summary, sizeof(summary), "全部描述合计：%zu 通过，%zu 失败", total_passed, total_failed);
    if (total_failed == 0U) {
        test_log_info("SUMMARY", summary);
    } else {
        test_log_error("SUMMARY", summary);
    }
    test_log_divider();
    return total_failed == 0U ? 0 : 1;
}

int test_assert_true(int condition, const char* expr, const char* file, int line) {
    if (condition) {
        return 1;
    }
    test_framework_fail(expr, file, line);
    return 0;
}

int test_assert_equal_ll(long long expected, long long actual, const char* expr, const char* file, int line) {
    if (expected == actual) {
        return 1;
    }
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s | 期望=%lld，实际=%lld", expr ? expr : "", expected, actual);
    test_framework_fail(buffer, file, line);
    return 0;
}

static void log_with_color(FILE* stream, const char* color, const char* label, const char* message) {
    if (!stream) {
        return;
    }
    const char* safe_color = color ? color : COLOR_RESET;
    const char* safe_label = label ? label : "";
    const char* safe_message = message ? message : "";
    fprintf(stream, "%s[ %-7s ]%s %s\n", safe_color, safe_label, COLOR_RESET, safe_message);
}

static void run_suite(const DescribeCase* suite, size_t* total_passed, size_t* total_failed) {
    if (!suite) {
        return;
    }
    const char* suite_name = suite->name ? suite->name : "(未命名描述)";
    test_log_info("DESCR", suite_name);
    test_log_divider();

    if (!suite->tests || suite->test_count == 0U) {
        test_log_info("INFO", "该描述下没有测试用例。");
        test_log_divider();
        return;
    }

    size_t suite_passed = 0U;
    size_t suite_failed = 0U;

    for (size_t i = 0; i < suite->test_count; ++i) {
        const TestCase* test = &suite->tests[i];
        if (!test->func) {
            continue;
        }
        current_case_failed = 0;
        test_log_label("RUN", test->name ? test->name : "(匿名用例)");
        test->func();
        if (current_case_failed) {
            ++suite_failed;
            ++(*total_failed);
            test_log_error("FAILED", test->name ? test->name : "(匿名用例)");
        } else {
            ++suite_passed;
            ++(*total_passed);
            test_log_success("OK", test->name ? test->name : "(匿名用例)");
        }
        test_log_divider();
    }

    char message[128];
    snprintf(message, sizeof(message), "%s：%zu 通过，%zu 失败", suite_name, suite_passed, suite_failed);
    if (suite_failed == 0U) {
        test_log_info("DESCR", message);
    } else {
        test_log_error("DESCR", message);
    }
    test_log_divider();
}
