#ifndef log_appender_h
#define log_appender_h

/**
 * @file appender.h
 *
 * @brief Implement this interface for your own strategies for printing log
 * statements.
 *
 * @todo the appender interface needs a better configuration system
 * depending on the layout type. The udata field is a just a trick.
 **/

#include <stdio.h>
#include "layout.h"

#ifdef __cplusplus
extern "C" {
#endif

struct __log_appender;

/**
 * log appender class
 **/
typedef struct __log_appender log_appender_t;

/**
 * @brief 输出方向
 *
 * Attributes description:
 * 
 * @li @c name appender type name 
 * @li @c open
 * @li @c append
 * @li @c close
 **/
typedef struct log_appender_type {
    const char*	  name;
    int (*open)	  (log_appender_t*);
    int (*append) (log_appender_t*, const log_logging_event_t*);
    int (*close)  (log_appender_t*);
} log_appender_type_t;

/**
 * Get a pointer to an existing appender type.
 * 获取指向现有追加器类型的指针
 *
 * @param a_name the name of the appender type to return.  
 * @returns a pointer to an existing appender type, or NULL if no appender
 * type with the specified name exists.
 **/
const log_appender_type_t* log_appender_type_get(const char* a_name);

/**
 * Use this function to register an appender type with log.
 * Once this is done you may refer to this type by name both 
 * programmatically and in the log
 * configuration file.
 *
 * @param a_type a pointer to the new appender type to set.
 * @returns a pointer to the previous appender type of same name.
 * 
 * Example code fragment: 
 * @code
 * 
 * const log_appender_type_t log_appender_type_s13_file = {
 *   "s13_file",
 *   s13_file_open,
 *   s13_file_append,
 *   s13_file_close,
 * };
 *  
 *  log_appender_type_set(&log_appender_type_s13_file);
 * @endcode
 **/
const log_appender_type_t* log_appender_type_set(
    const log_appender_type_t* a_type);

/**
 * Get a pointer to an existing appender.
 *
 * @param a_name the name of the appender to return.
 * @returns a pointer to an existing appender, or NULL if no appender
 * with the specfied name exists.
 **/
log_appender_t* log_appender_get(const char* a_name);

/**
 * Constructor for log_appender_t.
 **/
log_appender_t* log_appender_new(const char* a_name);

/**
 * Destructor for log_appender_t.
 **/
void log_appender_delete(log_appender_t* a_appender);

/**
 * @param a_appender the log_appender_t object
 * @return the appender name
 **/
const char* log_appender_get_name(const log_appender_t* a_appender);

/**
 * @param a_appender the log_appender_t object
 * @return the appender operations
 **/
const log_appender_type_t* log_appender_get_type(
    const log_appender_t* a_appender);

/**
 * @param a_appender the log_appender_t object
 * @return the appender layout
 **/
const log_layout_t* log_appender_get_layout(
    const log_appender_t* a_appender);

/**
 * @param a_appender the log_appender_t object
 * @return the appender user data
 **/
void* log_appender_get_udata(const log_appender_t* a_appender);

/**
 * sets the appender type
 *
 * @param a_appender the log_appender_t object
 * @param a_type the new appender type
 * @return the previous appender type
 **/
const log_appender_type_t* log_appender_set_type(
    log_appender_t* a_appender,
    const log_appender_type_t* a_type);

/**
 * sets the appender user data
 *
 * @param a_appender the log_appender_t object
 * @param a_udata the new appender user data
 * @return the previous appender user data
 **/
void* log_appender_set_udata(log_appender_t*	a_appender,
				      void* a_udata);

/**
 * sets the appender layout
 *
 * @param a_appender the log_appender_t object
 * @param a_layout the new appender layout
 * @return the previous appender layout
 **/
const log_layout_t* log_appender_set_layout(
    log_appender_t* a_appender,
    const log_layout_t* a_layout);

/**
 * opens the appender.
 *
 * @param a_appender the log_appender_t object
 **/
int log_appender_open(log_appender_t* a_appender);

/**
 * log in appender specific way.
 *
 * @param a_appender the log_appender object
 * @param a_event the log_logging_event_t object to log.
 **/
int log_appender_append(
    log_appender_t* a_appender,
    log_logging_event_t* a_event);

/**
 * closes the appender
 *
 * @param a_appender the log_appender_t object
 * @return zero if successful, -1 otherwise
 **/
int log_appender_close(log_appender_t* a_appender);

/**
 * prints the appender on a stream
 *
 * @param a_appender the log_appender_t object
 * @param a_stream the stream
 **/
void log_appender_print(const log_appender_t* a_appender,
				 FILE* a_stream);
     
/**
 * prints all the current registered appender types on a stream
 *
 * @param fp the stream
 **/                            
void log_appender_types_print(FILE *fp);

/**
 * Helper macro to define static appender types.
 *
 * @param a_type the log_appender_type_t object to define
 * @warning needs GCC support: otherwise this macro does nothing
 * @deprecated This macro, and the static initialialization
 * of appenders in general, is deprecated. Use rather
 * the log_appender_type_set() function to initialize your appenders
 * before calling log_init()
 *
 **/
#ifdef __GNUC__
#   define log_appender_type_define(a_type) \
    typedef int log_appender_type_define_##a_type __attribute__((deprecated)); \
    static log_appender_type_define_##a_type __unsused_var __attribute__ ((unused));
#else
#   define log_appender_type_define(a_type)
#endif

/**
 * @internal
 **/
struct __sd_factory;
struct __sd_factory* log_appender_factory;

#ifdef __cplusplus
}
#endif

#endif
