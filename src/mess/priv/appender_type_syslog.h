#ifndef log_appender_type_syslog_h
#define log_appender_type_syslog_h

/**
 * @file appender_type_syslog.h
 *
 * @brief log syslog(3) appender interface.
 *
 * The syslog appender uses the syslog(3) interface for logging. The log
 * priorities are mapped to the syslog priorities and the appender name is
 * used as a syslog identifier. 1 default syslog appender is defined: @c
 * "syslog".
 *
 * The following examples shows how to define and use syslog appenders.
 * 
 * @code
 *
 * log_appender_t* myappender;
 *
 * myappender = log_appender_get("myappender");
 * log_appender_set_type(myappender, &log_appender_type_syslog);
 * 
 * @endcode
 *
 **/

#include "appender.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Syslog appender type definition.
 *
 * This should be used as a parameter to the log_appender_set_type()
 * routine to set the type of the appender.
 *
 **/
extern const log_appender_type_t log_appender_type_syslog;

extern int log_appender_syslog_set_facility(const log_appender_t*, int);

#ifdef __cplusplus
}
#endif

#endif
