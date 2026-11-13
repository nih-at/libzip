#include "cmocka_utilities.h"

#include <stdio.h>
#include <string.h>

int run_test_table(const char *name, test_table_function_t test_function, void *data, size_t size, size_t count) {
    struct CMUnitTest *tests;
    char test_name[256];

    tests = (struct CMUnitTest *)malloc(sizeof(struct CMUnitTest) * count);
    if (tests == NULL) {
        /* TODO: handle memory allocation failure */
        return -1;
    }

    for (size_t i = 0; i < count; i++) {
        snprintf(test_name, 256, "%s_%zu", name, i);
        tests[i].name = strdup(test_name);
        tests[i].test_func = test_function;
        tests[i].setup_func = NULL;
        tests[i].teardown_func = NULL;
        tests[i].initial_state = (void *)((char *)data + i * size);
    }

    int result = _cmocka_run_group_tests(name, tests, count, NULL, NULL);

    for (size_t i = 0; i < count; i++) {
        free((void *)tests[i].name);
    }
    free(tests);
    return result;
}


static bool string_equal_save_display_error(const char *const left, const char *const right) {
    if (left == NULL && right == NULL) {
        return true;
    }
    if (left == NULL || right == NULL) {
        cmocka_print_error("\"%s\" != \"%s\"\n", left ? left : "NULL", right ? right : "NULL");
        return false;
    }
    if (strcmp(left, right) == 0) {
        return true;
    }
    cmocka_print_error("\"%s\" != \"%s\"\n", left, right);
    return false;
}

static bool string_not_equal_save_display_error(const char *const left, const char *const right) {
    if (left == NULL && right == NULL) {
        cmocka_print_error("NULL == NULL\n");
        return false;
    }
    if (left == NULL || right == NULL) {
        return true;
    }
    if (strcmp(left, right) != 0) {
        return true;
    }
    cmocka_print_error("\"%s\" == \"%s\"\n", left, right);
    return false;
}

void _assert_string_equal_save(const char *const a, const char *const b, const char *const file, const size_t line) {
    if (!string_equal_save_display_error(a, b)) {
        _fail(file, (int)line);
    }
}

void _assert_string_not_equal_save(const char *const a, const char *const b, const char *const file, const size_t line) {
    if (!string_not_equal_save_display_error(a, b)) {
        _fail(file, (int)line);
    }
}
