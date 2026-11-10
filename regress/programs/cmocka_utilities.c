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
