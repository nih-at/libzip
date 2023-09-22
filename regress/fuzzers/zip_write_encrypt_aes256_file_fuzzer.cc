#include <fstream>
#include <iostream>
#include <string>
#include <zip.h>

#ifdef __cplusplus
extern "C" {
#if 0
    /* fix autoindent */
    }
#endif
#endif

std::string random_string(size_t length);

/**
   This fuzzing target takes input data, creates a ZIP archive, load
   it to a buffer, adds a file to it with AES-256 encryption and a
   specified password, and then closes and removes the archive.

   The purpose of this fuzzer is to test security of ZIP archive
   handling and encryption in the libzip by subjecting it to various
   inputs, including potentially malicious or malformed data of
   different file types.
 **/

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    zip_source_t *src;
    zip_t *za;
    char buf[32768];
    zip_int64_t i, n;
    zip_file_t *f;

    std::string path = random_string(20) + "_aes256" + ".zip";
    std::string password = random_string(20);
    std::string file = random_string(20);
    int error = 0;
    struct zip *archive = zip_open(path.c_str(), ZIP_CREATE, &error);

    if (error) {
        std::remove(path.c_str());
        return -1;
    }

    struct zip_source *source = zip_source_buffer(archive, data, size, 0);
    if (source == NULL) {
        std::remove(path.c_str());
        printf("failed to create source buffer. %s\n", zip_strerror(archive));
        return -1;
    }

    int index = (int)zip_file_add(archive, file.c_str(), source, ZIP_FL_OVERWRITE);
    if (index < 0) {
        std::remove(path.c_str());
        printf("failed to add file to archive: %s\n", zip_strerror(archive));
        return -1;
    }
    if (zip_file_set_encryption(archive, index, ZIP_EM_AES_256, password.c_str()) < 0) {
        std::remove(path.c_str());
        printf("failed to set file encryption: %s\n", zip_strerror(archive));
        return -1;
    }
    if (zip_close(archive) < 0) {
        std::remove(path.c_str());
        printf("error closing archive: %s\n", zip_strerror(archive));
        return -1;
    }
    std::remove(path.c_str());

    return 0;
}

std::string
random_string(size_t length) {
    auto randchar = []() -> char {
        const char charset[] = "0123456789"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}
#ifdef __cplusplus
}
#endif
