#include "zip.h"
#include "error.h"


int
main(int argc, char *argv[])
{
#if 0
    int i;
#endif
    struct zip *zf, *destzf;
#if 0
#define BUFSIZE 65536
    struct zip_file *zff1, *zff2;
    char buf1[BUFSIZE], buf2[BUFSIZE];
#endif
    
#if 0
    if (argc != 2) {
	fprintf(stderr, "%s: call with one option: the zip-file to destroy"
		"^H^H^H^H^H^H^Htest", argv[0]);
	return 1;
    }
#endif
    if (argc != 3) {
	fprintf(stderr, "%s: call with two options: src dest", argv[0]);
	return 1;
    }

    if ((zf=zip_open(argv[1], ZIP_CHECKCONS))==NULL) {
	fprintf(stderr, "%s: %s: can't open file: %s", argv[0], argv[1],
		zip_err_str[zip_err]);
	return 1;
    }

    if ((destzf=zip_open(argv[2], ZIP_CREATE))==NULL) {
	fprintf(stderr, "%s: %s: can't open file: %s", argv[0], argv[2],
		zip_err_str[zip_err]);
	return 1;
    }

#if 0
    for (i=0; i<zf->nentry; i++) {
	printf("%8d %s\n", zf->entry[i].uncomp_size, zf->entry[i].fn);
	zip_add_zip(destzf, zf->entry[i].fn, zf, i, 0, 0);
    }
#endif

    if (zip_add_zip(destzf, NULL, NULL, zf, 0, 0, 0) == -1)
	fprintf(stderr, "%s: %s: can't add file to zip '%s': %s", argv[0],
		zf->entry[0].fn, argv[1], zip_err_str[zip_err]);

#if 0
    zff1= zff_open_index(zf, 1);
    if (!zff1) {
	fprintf(stderr, "boese, boese\n");
	exit(100);
    }
    
    i = zff_read(zff1, buf1, 100);
    if (i < 0)
	fprintf(stderr, "read error: %s\n", zip_err_str[zff1->flags]);
    else {
	buf1[i] = 0;
	printf("read %d bytes: '%s'\n", i, buf1);
    }
    zff2 = zff_open_index(zf, 1);
    i = zff_read(zff2, buf2, 200);
    if (i < 0)
	fprintf(stderr, "read error: %s\n", zip_err_str[zff2->flags]);
    else {
	buf2[i] = 0;
	printf("read %d bytes: '%s'\n", i, buf2);
    }
    i = zff_read(zff1, buf1, 100);
    if (i < 0)
	fprintf(stderr, "read error: %s\n", zip_err_str[zff1->flags]);
    else {
	buf1[i] = 0;
	printf("read %d bytes: '%s'\n", i, buf1);
    }
    zff_close(zff1);
    zff_close(zff2);
#endif
    
    if (zip_close(destzf)!=0) {
	fprintf(stderr, "%s: %s: can't close file: %s", argv[0], argv[2],
		zip_err_str[zip_err]);
	return 1;
    }

    if (zip_close(zf)!=0) {
	fprintf(stderr, "%s: %s: can't close file: %s", argv[0], argv[1],
		zip_err_str[zip_err]);
	return 1;
    }
    
    return 0;
}
