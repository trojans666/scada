
#include <stdio.h>

#include "layout.h"
#include "priority.h"
#include "sprintf.h"

/*******************************************************************************/
static const char* basic_format(
    const log_layout_t*	  	a_layout,
    const log_logging_event_t*	a_event)
{
    static char buffer[1024];

    snprintf(buffer, sizeof(buffer), "%-8s %s - %s\n",
         log_priority_to_string(a_event->evt_priority),
	     a_event->evt_category, a_event->evt_msg);
    
    return buffer;
}

/*******************************************************************************/
const log_layout_type_t log_layout_type_basic = {
    "basic",
    basic_format,
};

