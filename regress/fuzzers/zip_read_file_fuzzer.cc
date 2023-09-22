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
   This fuzzing target takes input data, creates a ZIP archive from it, checks the archive's consistency,
   and iterates over the entries in the archive, reading data from each entry.
**/

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    zip_source_t *src;
    zip_t *za;
    zip_error_t error;
    char buf[32768];
    zip_int64_t i, n;
    zip_file_t *f;

    std::string name = random_string(20) + ".zip";

    std::ofstream outfile(name, std::ios::binary);

    if (outfile.is_open()) {
        outfile.write(reinterpret_cast<const char *>(data), size);
        outfile.close();

        za = zip_open(name.c_str(), 0, NULL);
        if (za == NULL) {
            std::remove(name.c_str());
            std::cerr << "Error opening archive." << std::endl;
            return 1;
        }

        n = zip_get_num_entries(za, 0);
        for (i = 0; i < n; i++) {
            f = zip_fopen_index(za, i, 0);
            if (f == NULL) {
                std::remove(name.c_str());
                std::cerr << "Unable to open file." << std::endl;
                zip_close(za);
                return 1;
            }

            while (zip_fread(f, buf, sizeof(buf)) > 0) {
                ;
            }
            if (zip_fclose(f) < 0) {
                std::remove(name.c_str());
                std::cerr << "Error closing file." << std::endl;
                zip_close(za);
                return 1;
            }
        }
        if (zip_close(za) < 0) {
            std::remove(name.c_str());
            std::cerr << "Error closing archive." << std::endl;
            return 1;
        }
    }
    else {
        std::remove(name.c_str());
        std::cerr << "Unable to open the file." << std::endl;
        return 1;
    }
    std::remove(name.c_str());
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
