#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "error.h"

#include <errno.h>

extern char *prg;
static char *myerrorfn, *myerrorzipn;



void
myerror(int errtype, char *fmt, ...)
{
    va_list va;

    fprintf(stderr, "%s: ", prg);

    if (((errtype==ERRFILE)||(errtype==ERRSTR)) && myerrorfn) {
	if (myerrorzipn)
	    fprintf(stderr, "%s (%s): ", myerrorfn, myerrorzipn);
	else
	    fprintf(stderr, "%s: ", myerrorfn);
    }

    if (((errtype==ERRZIP)||(errtype==ERRZIPSTR)) && myerrorzipn)
	fprintf(stderr, "%s: ", myerrorzipn);
    
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);

    if ((errno != 0) && (((errtype==ERRSTR) && (!myerrorzipn)) ||
	((errtype==ERRZIPSTR) && (myerrorzipn))))
	fprintf(stderr, ": %s", strerror(errno));
    
    putc('\n', stderr);

    return;
}



void
seterrinfo(char *fn, char *zipn)
{
    myerrorfn = fn;
    myerrorzipn = zipn;
    
    return;
}

