#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "error.h"

extern char* progname;

/* error: print error message and terminate. for user error. */
void error(char* fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);

	fprintf(stderr, "%s: error: ", progname);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);

	va_end(ap);

	exit(1);
}

/* syserror: print error message and terminate. for failed system call. */
void syserror(int errnum, char* fmt, ...)
{
	va_list		ap;	
	char*		str;	

	va_start(ap, fmt);

	str = strerror(errnum);

	fprintf(stderr, "%s: error: ", progname);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, ": %s", str);
	fputc('\n', stderr);

	va_end(ap);

	exit(1);
}
