#ifndef __log_init_h
#define __log_init_h


#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @file init.h
 *
 * @brief log constructors and destructors
 *
 **/   

/**
 * constructor
 * 
 * @returns 0 for success 
 **/
int log_init(void);

/**
 * destructor
 *
 * @returns 0 for success 
 **/
int log_fini(void);

/*
 * Dumps all the current appender, layout and rollingpolicy types
 * known by log.
 * @param stream to write to
 */
void log_dump_all_types(FILE *fp);

/*
 * Dumps all the current instances of categories, appenders, layouts
 * and rollingpolicy objects.
 * An instances of a type consists of the base
 * type information (name plus function table) and an instance name and
 * configuration.  For example one can have an instance of the rollingfile
 * appender which logs to /var/tmp and another instance which logs to 
 * /usr/tmp.  They are both of type rollingfile, but are distinct instances of
 * it
 * @param stream to write t
 */
void log_dump_all_instances(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
