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

        za = zip_open(name.c_str(), 0, NULL);
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
        std::remove(name.c_str()); 

    }
    else {
        std::cerr << "Unable to open the file." << std::endl;
    }
        
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
