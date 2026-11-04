#include "zipsh.h"

zip_flags_t get_flags(invocation_t *invocation) {
    argument_t *argument = NULL;
    zip_flags_t flags = default_flags;

    if ((argument = invocation_get_option(invocation, OPTION_CASE_INSENSITIVE))) {
        flags |= ZIP_FL_NOCASE;
    }
    if ((argument = invocation_get_option(invocation, OPTION_CASE_SENSITIVE))) {
        flags &= ~ZIP_FL_NOCASE;
    }
    if ((argument = invocation_get_option(invocation, OPTION_CHANGED))) {
        flags &= ~ZIP_FL_UNCHANGED;
    }
    if ((argument = invocation_get_option(invocation, OPTION_IGNORE_DIRECTORY))) {
        flags |= ZIP_FL_NODIR;
    }
    if ((argument = invocation_get_option(invocation, OPTION_NO_IGNORE_DIRECTORY))) {
        flags &= ~ZIP_FL_NODIR;
    }
    if ((argument = invocation_get_option(invocation, OPTION_ORIGINAL))) {
        flags |= ZIP_FL_UNCHANGED;
    }
    if ((argument = invocation_get_option(invocation, OPTION_ENCODING))) {
        flags &= ~(ZIP_FL_ENC_RAW | ZIP_FL_ENC_STRICT | ZIP_FL_ENC_CP437);
        flags |= argument->u32;
    }

    return flags;
}

bool default_flags_implementation(invocation_t *invocation, error_message_t **error_message) {
    default_flags = get_flags(invocation);

    if (default_flags & ZIP_FL_NOCASE) {
        printf("case-insensitive");
    }
    else {
        printf("case-sensitive");
    }
    if (default_flags & ZIP_FL_NODIR) {
        printf(" ignore-directory");
    }
    else {
        printf(" no-ignore-directory");
    }
    if (default_flags & ZIP_FL_UNCHANGED) {
        printf(" original");
    }
    else {
        printf(" changed");
    }
    if (default_flags & ZIP_FL_ENC_RAW) {
        printf(" encoding=raw");
    }
    else if (default_flags & ZIP_FL_ENC_STRICT) {
        printf(" encoding=strict");
    }
    else if (default_flags & ZIP_FL_ENC_CP437) {
        printf(" encoding=cp437");
    }
    else {
        printf(" encoding=guess");
    }
    printf("\n");

    return true;
}

option_definition_t default_flags_options[] = {FLAGS_OPTIONS};

COMMAND_NO_ARGUMENTS(default_flags, "print or change default flags");
