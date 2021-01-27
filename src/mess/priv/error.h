#ifndef __sd_error_h
#define __sd_error_h

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


extern int sd_debug(const char *fmt, ...);
extern int sd_error(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __sd_error_h */
