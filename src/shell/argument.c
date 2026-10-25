#include <stdlib.h>
#include <string.h>

#include "shell.h"

/* clang-format off */
argument_type_enum_entry_t bool_entries[] = {
    {"true", {.boolean = true}}, 
    {"false", {.boolean = false}},
     {"on", {.boolean = true}}, 
     {"off", {.boolean = false}}, 
     {"yes", {.boolean = true}}, 
     {"no", {.boolean = false}}, 
     {NULL, {0}}};
/* clang-format on */

bool convert_int(const argument_type_t *type, argument_t *out, const char *str, error_message_t **error_message) {
    char *endptr;
    zip_int64_t value = strtoll(str, &endptr, 0);

    if (*endptr != '\0') {
        if (error_message) {
            *error_message = error_message_new_format("invalid integer value '%s'", str);
        }
        return false;
    }

    switch (type->ud.u64) {
    case sizeof(int8_t):
        if (value < INT8_MIN || value > INT8_MAX) {
            if (error_message) {
                *error_message = error_message_new_format("value out of range for int8: '%s'", str);
            }
            return false;
        }
        out->i8 = (int8_t)value;
        break;

    case sizeof(int16_t):
        if (value < INT16_MIN || value > INT16_MAX) {
            if (error_message) {
                *error_message = error_message_new_format("value out of range for int16: '%s'", str);
            }
            return false;
        }
        out->i16 = (int16_t)value;
        break;

    case sizeof(int32_t):
        if (value < INT32_MIN || value > INT32_MAX) {
            if (error_message) {
                *error_message = error_message_new_format("value out of range for int32: '%s'", str);
            }
            return false;
        }
        out->i32 = (int32_t)value;
        break;

    case sizeof(int64_t):
        out->i64 = value;
        break;

    default:
        if (error_message) {
            *error_message = error_message_new("unsupported integer size");
        }
        return false;
    }

    return true;
}

bool convert_uint(const argument_type_t *type, argument_t *out, const char *str, error_message_t **error_message) {
    char *endptr;
    zip_uint64_t value = strtoull(str, &endptr, 0);

    if (*endptr != '\0') {
        if (error_message) {
            *error_message = error_message_new_format("invalid unsigned integer value '%s'", str);
        }
        return false;
    }

    switch (type->ud.u64) {
    case sizeof(uint8_t):
        if (value > UINT8_MAX) {
            if (error_message) {
                *error_message = error_message_new_format("value out of range for uint8: '%s'", str);
            }
            return false;
        }
        out->u8 = (uint8_t)value;
        break;

    case sizeof(uint16_t):
        if (value > UINT16_MAX) {
            if (error_message) {
                *error_message = error_message_new_format("value out of range for uint16: '%s'", str);
            }
            return false;
        }
        out->u16 = (uint16_t)value;
        break;

    case sizeof(uint32_t):
        if (value > UINT32_MAX) {
            if (error_message) {
                *error_message = error_message_new_format("value out of range for uint32: '%s'", str);
            }
            return false;
        }
        out->u32 = (uint32_t)value;
        break;

    case sizeof(uint64_t):
        out->u64 = value;
        break;

    default:
        if (error_message) {
            *error_message = error_message_new("unsupported unsigned integer size");
        }
        return false;
    }

    return true;
}


/* clang-format off */
const argument_type_t argument_type_bool_instance = ARGUMENT_TYPE_ENUM("bool", bool_entries);

const argument_type_t argument_type_string_instance = {
    "string", NULL, NULL, NULL, {.pointer = NULL}
};

const argument_type_t argument_type_int8_instance = {
    "int8",convert_int, NULL, NULL, {.u64 = sizeof(int8_t)}
};

const argument_type_t argument_type_int16_instance = {
    "int16", convert_int, NULL, NULL, {.u64 = sizeof(int16_t)}
};

const argument_type_t argument_type_int32_instance = {
    "int32", convert_int, NULL, NULL, {.u64 = sizeof(int32_t)}
};

const argument_type_t argument_type_int64_instance = {
    "int64", convert_int, NULL, NULL, {.u64 = sizeof(int64_t)}
};

const argument_type_t argument_type_uint8_instance = {
    "uint8", convert_uint, NULL, NULL, {.u64 = sizeof(uint8_t)}
};

const argument_type_t argument_type_uint16_instance = {
    "uint16", convert_uint, NULL, NULL, {.u64 = sizeof(uint16_t)}
};

