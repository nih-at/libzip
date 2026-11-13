#include "cmocka_utilities.h"
#include "shell/shell_int.h"

#include <string.h>

#define MAX_COMPLETIONS 64

struct test_data {
    /* input */
    char *line;

    /* expected output */
    char open_quote;
    size_t last_word_length;
    int expect_matches;
    char *prefix;
    char *completions[MAX_COMPLETIONS];
};

typedef struct test_data test_data_t;

/* clang-format off */

static argument_type_enum_entry_t my_enum[] = {
    {"a", {false}},
    {"b", {false}},
    {"ba", {false}},
    {"bb", {false}},
    {NULL, {0}}
};

const argument_type_t argument_type_my_enum_instance = ARGUMENT_TYPE_ENUM("compression_method", my_enum);
#define argument_type_my_enum &argument_type_my_enum_instance


option_definition_t test_options[] = {
    {256, "longlong", NULL, NULL, NULL},
    {'l', "long", NULL, NULL, NULL},
    {'a', "argument", NULL, NULL, argument_type_my_enum}
};

command_t help_command = {
    "help",
    NULL,
    NULL,
    0,
    NULL,
    0,
    NULL
};
command_t test_command = {
    "test",
    NULL,
    test_options,
    sizeof(test_options) / sizeof(test_options[0]),
    NULL,
    0,
    NULL
};

command_t *commands[] = {
    &help_command,
    &test_command
};

size_t n_commands = sizeof(commands) / sizeof(commands[0]);



test_data_t tests[] = {
    /* Complete command names. */
    {
        "t",
        '\0',
        1,
        1,
        NULL,
        {"test", NULL}
    },

    /* Complete all options. */
    {
        "test -",
        '\0',
        1,
        1,
        "-",
        {"-argument", "-long", "-longlong", "a", "l", NULL}
    },

    /* Complete long option argument. */
    {
        "test --argument b",
        '\0',
        1,
        1,
        NULL,
        {"b", "ba", "bb", NULL}
    },

    /* Complete short option argument. */
    {
        "test -lab",
        '\0',
        4,
        1,
        "-la",
        {"b", "ba", "bb", NULL}
    }
};

/* clang-format on */

void run_test(void **state) {
    test_data_t *test = (test_data_t *)*state;
    completions_t *completions;
    size_t last_word_length, n_matches;
    char open_quote;

    completions = complete(test->line, strlen(test->line), &last_word_length, &open_quote);

    if (!test->expect_matches) {
        assert_null(completions);
        return;
    }
    assert_non_null(completions);
    completions_sort(completions);
    assert_int_equal(open_quote, test->open_quote);
    assert_int_equal(last_word_length, test->last_word_length);
    assert_string_equal(completions->prefix, test->prefix);

    /* Count expected matches. */
    for (n_matches = 0; n_matches < MAX_COMPLETIONS && completions->matches[n_matches] != NULL; n_matches++) {
    }

    assert_int_equal(n_matches, completions->n_matches);
    for (size_t i = 0; i < n_matches; i++) {
        assert_string_equal(completions->matches[i], test->completions[i]);
    }
}

int main(void) {
    return run_test_table("shell_completion", run_test, (void *)tests, sizeof(tests[0]), sizeof(tests) / sizeof(tests[0]));
}
