#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "layout.h"
#include "priority.h"
#include "sprintf.h"

/*******************************************************************************/
static const char* dated_format(
    const log_layout_t*  	a_layout,
    const log_logging_event_t*a_event)
{
    static char buffer[1024];

#ifndef _WIN32
    struct tm   tm;
    gmtime_r(&a_event->evt_timestamp.tv_sec, &tm);
    snprintf(buffer, sizeof(buffer), "%04d%02d%02d %02d:%02d:%02d.%03ld %-8s %s- %s\n",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec,
             a_event->evt_timestamp.tv_usec / 1000,
             log_priority_to_string(a_event->evt_priority),
             a_event->evt_category, a_event->evt_msg);
#else
        SYSTEMTIME stime;

        if ( FileTimeToSystemTime(&a_event->evt_timestamp, &stime)){

    snprintf(buffer, sizeof(buffer), "%04d%02d%02d %02d:%02d:%02d.%03ld %-8s %s- %s\n",
             stime.wYear, stime.wMonth , stime.wDay,
             stime.wHour, stime.wMinute, stime.wSecond,
             stime.wMilliseconds,
             log_priority_to_string(a_event->evt_priority),
             a_event->evt_category, a_event->evt_msg);
        }
#endif
    return buffer;
}

/*******************************************************************************/
const log_layout_type_t log_layout_type_dated = {
    "dated",
    dated_format,
};

