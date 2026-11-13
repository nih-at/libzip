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

void complete_filename(completions_t *completions, const argument_type_t *type, const char *str) {
    /* TODO: implement */
    (void)type;
    (void)str;
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

const argument_type_t argument_type_filename_instance = {
    "filename", NULL, complete_filename, NULL, {.pointer = NULL}
};
/* clang-format on */


bool convert_argument(const argument_type_t *type, argument_t *out, const char *str, error_message_t **error_message) {
    if (type->convert == NULL) {
        out->string = (char *)str;
        return true;
    }
    return type->convert(type, out, str, error_message);
}

completions_t *complete_argument(const argument_type_t *type, const char *str, size_t prefix_length) {
    completions_t *completions;
    if (type->complete == NULL) {
        return NULL;
    }
    completions = completions_new(str, prefix_length);
    type->complete(completions, type, str + prefix_length);
    return completions;
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

void complete_enum(completions_t *completions, const argument_type_t *type, const char *str) {
    argument_type_enum_entry_t *entries = (argument_type_enum_entry_t *)type->ud.pointer;
    for (size_t i = 0; entries[i].string != NULL; i++) {
        if (strncmp(entries[i].string, str, strlen(str)) == 0) {
            completions_add(completions, entries[i].string);
        }
    }
}

const char *enum_argument_to_string(const argument_type_t *type, argument_t value, bool string_for_unknown) {
    static char unknown_string[64];
    argument_type_enum_entry_t *entries = (argument_type_enum_entry_t *)type->ud.pointer;
    for (size_t i = 0; entries[i].string != NULL; i++) {
        if (memcmp(&entries[i].value, &value, sizeof(argument_t)) == 0) {
            return entries[i].string;
        }
    }
    if (string_for_unknown) {
        snprintf(unknown_string, sizeof(unknown_string), "unknown (%llu)", (unsigned long long)value.u64);
        return unknown_string;
    }
    return NULL;
}
