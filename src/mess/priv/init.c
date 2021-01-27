#include <stdlib.h>

#include "init.h"
#include "category.h"
#include "appender.h"
#include "layout.h"
#include "rollingpolicy.h"

#include "error.h"
#include "sprintf.h"
#include "factory.h"



#include "appender_type_stream.h"
#include "appender_type_stream2.h"
#include "appender_type_syslog.h"
#include "appender_type_mmap.h"
#include "appender_type_rollingfile.h"
#include "rollingpolicy_type_sizewin.h"
#include "layout_type_basic.h"
#include "layout_type_dated.h"
#include "layout_type_basic_r.h"
#include "layout_type_dated_r.h"



static const log_layout_type_t * const layout_types[] = {
    &log_layout_type_basic,
    &log_layout_type_dated,
    /* These appenders are not yet ported with msvc */
#ifndef _WIN32
    &log_layout_type_basic_r,
    &log_layout_type_dated_r
#endif
};
static size_t nlayout_types = sizeof(layout_types) / sizeof(layout_types[0]);

static const log_appender_type_t * const appender_types[] = {
    &log_appender_type_stream,
    &log_appender_type_stream2,
    &log_appender_type_mmap,
    &log_appender_type_syslog,
    &log_appender_type_rollingfile
};
static size_t nappender_types = sizeof(appender_types) / sizeof(appender_types[0]);


static const log_rollingpolicy_type_t * const rollingpolicy_types[] = {
    &log_rollingpolicy_type_sizewin
};
static size_t nrollingpolicy_types = 
    sizeof(rollingpolicy_types) / sizeof(rollingpolicy_types[0]);


/******************************************************************************/
extern int log_init(void)
{    
    size_t i;
    int ret = 0;
  
    sd_debug("log_init[");
  


  
    /* Initialize default types: layouts, appenders, rollingpolicies */
    sd_debug("intializing default types: appenders, layouts, rollingpolicies");
    for (i = 0; i < nlayout_types; i++)
    log_layout_type_set(layout_types[i]);
  
    for (i = 0; i < nappender_types; i++)
    log_appender_type_set(appender_types[i]);

    for (i = 0; i < nrollingpolicy_types; i++)
    log_rollingpolicy_type_set(rollingpolicy_types[i]);

    sd_debug("]");
    return ret;
}




/******************************************************************************/
extern int log_fini(void)
{
    int rc = 0;

    /* Some acceptable use of goto here to avoid lots of nested ifs
     * when we need a quick exit
     */
    sd_debug("log_fini[");

  

  

    
    sd_debug("cleaning up category, appender, layout and"
	     "rollingpolicy instances");
    if (log_category_factory) {
    sd_factory_delete(log_category_factory);
    log_category_factory = NULL;
    }
  
    if (log_appender_factory) {
    sd_factory_delete(log_appender_factory);
    log_appender_factory = NULL;
    }
  
    if (log_layout_factory) {
    sd_factory_delete(log_layout_factory);
    log_layout_factory = NULL;
    }
  
    if (log_rollingpolicy_factory) {
    sd_factory_delete(log_rollingpolicy_factory);
    log_rollingpolicy_factory = NULL;
    }
    

log_fini_exit:
    sd_debug("]");
  
    return 0;
}


/******************************************************************************/
#ifdef __GNUC__
extern void __attribute__ ((constructor)) __log_init(void)
{    
#ifdef WITH_CONSTRUCTORS
    log_init();
#endif
}

extern void __attribute__ ((destructor)) __log_fini(void)
{    
#ifdef WITH_CONSTRUCTORS
    log_fini();
#endif
}
#endif

/******************************************************************************/

extern void log_dump_all_types(FILE *fp){
    /*
     *    
     * For debug, dump all the types that have been registered during init. We just
     * display the name of the the type for the moment--the rest of the type info 
     * right now consists of functions to call, so not really printable.
     */
  
    log_appender_types_print(fp);
    log_layout_types_print(fp);
#ifdef WITH_ROLLINGFILE
    log_rollingpolicy_types_print(fp);
#endif
}

extern void log_dump_all_instances(FILE *fp){
  
    /*
     * Also dump any instances that were created during init by 
     * reading the conf file. 
     *
     * An instances of a type consists of the base
     * type information (name plus function table) and an instance name and
     * configuration.  For example one can have an instance of the rollingfile
     * appender which logs to /var/tmp and another instance which logs to 
     * /usr/tmp.  They are both of type rollingfile, but are distinct instances of
     * it
     */
    fprintf(fp, "instance dump follows (may be empty):\n");
    sd_factory_print(log_category_factory, fp);
    sd_factory_print(log_appender_factory, fp);
    sd_factory_print(log_layout_factory, fp);
#ifdef WITH_ROLLINGFILE
    sd_factory_print(log_rollingpolicy_factory, fp);
#endif
}
