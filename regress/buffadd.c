#include <stdio.h>
#include <stdlib.h>

#include "zip.h"

char *teststr="This is a test, and it seems to have been successful.\n";
char *testname="testfile.txt";
char *testzip="test_zip.zip";

int
main(int argc, char *argv[])
{
    struct zip *z;
    struct zip_file *ze;
    
    char buf[2000];

    remove(testzip);
    
    if ((z=zip_open(testzip, ZIP_CREATE)) == NULL) {
	fprintf(stderr,"%s: can't open zipfile %s: %s\n", argv[0],
		testzip, zip_err_str[zip_err]);
	exit(1);
    }

    if (zip_add_data(z, testname, NULL, teststr, strlen(teststr), 0)==-1) {
	fprintf(stderr,"%s: can't add buffer '%s': %s\n", argv[0],
		teststr, zip_err_str[zip_err]);
	exit(1);
    }

    if (zip_close(z) == -1) {
	fprintf(stderr,"%s: can't close zipfile %s: %s\n", argv[0],
		testzip, zip_err_str[zip_err]);
	exit(1);
    }

    if ((z=zip_open(testzip, ZIP_CHECKCONS))==NULL) {
	fprintf(stderr,"%s: can't re-open zipfile %s: %s\n", argv[0],
		testzip, zip_err_str[zip_err]);
	exit(1);
    }

    if ((ze=zip_fopen(z, testname, 0))==NULL) {
	fprintf(stderr,"%s: can't fopen file '%s' in '%s': %s\n", argv[0],
		testname, testzip, zip_err_str[zip_err]);
	exit(1);
    }

    if (zip_fread(ze, buf, 2000) < 0) {
	fprintf(stderr,"%s: can't read from '%s' in zipfile '%s': %s\n",
		argv[0], testname, testzip, zip_err_str[zip_err]);
	exit(1);
    }
    
    if (strcmp(buf, teststr)) {
	fprintf(stderr,"%s: wrong data: '%s' instead of '%s'\n", argv[0],
		buf, teststr);
	exit(1);
    }

    if (zip_close(z) == -1) {
	fprintf(stderr,"%s: can't close zipfile %s: %s\n", argv[0],
		testzip, zip_err_str[zip_err]);
	exit(1);
    }

    remove(testzip);
    
    return 0;
}
