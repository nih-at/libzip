#ifndef _HAD_ZIP_H
#define _HAD_ZIP_H

/*
  $NiH: zip.h,v 1.32 2003/10/02 14:13:28 dillo Exp $

  zip.h -- exported declarations.
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



#include <sys/types.h>
#include <stdio.h>
#include <zlib.h>
#include <time.h>

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
#define ZERR_MULTIDISK        1  /* multi-disk zip archives not supported */
#define ZERR_RENAME           2  /* renaming temporary file failed */
#define ZERR_CLOSE            3  /* closing zip archive failed */
#define ZERR_SEEK             4  /* seek error */
#define ZERR_READ             5  /* read error */
#define ZERR_WRITE            6  /* write error */
#define ZERR_CRC              7  /* CRC error */
#define ZERR_ZIPCLOSED        8  /* containing zip archive was closed */
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
#define ZERR_NOZIP           19  /* not a zip archive */
#define ZERR_INTERNAL        20  /* internal error */
#define ZERR_INCONS	     21  /* zip archive inconsistent */

extern const char * const zip_err_str[];

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
    int ch_comp;		/* 1: data returned by ch_func is compressed */
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



int zip_add(struct zip *, const char *, struct zip_meta *,
	    zip_read_func, void *, int);
int zip_add_data(struct zip *, const char *, struct zip_meta *,
		 const char *, int, int);
int zip_add_file(struct zip *, const char *, struct zip_meta *,
		 const char *, int, int);
int zip_add_filep(struct zip *, const char *, struct zip_meta *,
		 FILE *, int, int);
int zip_add_zip(struct zip *, const char *, struct zip_meta *,
		struct zip *, int, int, int);
int zip_close(struct zip *);
int zip_delete(struct zip *, int);
int zip_fclose(struct zip_file *);
struct zip_file *zip_fopen(struct zip *, const char *, int);
struct zip_file *zip_fopen_index(struct zip *, int);
int zip_fread(struct zip_file *, char *, int);
void zip_free_meta(struct zip_meta *);
struct zip_meta *zip_get_meta(struct zip *, int);
const char *zip_get_name(struct zip *, int);
int zip_name_locate(struct zip *, const char *, int);
struct zip_meta *zip_new_meta(void);
struct zip *zip_open(const char *, int);
int zip_rename(struct zip *, int, const char *);
int zip_replace(struct zip *, int, const char *, struct zip_meta *,
		zip_read_func, void *, int);
int zip_replace_data(struct zip *, int, const char *, struct zip_meta *,
		     const char *, int, int);
int zip_replace_file(struct zip *, int, const char *, struct zip_meta *,
		     const char *, int, int);
int zip_replace_filep(struct zip *, int, const char *, struct zip_meta *,
		     FILE *, int, int);
int zip_replace_zip(struct zip *, int, const char *, struct zip_meta *,
		    struct zip *, int, int, int);
int zip_unchange(struct zip *, int);
int zip_unchange_all(struct zip *);

#endif /* _HAD_ZIP_H */
