
#include <stdio.h>
#include <string.h>

#include "appender.h"

/*******************************************************************************/
static int stream_open(log_appender_t* this)
{
    FILE* fp = log_appender_get_udata(this);
    
    if (fp)
	return 0;
    
    if ( (fp = fopen(log_appender_get_name(this), "w+")) == NULL)
	fp = stderr;
    
    /* unbuffered mode */
    setbuf(fp, NULL);
    
    log_appender_set_udata(this, fp);
    return 0;
}

/*******************************************************************************/
static int stream_append(log_appender_t* this,
             const log_logging_event_t* a_event)
{
    FILE* fp = log_appender_get_udata(this);
    
    return fprintf(fp, "[%s] %s", log_appender_get_name(this),
		   a_event->evt_rendered_msg);
}

/*******************************************************************************/
static int stream_close(log_appender_t* this)
{
    FILE* fp = log_appender_get_udata(this);
    

    if (!fp || fp == stdout || fp == stderr)
	return 0;

    return fclose(fp);
}

/*******************************************************************************/
const log_appender_type_t log_appender_type_stream = {
    "stream",
    stream_open,
    stream_append,
    stream_close,
};

