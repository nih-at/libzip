#include <stdlib.h>

#include <cmocka.h>

typedef void (*test_table_function_t)(void **);

int run_test_table(const char* name, test_table_function_t test_function, void *data, size_t size, size_t count);

/* Unbelievably, CMocka's assert_string_equal can't handle NULL pointers, so replace them with versions that can. */

#undef assert_string_equal
#define assert_string_equal(a, b) \
    _assert_string_equal_save((a), (b), __FILE__, __LINE__)

void _assert_string_equal_save(const char * const a, const char * const b, const char * const file, const size_t line);

#undef assert_string_not_equal
#define assert_string_not_equal(a, b) \
    _assert_string_not_equal_save((a), (b), __FILE__, __LINE__)

void _assert_string_not_equal_save(const char * const a, const char * const b, const char * const file, const size_t line);
