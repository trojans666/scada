
#include <stdio.h>

#include "layout.h"
#include "priority.h"
#include "sprintf.h"

/*******************************************************************************/
static const char* basic_r_format(
    const log_layout_t*	  	a_layout,
    const log_logging_event_t*	a_event)
{
    int n, i;

    n = snprintf(a_event->evt_buffer.buf_data, a_event->evt_buffer.buf_size,
		 "%-8s %s - %s\n",
         log_priority_to_string(a_event->evt_priority),
		 a_event->evt_category, a_event->evt_msg);

    if (n >= a_event->evt_buffer.buf_size) {
	/*
	 * append '...' at the end of the message to show it was
	 * trimmed
	 */
	for (i = 0; i < 3; i++)
	    a_event->evt_buffer.buf_data[a_event->evt_buffer.buf_size - 4 + i] = '.';
    }

    return a_event->evt_buffer.buf_data;
}

/*******************************************************************************/
const log_layout_type_t log_layout_type_basic_r = {
    "basic_r",
    basic_r_format,
};

