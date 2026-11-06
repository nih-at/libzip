#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include <cmocka.h>

typedef void (*test_table_function_t)(void **);

int run_test_table(const char *name, test_table_function_t test_function, void *data, size_t size, size_t count);
