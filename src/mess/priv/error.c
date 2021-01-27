
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdarg.h>


#include "error.h"


int sd_debug(const char *fmt, ...)
{
    va_list args;
    int r;

    if (!getenv("SD_DEBUG"))
	return 0;

    r = fprintf(stderr, "[DEBUG] ");
    va_start(args, fmt);
    r += vfprintf(stderr, fmt, args);
    va_end(args);
    r += fprintf(stderr, "\n");

    return r;
}

int sd_error(const char *fmt, ...)
{
    va_list args;
    int r;

    if (!getenv("SD_ERROR"))
	return 0;

    r = fprintf(stderr, "[ERROR] ");
    va_start(args, fmt);
    r += vfprintf(stderr, fmt, args);
    va_end(args);
    r += fprintf(stderr, "\n");

    return r;
}
