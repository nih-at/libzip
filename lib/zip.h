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

/* 0 is no error */
#define ZERR_MULTIDISK        1
#define ZERR_RENAME           2
#define ZERR_CLOSE            3
#define ZERR_SEEK             4
#define ZERR_READ             5
#define ZERR_WRITE            6
#define ZERR_CRC              7
#define ZERR_ZIPCLOSED        8
#define ZERR_FILEEXISTS       9
#define ZERR_FILENEXISTS     10
#define ZERR_OPEN            11
#define ZERR_TMPOPEN         12
#define ZERR_ZLIB            13
#define ZERR_MEMORY          14

extern char *zip_err_str[];

/* zip file */

struct zf {
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

struct zf_file {
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

struct zf_entry {
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

struct zf *zip_open(char *fn, int checkp);
int zip_close(struct zf *zf);

int zip_name_locate(struct zf *zf, char *fn, int case_sens);

/* read access to files in zip file */

struct zf_file *zff_open(struct zf *zf, char *fn, int case_sens);
struct zf_file *zff_open_index(struct zf *zf, int fileno);
int zff_read(struct zf_file *zff, char *outbuf, int toread);
int zff_close(struct zf_file *zff);

/* high level routines to modify zip file */

int zip_unchange(struct zf *zf, int idx);
int zip_rename(struct zf *zf, int idx, char *name);
int zip_add_file(struct zf *zf, char *name, FILE *file, int start, int len);
int zip_add_data(struct zf *zf, char *name, char *buf, int start, int len);
int zip_add_zip(struct zf *zf, char *name, struct zf *srczf, int srcidx,
		int start, int len);
int zip_replace_file(struct zf *zf, int idx, char *name, FILE *file,
		     int start, int len);
int zip_replace_data(struct zf *zf, int idx, char *name, char *buf,
		     int start, int len);
int zip_replace_zip(struct zf *zf, int idx, char *name,
		    struct zf *srczf, int srcidx, int start, int len);

int zip_delete(struct zf *zf, int idx);


#endif /* _HAD_ZIP_H */
