#ifndef _HAD_ZIPINT_H
#define _HAD_ZIPINT_H


#define MAXCOMLEN        65536
#define EOCDLEN             22
#define BUFSIZE       (MAXCOMLEN+EOCDLEN)
#define LOCAL_MAGIC   "PK\3\4"
#define CENTRAL_MAGIC "PK\1\2"
#define EOCD_MAGIC    "PK\5\6"
#define DATADES_MAGIC "PK\7\8"
#define CDENTRYSIZE         46
#define LENTRYSIZE          30

struct zip *_zip_new(void);
int _zip_free(struct zip *zf);
int _zip_readcdentry(FILE *fp, struct zip_entry *zfe, unsigned char **cdpp, 
		     int left, int readp, int localp);

#endif /* zipint.h */
