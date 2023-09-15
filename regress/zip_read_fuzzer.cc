#include <zip.h>
#include <zipint.h>
#include <iostream>
#include <fstream>
#include <string>  

std::string random_string(size_t length);

#ifdef __cplusplus
extern "C"
#endif

/**
This fuzzing target takes input data, creates a ZIP archive from it, checks the archive's consistency, 
and iterates over the entries in the archive, reading data from each entry.
**/

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    zip_source_t* src;
    zip_t* za;
    zip_error_t error;
    char buf[32768];
    zip_int64_t i, n;
    zip_file_t* f;

    std::string name = random_string(20) + ".zip";

    std::ofstream outfile(name, std::ios::binary);

    if (outfile.is_open()) {
        outfile.write(reinterpret_cast<const char*>(data), size);
        outfile.close();

        struct zip* archive = zip_open(name.c_str(), ZIP_CHECKCONS, NULL);
        if (archive) {
            int result = zip_close(archive);
        } 
    } else {
        std::cerr << "Unable to open the file." << std::endl;
    }
    std::remove(name.c_str()); 

    zip_error_init(&error);

    if ((src = zip_source_buffer_create(data, size, 0, &error)) == NULL) {
        zip_error_fini(&error);
        return 0;
    }

    
    if ((za = zip_open_from_source(src, 0, &error)) == NULL) {
        zip_source_free(src);
        zip_error_fini(&error);
        return 0;
    }

    
    zip_error_fini(&error);

    n = zip_get_num_entries(za, 0);

    for (i = 0; i < n; i++) {
        f = zip_fopen_index(za, i, 0);
        if (f == NULL) {
            continue;
        }

        while (zip_fread(f, buf, sizeof(buf)) > 0) {
            ;
        }

        zip_fclose(f);
    }

    zip_close(za);

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
