#ifndef log_layout_type_dated_h
#define log_layout_type_dated_h

/**
 * @file layout_type_dated.h
 *
 * @brief Implement a dated layout.
 *
 * In @c log4j.PatternLayout conventions, the dated layout has the following
 * conversion pattern: @c "%d %P %c - %m\n".
 *
 * Where 
 * @li @c "%d" is the date of the logging event
 * @li @c "%P" is the priority of the logging event
 * @li @c "%c" is the category of the logging event
 * @li @c "%m" is the application supplied message associated with the
 * logging event
 *
 * 
 * 
 **/


#include "layout.h"

#ifdef __cplusplus
extern "C" {
#endif


extern const log_layout_type_t log_layout_type_dated;

#ifdef __cplusplus
}
#endif

#endif
