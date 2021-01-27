/* $Id$
 *
 * priority.h
 * 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef log_priority_h
#define log_priority_h

/**
 * @file priority.h
 *
 * @brief The priority class provides importance levels with which one can
 * categorize log messages.
 **/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Predefined Levels of priorities. These correspond to the priority levels
 * used by syslog(3).
 **/
 typedef enum {
    /** fatal */	LOG_PRIORITY_FATAL	= 000,
    /** alert */	LOG_PRIORITY_ALERT	= 100,
    /** crit */	      	LOG_PRIORITY_CRIT	= 200,
    /** error */	LOG_PRIORITY_ERROR	= 300,
    /** warn */	      	LOG_PRIORITY_WARN	= 400,
    /** notice */	LOG_PRIORITY_NOTICE	= 500,
    /** info */	      	LOG_PRIORITY_INFO	= 600,
    /** debug */	LOG_PRIORITY_DEBUG	= 700,
    /** trace */	LOG_PRIORITY_TRACE	= 800,
    /** notset */	LOG_PRIORITY_NOTSET	= 900,
    /** unknown */	LOG_PRIORITY_UNKNOWN	= 1000
} log_priority_level_t;

/**
 * @param a_priority a numeric value of the priority.
 * @returns the given priority string name.
 **/
const char* log_priority_to_string(int a_priority);

/**
 * @param a_priority_name a priority string name.
 * @returns the given numeric value of the priority.
 **/
int log_priority_to_int(const char* a_priority_name);

#ifdef __cplusplus
}
#endif

#endif
