#ifndef _HAD_ZIPLOW_H
#define _HAD_ZIPLOW_H

#include "zip.h"

#define MAXCOMLEN        65536
#define EOCDLEN             22
#define BUFSIZE       (MAXCOMLEN+EOCDLEN)
#define LOCAL_MAGIC   "PK\3\4"
#define CENTRAL_MAGIC "PK\1\2"
#define EOCD_MAGIC    "PK\5\6"
#define DATADES_MAGIC "PK\7\8"
#define CDENTRYSIZE         46
#define LENTRYSIZE          30

struct zf *readcdir(FILE *fp, unsigned char *buf, unsigned char *eocd, 
		    int buflen);
int writecdir(struct zf *zfp);
struct zf *zf_new(void);
int zf_free(struct zf *zf);
char *readstr(unsigned char **buf, int len, int nullp);
char *readfpstr(FILE *fp, int len, int nullp);
int read2(unsigned char **a);
int read4(unsigned char **a);
int readcdentry(FILE *fp, struct zf_entry *zfe, unsigned char **cdpp, 
		int left, int readp, int localp);
int writecdentry(FILE *fp, struct zf_entry *zfe, int localp);
int checkcons(FILE *fp, struct zf *zf);
int headercomp(struct zf_entry *h1, int local1p, struct zf_entry *h2,
	       int local2p);
int zip_entry_copy(struct zf *dest, struct zf *src, int entry_no,
		   char *name);

#endif /* _HAD_ZIPLOW_H */
