#ifndef _HAD_ZIP_H
#define _HAD_ZIP_H

#include <sys/types.h>
#include <stdio.h>
#include <zlib.h>

enum zip_state { Z_UNCHANGED, Z_DELETED, Z_REPLACED, Z_ADDED, Z_RENAMED };

/* flags for zip_open */
#define ZIP_CREATE           1
#define ZIP_EXCL             2
#define ZIP_CHECKCONS        4

int zip_err; /* global variable for errors returned by the low-level
		library */

#define ZERR_OK               0  /* no error */
#define ZERR_MULTIDISK        1  /* multi-disk zip-files not supported */
#define ZERR_RENAME           2  /* replacing zipfile with tempfile failed */
#define ZERR_CLOSE            3  /* closing zipfile failed */
#define ZERR_SEEK             4  /* seek error */
#define ZERR_READ             5  /* read error */
#define ZERR_WRITE            6  /* write error */
#define ZERR_CRC              7  /* CRC error */
#define ZERR_ZIPCLOSED        8  /* containing zip file closed */
#define ZERR_NOENT            9  /* no such file */
#define ZERR_EXISTS          10  /* file already exists */
#define ZERR_OPEN            11  /* can't open file */
#define ZERR_TMPOPEN         12
#define ZERR_ZLIB            13
#define ZERR_MEMORY          14

extern char *zip_err_str[];

/* zip file */

struct zip {
    char *zn;
    FILE *zp;
    unsigned short comlen, changes;
    unsigned int cd_size, cd_offset;
    char *com;
    int nentry, nentry_alloc;
    struct zf_entry *entry;
    int nfile, nfile_alloc;
    struct zf_file **file;
};

/* file in zip file */

struct zip_file {
    struct zf *zf;
    char *name;
    int flags; /* -1: eof, >0: error */

    int method;
    /* position within zip file (fread/fwrite) */
    long fpos;
    /* no of bytes left to read */
    unsigned long bytes_left;
    /* no of bytes of compressed data left */
    unsigned long cbytes_left;
    /* crc so far */
    unsigned long crc, crc_orig;
    
    char *buffer;
    z_stream *zstr;
};

/* entry in zip file directory */

struct zip_entry {
    unsigned short version_made, version_need, bitflags, comp_meth,
	lmtime, lmdate, fnlen, eflen, fcomlen, disknrstart, intatt;
    unsigned int crc, comp_size, uncomp_size, extatt, local_offset;
    enum zip_state state;
    char *fn, *ef, *fcom;
    char *fn_old;
    /* only use one of the following three for supplying new data
       listed in order of priority, if more than one is set */
    struct zf *ch_data_zf;
    char *ch_data_buf;
    FILE *ch_data_fp;
    /* offset & len of new data in ch_data_fp or ch_data_buf */
    unsigned int ch_data_offset, ch_data_len;
    /* if source is another zipfile, number of file in zipfile */
    unsigned int ch_data_zf_fileno;
};



/* opening/closing zip files */

struct zip *zip_open(char *fn, int checkp);
int zip_close(struct zip *zf);

int zip_name_locate(struct zip *zf, char *fn, int case_sens);

/* read access to files in zip file */

struct zip_file *zip_fopen(struct zip *zf, char *fn, int case_sens);
struct zip_file *zip_fopen_index(struct zip *zf, int fileno);
int zip_fread(struct zip_file *zff, char *outbuf, int toread);
int zip_fclose(struct zip_file *zff);

/* high level routines to modify zip file */

int zip_unchange(struct zip *zf, int idx);
int zip_rename(struct zip *zf, int idx, char *name);
int zip_add_file(struct zip *zf, char *name, FILE *file, int start, int len);
int zip_add_data(struct zip *zf, char *name, char *buf, int start, int len);
int zip_add_zip(struct zip *zf, char *name, struct zf *srczf, int srcidx,
		int start, int len);
int zip_replace_file(struct zip *zf, int idx, char *name, FILE *file,
		     int start, int len);
int zip_replace_data(struct zip *zf, int idx, char *name, char *buf,
		     int start, int len);
int zip_replace_zip(struct zip *zf, int idx, char *name,
		    struct zip *srczf, int srcidx, int start, int len);

int zip_delete(struct zip *zf, int idx);


#endif /* _HAD_ZIP_H */
