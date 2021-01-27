#include "appender.h"
#include "priority.h"

#ifdef __linux__
#include <syslog.h>
#endif

/*******************************************************************************/
static int log_to_syslog_priority(int a_priority)
{
    static int priorities[] = {
	LOG_EMERG,	
	LOG_ALERT,	
	LOG_CRIT, 
	LOG_ERR,	
	LOG_WARNING,	
	LOG_NOTICE, 
	LOG_INFO, 	
	LOG_DEBUG 
    };
    int result;
    
    a_priority++;
    a_priority /= 100;
    
    if (a_priority < 0) {
	result = LOG_EMERG;
    } else if (a_priority > 7) {
	result = LOG_DEBUG;
    } else {
	result = priorities[a_priority];
    }
    
    return result;
}

/*******************************************************************************/
static int syslog_open(log_appender_t* this)
{
    int facility = (int) log_appender_get_udata(this);

    if (!facility)
	facility = LOG_USER;

    openlog(log_appender_get_name(this), LOG_PID, facility);
    return 0;
}

/*******************************************************************************/
static int syslog_append(log_appender_t*	this,
             const log_logging_event_t* a_event)
{
    int facility = (int) log_appender_get_udata(this);

    if (!facility)
	facility = LOG_USER;

    syslog(log_to_syslog_priority(a_event->evt_priority) | facility,
	   a_event->evt_rendered_msg); 
    return 0;
}

/*******************************************************************************/
static int syslog_close(log_appender_t*	this)
{
    closelog();
    return 0;
}



extern int log_appender_syslog_set_facility(log_appender_t* this, int facility)
{
    return (int) log_appender_set_udata(this, (void*) facility);
}

/*******************************************************************************/
const log_appender_type_t log_appender_type_syslog = {
    "syslog",
    syslog_open,
    syslog_append,
    syslog_close,
};

