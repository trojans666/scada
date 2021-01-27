#ifndef log_location_info_h
#define log_location_info_h

/**
 * @file location_info.h
 *
 * @brief The internal representation of caller location information.
 * 
 * When a affirmative logging decision is made a log_location_info_t is
 * created and is passed around the different log components.
 **/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief logging location information
 *
 * Attributes description:
 * 
 * @li @c loc_file file name
 * @li @c loc_line file line
 * @li @c loc_function function name
 * @li @c loc_data user data
 *
 * @todo this is not used
 **/
typedef struct 
{
    const char* loc_file;
    int loc_line;
    const char* loc_function;
    void* loc_data;

} log_location_info_t;

/**
 * log_location_info_t initializer
 **/
#ifdef __GNUC__
#   define LOG_LOCATION_INFO_INITIALIZER(user_data) { __FILE__, __LINE__, __FUNCTION__, user_data }
#else
#   define LOG_LOCATION_INFO_INITIALIZER(user_data) { __FILE__, __LINE__, "(nil)", user_data }
#endif

#define __log_str(n) #n

#ifdef __GNUC__
#   define __log_location(n)	__FUNCTION__ "() at " __FILE__ ":" __log_str(n)
#else
#   define __log_location(n)	__FILE__ ":" __log_str(n)
#endif

/**
 * This macro returns the literal representation of a logging event
 * location
 **/
#define log_location __log_location(__LINE__)

#ifdef __cplusplus
}
#endif

#endif
