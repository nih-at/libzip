/* error types: default error message (no filename), */
#define ERRDEF    0
/* or file error, which also outputs filename (and zipfilename), */
#define ERRFILE   1
/* or zipfile error, which only outputs the zipfilename, */
#define ERRZIP    2
/* or file error with additional strerror(errno) output, */
#define ERRSTR    3
/* or zipfile error with additional strerror(errno) output */
#define ERRZIPSTR 4

void myerror(int errtype, char *fmt, ...);
void seterrinfo(char *fn, char *zipn);
