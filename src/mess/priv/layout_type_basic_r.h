#ifndef log_layout_type_basic_r_h
#define log_layout_type_basic_r_h

/**
 * @file layout_type_basic_r.h
 *
 * @brief Implement a basic_r layout.
 *
 * In @c log4j.PatternLayout conventions, the basic_r layout has the following
 * conversion pattern: @c "%P %c - %m\n".
 *
 * Where
 * @li @c "%P" is the priority of the logging event
 * @li @c "%c" is the category of the logging event
 * @li @c "%m" is the application supplied message associated with the
 * logging event
 *
 **/

#include "layout.h"

#ifdef __cplusplus
extern "C" {
#endif


extern const log_layout_type_t log_layout_type_basic_r;

#ifdef __cplusplus
}
#endif

#endif
