
#ifndef _HAD_ZIPINT_H
#define _HAD_ZIPINT_H

/*
  $NiH: zipint.h,v 1.22.4.6 2004/04/06 20:30:08 dillo Exp $

  zipint.h -- internal declarations.
  Copyright (C) 1999, 2003 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <nih@giga.or.at>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.
 
  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



#define MAXCOMLEN        65536
#define EOCDLEN             22
#define BUFSIZE       (MAXCOMLEN+EOCDLEN)
#define LOCAL_MAGIC   "PK\3\4"
#define CENTRAL_MAGIC "PK\1\2"
#define EOCD_MAGIC    "PK\5\6"
#define DATADES_MAGIC "PK\7\8"
#define CDENTRYSIZE         46
#define LENTRYSIZE          30



/* state of change of a file in zip archive */

enum zip_state { ZIP_ST_UNCHANGED, ZIP_ST_DELETED, ZIP_ST_REPLACED,
		 ZIP_ST_ADDED, ZIP_ST_RENAMED };

/* error information */

struct zip_error {
    int zip_err;	/* libzip error code (ZERR_*) */
    int sys_err;	/* copy of errno (E*) or zlib error code */
    char *str;		/* string representation or NULL */
};

/* zip archive, part of API */

struct zip {
    char *zn;			/* file name */
    FILE *zp;			/* file */
    struct zip_error error;	/* error information */

    struct zip_cdir *cdir;	/* central directory */
    int nentry;			/* number of entries */
    int nentry_alloc;		/* number of entries allocated */
    struct zip_entry *entry;	/* entries */
    int nfile;			/* number of opened files within archvie */
    int nfile_alloc;		/* number of files allocated */
    struct zip_file **file;	/* opened files within archvie */
};

/* file in zip archive, part of API */

struct zip_file {
    struct zip *zf;		/* zip archive containing this file */
    char *name;			/* name of file */
    struct zip_error error;	/* error information */
    int flags;			/* -1: eof, >0: error */

    int method;			/* compression method */
    long fpos;			/* position within zip file (fread/fwrite) */
    unsigned long bytes_left;	/* number of bytes left to read */
    unsigned long cbytes_left;  /* number of bytes of compressed data left */
    
    unsigned long crc;		/* crc so far */
    unsigned long crc_orig;	/* CRC recorded in archive */
    
    char *buffer;
    z_stream *zstr;
};

/* zip archive directory entry (central or local) */

struct zip_dirent {
    unsigned short version_madeby;	/* (c)  version of creator */
    unsigned short version_needed;	/* (cl) version needed to extract */
    unsigned short bitflags;		/* (cl) general purposee bit flag */
    unsigned short comp_method;		/* (cl) compression method used */
    time_t last_mod;			/* (cl) time of last modification */
    unsigned int crc;			/* (cl) CRC-32 of uncompressed data */
    unsigned int comp_size;		/* (cl) size of commpressed data */
    unsigned int uncomp_size;		/* (cl) size of uncommpressed data */
    char *filename;			/* (cl) file name (NUL-terminated) */
    unsigned short filename_len;	/* (cl) length of filename (w/o NUL) */
    char *extrafield;			/* (cl) extra field */
    unsigned short extrafield_len;	/* (cl) length of extra field */
    char *comment;			/* (c)  file comment */
    unsigned short comment_len;		/* (c)  length of file comment */
    unsigned short disk_number;		/* (c)  disk number start */
    unsigned short int_attrib;		/* (c)  internal file attributes */
    unsigned int ext_attrib;		/* (c)  external file attributes */
    unsigned int offset;		/* (c)  offest of local header  */
};

/* zip archive central directory */

struct zip_cdir {
    struct zip_dirent *entry;	/* directory entries */
    int nentry;			/* number of entries */

    unsigned int size;		/* size of central direcotry */
    unsigned int offset;	/* offset of central directory in file */
    char *comment;		/* zip archive comment */
    unsigned short comment_len;	/* length of zip archive comment */
};

/* entry in zip archive directory */

struct zip_entry {
#if 0
    char *fn;			/* file name */
    unsigned short comp_method;	/* compression method */
    unsigned int comp_size;	/* size of compressed data */
    unsigned int uncomp_size;	/* size of uncompressed data */
    unsigned int crc;		/* CRC-32 of uncompressed data */
    unsigned int offset;	/* byte offset of local dir entry */
    time_t mtime;		/* time of last modification */
#endif

    enum zip_state state;
    zip_read_func ch_func;
    void *ch_data;
    int ch_flags;		/* 1: data returned by ch_func is compressed */
    char *ch_filename;
    time_t ch_mtime;
    int ch_comp_method;
};



extern const char * const _zip_err_str[];
extern const int _zip_nerr_str;
extern const int _zip_err_type[];



void _zip_cdir_free(struct zip_cdir *);
void _zip_dirent_finalize(struct zip_dirent *);
int _zip_dirent_read(struct zip_dirent *, FILE *,
		     unsigned char **, int, int, struct zip_error *);
int _zip_dirent_write(struct zip_dirent *, FILE *, int, struct zip_error *);
void _zip_entry_init(struct zip *, int);
void _zip_error_copy(struct zip_error *, struct zip_error *);
void _zip_error_fini(struct zip_error *);
void _zip_error_get(struct zip_error *, int *, int *);
void _zip_error_init(struct zip_error *);
void _zip_error_set(struct zip_error *, int, int);
const char *_zip_error_strerror(struct zip_error *);
int _zip_file_fillbuf(void *, size_t, struct zip_file *);
unsigned int _zip_file_get_offset(struct zip *, int);
void _zip_free(struct zip *);
int _zip_free_entry(struct zip_entry *);
int _zip_local_header_read(struct zip *, int);
void *_zip_memdup(const void *, int);
struct zip *_zip_new(struct zip_error *);
struct zip_entry *_zip_new_entry(struct zip *);
unsigned short _zip_read2(unsigned char **);
unsigned int _zip_read4(unsigned char **);
int _zip_replace(struct zip *, int, const char *,zip_read_func, void *, int);
int _zip_replace_data(struct zip *, int, const char *,
		      const void *, off_t, int);
int _zip_replace_file(struct zip *, int, const char *,
		      const char *, off_t, off_t);
int _zip_replace_filep(struct zip *, int, const char *, FILE *, off_t, off_t);
int _zip_replace_zip(struct zip *zf, int idx, const char *name,
		     struct zip *srczf, int srcidx, off_t start, off_t len);
int _zip_set_name(struct zip *, int, const char *);
int _zip_unchange(struct zip_entry *);
int _zip_unchange_data(struct zip_entry *);

#endif /* zipint.h */

