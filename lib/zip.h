#ifndef _HAD_ZIP_H
#define _HAD_ZIP_H

#include <sys/types.h>
#include <stdio.h>
#include <zlib.h>

enum zip_state { ZIP_ST_UNCHANGED, ZIP_ST_DELETED, ZIP_ST_REPLACED,
		 ZIP_ST_ADDED, ZIP_ST_RENAMED };
enum zip_cmd { ZIP_CMD_INIT, ZIP_CMD_READ, ZIP_CMD_META, ZIP_CMD_CLOSE };

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
#define ZERR_ZIPCLOSED        8  /* containing zip file was closed */
#define ZERR_NOENT            9  /* no such file */
#define ZERR_EXISTS          10  /* file already exists */
#define ZERR_OPEN            11  /* can't open file */
#define ZERR_TMPOPEN         12  /* failure to create temporary file */
#define ZERR_ZLIB            13  /* zlib error */
#define ZERR_MEMORY          14  /* malloc failure */
#define ZERR_CHANGED         15  /* entry has been changed */
#define ZERR_COMPNOTSUPP     16  /* compression method not supported */
#define ZERR_EOF             17  /* premature EOF */
#define ZERR_INVAL           18  /* invalid argument */
#define ZERR_NOZIP           19  /* not a zip file */
#define ZERR_INTERNAL        20  /* internal error */

extern char *zip_err_str[];

/* zip file */

typedef int (*zip_read_func)(void *state, void *data,
			     int len, enum zip_cmd cmd);

struct zip {
    char *zn;
    FILE *zp;
    unsigned short comlen, changes;
    unsigned int cd_size, cd_offset;
    char *com;
    int nentry, nentry_alloc;
    struct zip_entry *entry;
    int nfile, nfile_alloc;
    struct zip_file **file;
};

/* file in zip file */

struct zip_file {
    struct zip *zf;
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
    struct zip_meta *meta;
    char *fn;
    char *fn_old;
    unsigned int file_fnlen;

    enum zip_state state;
    zip_read_func ch_func;
    void *ch_data;
    int ch_comp;
    struct zip_meta *ch_meta;
};

struct zip_meta {
    unsigned short version_made, version_need, bitflags, comp_method,
	disknrstart, int_attr;
    time_t last_mod;
    unsigned int crc, comp_size, uncomp_size, ext_attr, local_offset;
    unsigned short ef_len, lef_len, fc_len;
    unsigned char *ef, *lef, *fc;
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

/* data structure to set file attributes (except the file name) */

struct zip_meta *zip_new_meta(void);
void zip_free_meta(struct zip_meta *meta);
struct zip_meta *zip_get_meta(struct zip *zf, int idx);

/* high level routines to modify zip file */

int zip_rename(struct zip *zf, int idx, char *name);
int zip_delete(struct zip *zf, int idx);

int zip_replace(struct zip *zf, int idx, char *name, struct zip_meta *meta,
		zip_read_func fn, void *state, int comp);

int zip_add_data(struct zip *zf, char *name, struct zip_meta *meta,
		 char *data, int len, int freep);
int zip_replace_data(struct zip *zf, int idx, char *name,
		     struct zip_meta *meta,
		     char *data, int len, int freep);
int zip_add_file(struct zip *zf, char *name, struct zip_meta *meta,
		     char *fname, int start, int len);
int zip_replace_file(struct zip *zf, int idx, char *name,
		     struct zip_meta *meta,
		     char *fname, int start, int len);
int zip_add_filep(struct zip *zf, char *name, struct zip_meta *meta,
		 FILE *file, int start, int len);
int zip_replace_filep(struct zip *zf, int idx, char *name,
		     struct zip_meta *meta,
		     FILE *file, int start, int len);
int zip_add_zip(struct zip *zf, char *name, struct zip_meta *meta,
		struct zip *srczf, int srcidx, int start, int len);
int zip_replace_zip(struct zip *zf, int idx, char *name,
		    struct zip_meta *meta,
		    struct zip *srczf, int srcidx, int start, int len);

int zip_unchange(struct zip *zf, int idx);


#endif /* _HAD_ZIP_H */
