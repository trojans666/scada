
#ifndef __log_buffer_h
#define __log_buffer_h

/**
 * @file buffer.h
 *
 * @brief log buffer
 *
 **/

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief buffer object
 *
 * Attributes description:
 *
 * @li @c size current size of the buffer
 * @li @c maxsize maximum size of the buffer. 0 means no limitation.
 * @li @c data raw data
 **/
typedef struct
{
    size_t buf_size;
    size_t buf_maxsize;
    char*  buf_data;

} log_buffer_t;

#define LOG_BUFFER_SIZE_DEFAULT  512


#ifdef __cplusplus
}
#endif

#endif