const argument_type_t argument_type_uint32_instance = {
    "uint32", convert_uint, NULL, NULL, {.u64 = sizeof(uint32_t)}
};

const argument_type_t argument_type_uint64_instance = {
    "uint64", convert_uint, NULL, NULL, {.u64 = sizeof(uint64_t)}
};
/* clang-format on */


bool convert_argument(const argument_type_t *type, argument_t *out, const char *str, error_message_t **error_message) {
    if (type->convert == NULL) {
        out->string = (char *)str;
        return true;
    }
    return type->convert(type, out, str, error_message);
}

char **complete_argument(const argument_type_t *type, const char *str) {
    if (type->complete == NULL) {
        return NULL;
    }
    return type->complete(type, str);
}

invocation_t *invocation_new(size_t options_capacity, size_t arguments_capacity) {
    invocation_t *invocation = malloc(sizeof(invocation_t));
    if (invocation == NULL) {
        return NULL;
    }

    invocation->n_options = 0;
    invocation->options = NULL;
    invocation->arguments = NULL;
    invocation->options_capacity = options_capacity;
    invocation->arguments_capacity = arguments_capacity;

    if (options_capacity > 0) {
        invocation->options = malloc(options_capacity * sizeof(option_t));
        if (invocation->options == NULL) {
            invocation_free(invocation);
            return NULL;
        }
    }
    if (arguments_capacity > 0) {
        invocation->arguments = malloc(arguments_capacity * sizeof(argument_t));
        if (invocation->arguments == NULL) {
            invocation_free(invocation);
            return NULL;
        }
    }

    return invocation;
}

void invocation_free(invocation_t *invocation) {
    if (invocation == NULL) {
        return;
    }
    free(invocation->options);
    free(invocation->arguments);
    free(invocation);
}

bool invocation_add_option(invocation_t *invocation, int id, argument_t value) {
    if (invocation->n_options >= invocation->options_capacity) {
        size_t new_capacity = invocation->options_capacity == 0 ? 1 : invocation->options_capacity * 2;
        option_t *new_options = realloc(invocation->options, new_capacity * sizeof(option_t));
        if (new_options == NULL) {
            return false;
        }
        invocation->options = new_options;
        invocation->options_capacity = new_capacity;
    }
    invocation->options[invocation->n_options].id = id;
    invocation->options[invocation->n_options].value = value;
    invocation->n_options++;
    return true;
}

bool invocation_add_argument(invocation_t *invocation, argument_t value) {
    if (invocation->n_arguments >= invocation->arguments_capacity) {
        size_t new_capacity = invocation->arguments_capacity == 0 ? 1 : invocation->arguments_capacity * 2;
        argument_t *new_arguments = realloc(invocation->arguments, new_capacity * sizeof(argument_t));
        if (new_arguments == NULL) {
            return false;
        }
        invocation->arguments = new_arguments;
        invocation->arguments_capacity = new_capacity;
    }
    invocation->arguments[invocation->n_arguments] = value;
    invocation->n_arguments++;
    return true;
}

argument_t *invocation_get_option(invocation_t *invocation, int id) {
    for (size_t i = 0; i < invocation->n_options; i++) {
        if (invocation->options[i].id == id) {
            return &invocation->options[i].value;
        }
    }
    return NULL;
}


bool convert_enum(const argument_type_t *type, argument_t *out, const char *str, error_message_t **error_message) {
    argument_type_enum_entry_t *entries = (argument_type_enum_entry_t *)type->ud.pointer;
    for (size_t i = 0; entries[i].string != NULL; i++) {
        if (strcmp(entries[i].string, str) == 0) {
            *out = entries[i].value;
            return true;
        }
    }
    if (error_message) {
        *error_message = error_message_new_format("invalid %s '%s'", type->name, str);
    }
    return false;
}

char **complete_enum(const argument_type_t *type, const char *str) {
    argument_type_enum_entry_t *entries = (argument_type_enum_entry_t *)type->ud.pointer;
    size_t count = 0;
    for (size_t i = 0; entries[i].string != NULL; i++) {
        if (strncmp(entries[i].string, str, strlen(str)) == 0) {
            count++;
        }
    }
    char **matches = malloc((count + 1) * sizeof(char *));
    if (matches == NULL) {
        return NULL;
    }
    size_t j = 0;
    for (size_t i = 0; entries[i].string != NULL; i++) {
        if (strncmp(entries[i].string, str, strlen(str)) == 0) {
            matches[j++] = strdup(entries[i].string);
        }
    }
    matches[j] = NULL;
    return matches;
}
