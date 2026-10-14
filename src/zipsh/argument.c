#include <stdlib.h>
#include <string.h>

#include "zipsh.h"

#define convert_int8(out, str, error_message) convert_int(out, str, sizeof(int8_t), error_message)
#define convert_int16(out, str, error_message) convert_int(out, str, sizeof(int16_t), error_message)
#define convert_int32(out, str, error_message) convert_int(out, str, sizeof(int32_t), error_message)
#define convert_int64(out, str, error_message) convert_int(out, str, sizeof(int64_t), error_message)
#define convert_uint8(out, str, error_message) convert_uint(out, str, sizeof(uint8_t), error_message)
#define convert_uint16(out, str, error_message) convert_uint(out, str, sizeof(uint16_t), error_message)
#define convert_uint32(out, str, error_message) convert_uint(out, str, sizeof(uint32_t), error_message)
#define convert_uint64(out, str, error_message) convert_uint(out, str, sizeof(uint64_t), error_message)

bool convert_int(void *out, const char *str, int size, error_message_t **error_message) {
    char *endptr;
    zip_int64_t value = strtoll(str, &endptr, 0);

    if (*endptr != '\0') {
        if (error_message) {
            *error_message = error_message_new_format("invalid integer value '%s'", str);
        }
        return false;
    }

    switch (size) {
    case sizeof(int8_t):
        if (value < INT8_MIN || value > INT8_MAX) {
            if (error_message) {
                *error_message = error_message_new_format("value out of range for int8: '%s'", str);
            }
            return false;
        }
        *(int8_t *)out = (int8_t)value;
        break;

    case sizeof(int16_t):
        if (value < INT16_MIN || value > INT16_MAX) {
            if (error_message) {
                *error_message = error_message_new_format("value out of range for int16: '%s'", str);
            }
            return false;
        }
        *(int16_t *)out = (int16_t)value;
        break;

    case sizeof(int32_t):
        if (value < INT32_MIN || value > INT32_MAX) {
            if (error_message) {
                *error_message = error_message_new_format("value out of range for int32: '%s'", str);
            }
            return false;
        }
        *(int32_t *)out = (int32_t)value;
        break;

    case sizeof(int64_t):
        *(int64_t *)out = value;
        break;

    default:
        if (error_message) {
            *error_message = error_message_new("unsupported integer size");
        }
        return false;
    }

    return true;
}

bool convert_uint(void *out, const char *str, int size, error_message_t **error_message) {
    char *endptr;
    zip_uint64_t value = strtoull(str, &endptr, 0);

    if (*endptr != '\0') {
        if (error_message) {
            *error_message = error_message_new_format("invalid unsigned integer value '%s'", str);
        }
        return false;
    }

    switch (size) {
    case sizeof(uint8_t):
        if (value > UINT8_MAX) {
            if (error_message) {
                *error_message = error_message_new_format("value out of range for uint8: '%s'", str);
            }
            return false;
        }
        *(uint8_t *)out = (uint8_t)value;
        break;

    case sizeof(uint16_t):
        if (value > UINT16_MAX) {
            if (error_message) {
                *error_message = error_message_new_format("value out of range for uint16: '%s'", str);
            }
            return false;
        }
        *(uint16_t *)out = (uint16_t)value;
        break;

    case sizeof(uint32_t):
        if (value > UINT32_MAX) {
            if (error_message) {
                *error_message = error_message_new_format("value out of range for uint32: '%s'", str);
            }
            return false;
        }
        *(uint32_t *)out = (uint32_t)value;
        break;

    case sizeof(uint64_t):
        *(uint64_t *)out = value;
        break;

    default:
        if (error_message) {
            *error_message = error_message_new("unsupported unsigned integer size");
        }
        return false;
    }

    return true;
}

bool convert_argument(const char *str, argument_t *arg, argument_type_t type, error_message_t **error_message) {
    switch (type) {
    case ARGUMENT_TYPE_BOOL:
        if (strcasecmp(str, "true") == 0 || strcmp(str, "1") == 0 || strcasecmp(str, "yes") == 0 || strcasecmp(str, "on") == 0) {
            arg->b = true;
        }
        else if (strcasecmp(str, "false") == 0 || strcmp(str, "0") == 0 || strcasecmp(str, "no") == 0 || strcasecmp(str, "off") == 0) {
            arg->b = false;
        }
        else {
            if (error_message) {
                *error_message = error_message_new_format("invalid boolean value '%s'", str);
            }
            return false;
        }
        break;

    case ARGUMENT_TYPE_STRING:
        arg->string = str;
        break;

    case ARGUMENT_TYPE_INT8:
        return convert_int8(&arg->i8, str, error_message);
    case ARGUMENT_TYPE_INT16:
        return convert_int16(&arg->i16, str, error_message);
    case ARGUMENT_TYPE_INT32:
        return convert_int32(&arg->i32, str, error_message);
    case ARGUMENT_TYPE_INT64:
        return convert_int64(&arg->i64, str, error_message);
    case ARGUMENT_TYPE_UINT8:
        return convert_uint8(&arg->u8, str, error_message);
    case ARGUMENT_TYPE_UINT16:
        return convert_uint16(&arg->u16, str, error_message);
    case ARGUMENT_TYPE_UINT32:
        return convert_uint32(&arg->u32, str, error_message);
    case ARGUMENT_TYPE_UINT64:
        return convert_uint64(&arg->u64, str, error_message);
    default:
        *error_message = error_message_new("internal error: unknown argument type");
        return false;
    }
    return true;
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
