#ifndef log_appender_type_stream_h
#define log_appender_type_stream_h

/**
 * @file appender_type_stream.h
 *
 * @brief log stream appender interface.
 *
 * The stream appender uses a file handle @c FILE* for logging. The
 * appender's name is used as the file name which will be opened at first
 * log. An appender can also be associated to an opened file handle using
 * the log_appender_set_udata() method to update the appender user data
 * field. In this last case, the appender name has no meaning. 2 default
 * stream appenders are defined: @c "stdout" and @c "stderr".
 *
 * The following examples shows how to define and use stream appenders.
 * 
 * @li the simple way
 * @code
 *
 * log_appender_t* myappender;
 *
 * myappender = log_appender_get("myfile.log");
 * log_appender_set_type(myappender, &log_appender_type_stream);
 * 
 * @endcode
 *
 * @li the sophisticated way
 * @code
 *
 * log_appender_t* myappender;
 *
 * myappender = log_appender_get("myappender");
 *    
 * log_appender_set_type(myappender, &log_appender_type_stream);
 * log_appender_set_udata(myappender, fopen("myfile.log", "w"));
 *
 * @endcode
 *
 **/


#include "appender.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Stream appender type definition.
 *
 * This should be used as a parameter to the log_appender_set_type()
 * routine to set the type of the appender.
 *
 **/
extern const log_appender_type_t log_appender_type_stream;

#ifdef __cplusplus
}
#endif

#endif
