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
int _zip_file_fillbuf(char *buf, int buflen, struct zip_file *zff);
void *_zip_memdup(const void *mem, int len);
void _zip_entry_init(struct zip *zf, int idx);
int _zip_set_name(struct zip *zf, int idx, char *name);
struct zip_entry *_zip_new_entry(struct zip *zf);
int _zip_free_entry(struct zip_entry *ze);
int _zip_unchange_data(struct zip_entry *ze);
int _zip_unchange(struct zip_entry *ze);
int _zip_merge_meta(struct zip_meta *dest, struct zip_meta *src);
int _zip_merge_meta_fix(struct zip_meta *dest, struct zip_meta *src);

#endif /* zipint.h */
