#include "zip.h"



char * zip_err_str[]={
    "no error",
    "multi-disk zip-files not supported",
    "replacing zipfile with tempfile failed",
    "closing zipfile failed",
    "seek error",
    "read error",
    "write error",
    "CRC error",
    "zip file closed without closing this file",
    "file does already exist",
    "file doesn't exist",
    "can't open file"    
};
