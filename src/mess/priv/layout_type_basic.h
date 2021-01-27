#ifndef log_layout_type_basic_h
#define log_layout_type_basic_h

/**
 * @file layout_type_basic.h
 *
 * @brief Implement a basic layout.
 *
 * In @c log4j.PatternLayout conventions, the basic layout has the following
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


extern const log_layout_type_t log_layout_type_basic;

#ifdef __cplusplus
}
#endif

#endif
