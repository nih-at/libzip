#include "shell_util.h"

void print_stat(const zip_stat_t *stat) {
    if (stat->valid & ZIP_STAT_NAME) {
        printf("Name: %s\n", stat->name);
    }
    if (stat->valid & ZIP_STAT_INDEX) {
        printf("Index: %" PRIu64 "\n", stat->index);
    }
    if (stat->valid & ZIP_STAT_SIZE) {
        printf("Size: %" PRIu64 "\n", stat->size);
    }
    if (stat->valid & ZIP_STAT_COMP_SIZE) {
        printf("Compressed Size: %" PRIu64 "\n", stat->comp_size);
    }
    if (stat->valid & ZIP_STAT_MTIME) {
        printf("Modification Time: %s\n", format_mtime(stat->mtime));
    }
    if (stat->valid & ZIP_STAT_CRC) {
        printf("CRC: %08x\n", (unsigned int)stat->crc);
    }
    if (stat->valid & ZIP_STAT_COMP_METHOD) {
        printf("Compression Method: %s\n", enum_argument_to_string(argument_type_compression_method, (argument_t){.u64 = stat->comp_method}, true));
    }
    if (stat->valid & ZIP_STAT_ENCRYPTION_METHOD) {
        printf("Encryption Method: %s\n", enum_argument_to_string(argument_type_encryption_method, (argument_t){.u64 = stat->encryption_method}, true));
    }
}