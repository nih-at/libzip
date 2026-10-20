#include <stdlib.h>

#include "shell.h"

words_t *words_new(void) {
    words_t *words = (words_t *)malloc(sizeof(words_t));
    if (!words) {
        return NULL;
    }
    words->words = NULL;
    words->words_capacity = 0;

    words->text = NULL;
    words->text_capacity = 0;

    words_clear(words);

    return words;
}

void words_clear(words_t *words) {
    if (!words) {
        return;
    }
    words->next_word_start = 0;
    words->next_char = 0;
    words->n_words = 0;
}

void words_free(words_t *words) {
    if (!words) {
        return;
    }
    free(words->text);
    free(words->words);
    free(words);
}


bool words_add_char(words_t *words, char c, error_message_t **error_message) {
    if (words->next_char + 1 >= words->text_capacity) {
        size_t new_capacity = words->text_capacity == 0 ? 64 : words->text_capacity * 2;
        char *new_text = (char *)realloc(words->text, new_capacity);
        if (!new_text) {
            *error_message = error_message_new("out of memory");
            return false;
        }
        words->text = new_text;
        words->text_capacity = new_capacity;
    }
    words->text[words->next_char++] = c;
    return true;
}

bool words_add_word(words_t *words, bool add_empty, error_message_t **error_message) {
    if (!add_empty && words->next_word_start == words->next_char) {
        return true;
    }

    words_add_char(words, '\0', error_message);
    if (words->n_words + 1 >= words->words_capacity) {
        size_t new_capacity = words->words_capacity == 0 ? 8 : words->words_capacity * 2;
        char **new_words = (char **)realloc(words->words, new_capacity * sizeof(char *));
        if (!new_words) {
            *error_message = error_message_new("out of memory");
            return false;
        }
        words->words = new_words;
        words->words_capacity = new_capacity;
    }
    words->words[words->n_words++] = words->text + words->next_word_start;
    words->next_word_start = words->next_char;
    return true;
}

bool words_parse(words_t *words, const char *line, error_message_t **error_message) {
    return words_parse_partial(words, line, NULL, error_message);
}

bool words_parse_partial(words_t *words, const char *line, const char *end, error_message_t **error_message) {
    const char *p = line;
    char quote = '\0';
    bool add_empty = false;

    words_clear(words);

    while (*p && (end == NULL || p < end)) {
        switch (*p) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            if (quote == '\0') {
                if (!words_add_char(words, '\0', error_message)) {
                    return false;
                }
                if (!words_add_word(words, add_empty, error_message)) {
                    return false;
                }
                add_empty = false;
            }
            else {
                if (!words_add_char(words, *p, error_message)) {
                    return false;
                }
            }
            break;

        case '\'':
        case '"':
            if (quote == '\0') {
                quote = *p;
                add_empty = true;
            }
            else if (quote == *p) {
                quote = '\0';
            }
            else {
                if (!words_add_char(words, *p, error_message)) {
                    return false;
                }
            }
            break;

        case '\\':
            p++;
            if (*p) {
                if (!words_add_char(words, *p, error_message)) {
                    return false;
                }
            }
            else {
                if (error_message) {
                    *error_message = error_message_new("unexpected end of line after escape character");
                }
                return false;
            }
            break;

        default:
            if (!words_add_char(words, *p, error_message)) {
                return false;
            }
        }
        p++;
    }

    /* For partial parsing, unclosed quotes are okay. */
    if (quote != '\0' && end == NULL) {
        if (error_message) {
            *error_message = error_message_new_format("unmatched quote %c", quote);
        }
        return false;
    }

    return words_add_word(words, add_empty, error_message);
}
