#include <zip.h>
#include <zipint.h>
#include <iostream>
#include <fstream>
#include <string>

std::string random_string(size_t length);

#ifdef __cplusplus
extern "C"
#endif

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    zip_source_t* src;
    zip_t* za;
    char buf[32768];
    zip_int64_t i, n;
    zip_file_t* f;

    std::string path = random_string(20) + "_pkware"+ ".zip";
    const char *password = "secretpassword";
    const char * file    = "file";   
    int error = 0;
    struct zip *archive = zip_open(path.c_str(), ZIP_CREATE, &error);

    if(error)
        return -1;

    struct zip_source *source = zip_source_buffer(archive, data, size, 0);
    if(source == NULL){
        printf("failed to create source buffer. %s\n",zip_strerror(archive));
        return -1;
    }

    int index = (int)zip_file_add(archive, file, source, ZIP_FL_OVERWRITE);
    if(index < 0){
        printf("failed to add file to archive: %s\n",zip_strerror(archive));
        return -1;
    }
    zip_file_set_encryption(archive, index, ZIP_EM_TRAD_PKWARE,/* Password to encrypt file */ password);
    zip_close(archive);
    std::remove(path.c_str()); 

    return 0;
}

std::string random_string(size_t length)
{
    auto randchar = []() -> char {
        const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}
