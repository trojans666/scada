#ifndef log_layout_h
#define log_layout_h

/**
 * @file layout.h
 *
 * @brief Interface for user specific layout format of log_logging_event
 * events. 
 *
 * @todo the layout interface needs a better configuration system
 * depending on the layout type. The udata field is a just a trick.
 *
 * @todo a pattern layout would be welcomed !!
 **/


#include <stdio.h>

#include "logging_event.h"

#ifdef __cplusplus
extern "C" {
#endif

struct __log_layout;

/**
 * log layout class
 **/
typedef struct __log_layout log_layout_t;

/**
 * @brief log layout type class
 *
 * Attributes description:
 * 
 * @li @c name layout type name 
 * @li @c format 
 **/
typedef struct log_layout_type {
    const char* name;
    const char* (*format) (const log_layout_t*, const log_logging_event_t*);
} log_layout_type_t;

/**
 * Get a pointer to an existing layout type.
 *
 * @param a_name the name of the layout type to return.  
 * @returns a pointer to an existing layout type, or NULL if no layout
 * type with the specified name exists.
 **/
const log_layout_type_t* log_layout_type_get(const char* a_name);

/**
 * Use this function to register a layout type with log.
 * Once this is done you may refer to this type by name both 
 * programatically and in the log configuration file.
 *
 * @param a_type a pointer to the new layout type to set.
 * @returns a pointer to the previous layout type of same name.
 *
 * Example code fragment: 
 * @code
 * 
 * const log_layout_type_t log_layout_type_xml = {
 *    "s13_xml",
 *    xml_format,
 * };
 *  
 * log_layout_type_set(&log_layout_type_xml);
 *
 * @endcode
 **/
const log_layout_type_t* log_layout_type_set(
    const log_layout_type_t* a_type);

/**
 * Get a pointer to an existing layout.
 *
 * @param a_name the name of the layout to return.
 * @returns a pointer to an existing layout, or NULL if no layout
 * with the specfied name exists.
 **/
log_layout_t* log_layout_get(const char* a_name);

/**
 * Constructor for layout. 
 **/
log_layout_t* log_layout_new(const char* a_name);

/**
 * Destructor for layout.
 **/
void log_layout_delete(log_layout_t* a_layout);

/**
 * @param a_layout the log_layout_t object
 * @return the layout name
 **/
const char* log_layout_get_name(const log_layout_t* a_layout);

/**
 * @param a_layout the log_layout_t object
 * @return a log_layout_type_t object
 **/
const log_layout_type_t* log_layout_get_type(
    const log_layout_t* a_layout);

/**
 * sets the layout type
 *
 * @param a_layout the log_layout_t object
 * @param a_type the new layout type
 * @return the previous layout type
 *
 **/
const log_layout_type_t* log_layout_set_type(
    log_layout_t* a_layout,
    const log_layout_type_t* a_type);

/**
 * @param a_layout the log_layout_t object
 * @return the layout user data
 **/
void* log_layout_get_udata(const log_layout_t* a_layout);

/**
 * sets the layout user data
 *
 * @param a_layout the log_layout_t object
 * @param a_udata the new layout user data
 * @return the previous layout user data
 **/
void* log_layout_set_udata(log_layout_t*	a_layout,
				    void*		a_udata);
/**
 * format a log_logging_event events to a string.
 *
 * @param a_layout the log_layout_t object
 * @param a_event a logging_event_t object
 * @returns an appendable string.
 **/
const char* log_layout_format(
    const log_layout_t*		a_layout,
    const log_logging_event_t*	a_event);

/**
 * prints the layout on a stream
 * @param a_layout the log_layout_t object
 * @param a_stream the stream
 **/
void log_layout_print(
    const log_layout_t* a_layout, FILE* a_stream);

/**
 * prints all the current registered layout types on a stream
 *
 * @param fp the stream
 **/                            
void log_layout_types_print(FILE *fp);

/**
 * Helper macro to define static layout types.
 *
 * @param a_type the log_layout_type_t object to define
 * @warning needs GCC support: otherwise this macro does nothing
 * @deprecated This macro, and the static initialialization
 * of layouts in general, is deprecated. Use rather
 * the log_layout_type_set() function to initialize your appenders
 * before calling log_init()
 **/
#ifdef __GNUC__
#   define log_layout_type_define(a_type) \
    typedef int log_layout_type_define_##a_type __attribute__((deprecated)); \
    static log_layout_type_define_##a_type __unsused_var __attribute__((unused));
#else
#   define log_layout_type_define(a_type)
#endif

/**
 * @internal
 **/
struct __sd_factory;
struct __sd_factory* log_layout_factory;

#ifdef __cplusplus
}
#endif

#endif
