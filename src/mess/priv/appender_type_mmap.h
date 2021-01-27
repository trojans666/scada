
#ifndef log_appender_type_mmap_h
#define log_appender_type_mmap_h

/**
 * @file appender_type_mmap.h
 *
 * @brief log mmap(2) appender interface.
 *
 * The mmap appender uses a fixed length memory mapped file for
 * logging. The appender's name is used as the file name which will be
 * opened and mapped to memory at first use. The memory mapped file is then
 * used as a rotating buffer in which logging events are written.
 *
 * The following examples shows how to define and use mmap appenders.
 * 
 * @code
 *
 * log_appender_t* myappender;
 *
 * myappender = log_appender_get("myfile.log");
 * log_appender_set_type(myappender, &log_appender_type_mmap);
 * 
 * @endcode
 *
 * @warning the file is not created at first use. It should already exist
 * and have a reasonable size, a mutilple of a page size.
 *
 **/

#include "appender.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Mmap appender type definition.
 *
 * This should be used as a parameter to the log_appender_set_type()
 * routine to set the type of the appender.
 *
 **/
extern const log_appender_type_t log_appender_type_mmap;

#ifdef __cplusplus
}
#endif

#endif
