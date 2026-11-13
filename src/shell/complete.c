#include "compat/src_compat.h"
#include "shell_int.h"

#include <stdlib.h>
#include <string.h>

words_t *completion_words = NULL;

completions_t *completions_new(const char *prefix, size_t prefix_length) {
    completions_t *completions = malloc(sizeof(completions_t));
    if (!completions) {
        return NULL;
    }

    if (prefix != NULL && prefix_length != 0) {
        if (prefix_length == SIZE_MAX) {
            completions->prefix = strdup(prefix);
        }
        else {
            completions->prefix = (char *)malloc(prefix_length + 1);
            if (completions->prefix != NULL) {
                memcpy(completions->prefix, prefix, prefix_length);
                completions->prefix[prefix_length] = '\0';
            }
        }
        if (completions->prefix == NULL) {
            free(completions);
            return NULL;
        }
    }
    else {
        completions->prefix = NULL;
    }
    completions->matches = NULL;
    completions->n_matches = 0;
    completions->matches_capacity = 0;
    return completions;
}

void completions_free(completions_t *completions) {
    if (!completions) {
        return;
    }
    free(completions->prefix);
    for (size_t i = 0; i < completions->n_matches; i++) {
        free(completions->matches[i]);
    }
    free(completions->matches);
    free(completions);
}

bool completions_add(completions_t *completions, const char *match) {
    if (completions->n_matches == completions->matches_capacity) {
        size_t new_capacity = completions->matches_capacity == 0 ? 4 : completions->matches_capacity * 2;
        char **new_matches = realloc(completions->matches, new_capacity * sizeof(char *));
        if (!new_matches) {
            return false;
        }
        completions->matches = new_matches;
        completions->matches_capacity = new_capacity;
    }
    completions->matches[completions->n_matches] = strdup(match);
    if (!completions->matches[completions->n_matches]) {
        return false;
    }
    completions->n_matches++;
    return true;
}

char **completions_get_matches(const completions_t *completions) {
    char **matches = (char **)malloc(completions->n_matches * sizeof(char *));
    if (!matches) {
        return NULL;
    }
    for (size_t i = 0; i < completions->n_matches; i++) {
        if (completions->prefix) {
            asprintf(matches + i, "%s%s", completions->prefix, completions->matches[i]);
        }
        else {
            matches[i] = strdup(completions->matches[i]);
        }
        if (!matches[i]) {
            for (size_t j = 0; j < i; j++) {
                free(matches[j]);
            }
            free(matches);
            return NULL;
        }
    }
    return matches;
}

static int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void completions_sort(completions_t *completions) {
    if (completions->n_matches < 2) {
        return;
    }
    qsort(completions->matches, completions->n_matches, sizeof(char *), compare_strings);
}


completions_t *complete(const char *line, size_t position, size_t *last_word_length, char *open_quote) {
    completions_t *completions = NULL;
    error_message_t *error_message = NULL;

    if (completion_words == NULL) {
        completion_words = words_new();
    }
    words_clear(completion_words);
    if (!words_parse_partial(completion_words, line, line + position, &error_message)) {
        if (error_message) {
            error_message_free(error_message);
        }
        return NULL;
    }

    if (last_word_length) {
        if (completion_words->n_words == 0) {
            *last_word_length = 0;
        }
        else {
            *last_word_length = strlen(completion_words->words[completion_words->n_words - 1]);
        }
    }
    if (open_quote) {
        *open_quote = completion_words->open_quote;
    }

    if (completion_words->n_words < 2) {
        const char *word = completion_words->n_words > 0 ? completion_words->words[0] : "";
        completions = completions_new(NULL, 0);
        complete_command(completions, argument_type_string, word);
    }
    else {
        const command_t *cmd;
        invocation_t *invocation = NULL;

        if ((cmd = find_command(completion_words->words[0])) == NULL) {
            return NULL;
        }

        invocation = invocation_new(cmd, NULL, completion_words->n_words, completion_words->words, &error_message, &completions);

        if (error_message) {
            error_message_free(error_message);
        }
        invocation_free(invocation);
    }
    return completions;
}

void free_completions(char **matches) {
    size_t i;

    if (matches == NULL) {
        return;
    }
    for (i = 0; matches[i] != NULL; i++) {
        free(matches[i]);
    }
    free(matches);
}
