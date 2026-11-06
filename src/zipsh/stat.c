#include <errno.h>
#include <string.h>

#include "zipsh.h"


/* clang-format off */
argument_definition_t stat_arguments[] = {
    FILE_REFERENCE_ARGUMENTS,
};

option_definition_t stat_options[] = {
    FILE_REFERENCE_OPTIONS
};
/* clang-format on */

bool stat_implementation(invocation_t *invocation, error_message_t **error_message) {
    zip_stat_t stat;
    file_reference_t *file_ref = file_reference_create(invocation, error_message);
    if (!file_ref) {
        return false;
    }
    if (zip_stat_index(archive, file_ref->index, 0, &stat) < 0) {
        *error_message = error_message_new_format("can't get file info for %s: %s", file_ref->error_info, zip_strerror(archive));
        file_reference_free(file_ref);
        return false;
    }

    printf("Name: %s\n", stat.name);
    printf("Index: %" PRIu64 "\n", file_ref->index);
    printf("Size: %" PRIu64 "\n", stat.size);
    printf("Compressed Size: %" PRIu64 "\n", stat.comp_size);
    printf("Modification Time: %s\n", format_mtime(stat.mtime));
    printf("CRC: %08x\n", (unsigned int)stat.crc);
    printf("Compression Method: %s\n", enum_argument_to_string(argument_type_compression_method, (argument_t){.u64 = stat.comp_method}, true));
    printf("Encryption Method: %s\n", enum_argument_to_string(argument_type_encryption_method, (argument_t){.u16 = stat.encryption_method}, true));

    file_reference_free(file_ref);

    return true;
}

COMMAND(stat, "show information of file in archive");
