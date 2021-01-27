
#include <stdlib.h>

#include "logging_event.h"
#include "category.h"

/*******************************************************************************/
extern log_logging_event_t* log_logging_event_new(
    const char* a_category,
    int		a_priority,
    const char*	a_message)
{
    log_logging_event_t* evt;

    evt 		= calloc(1, sizeof(log_logging_event_t));
    evt->evt_category	= a_category;
    evt->evt_priority	= a_priority;
    evt->evt_msg	= a_message;
    
    gettimeofday(&evt->evt_timestamp, NULL);

    return evt;
}

/*******************************************************************************/
extern void log_logging_event_delete(log_logging_event_t* this)
{
    if (!this)
	return;

    free(this);
}

