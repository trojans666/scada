#include <string.h>
#include <stdlib.h>

#include "appender.h"
#include "appender_type_stream.h"

#include "error.h"

#include "factory.h"
#include "hash.h"


struct __log_appender
{
  char*				app_name;
  const log_layout_t*		app_layout;
  const log_appender_type_t*	app_type;
  int					app_isopen;
  void*				app_udata;
};

sd_factory_t* log_appender_factory = NULL;

/**
* @bug log_appender_type hash is not freed in destructor
*/

/*******************************************************************************/
static sd_hash_t* log_appender_types(void)
{
  static sd_hash_t* types = NULL;
  
  if (!types)
    types = sd_hash_new(20, NULL);
  
  return types;
}

extern void log_appender_types_print(FILE *fp)
{
  sd_hash_iter_t* i;
  
  fprintf(fp, "appender types:");
  for (i = sd_hash_begin(log_appender_types());
    i != sd_hash_end(log_appender_types());
    i = sd_hash_iter_next(i) ) 
  {
    fprintf(fp, "'%s' ",((log_appender_type_t *)(i->data))->name );
  }
  fprintf(fp, "\n");
}

/*******************************************************************************/
extern const log_appender_type_t* log_appender_type_get(const char* a_name)
{
  sd_hash_iter_t* i;
  
  if (!a_name)
    return NULL;
  
  if ( (i = sd_hash_lookup(log_appender_types(), a_name)) != NULL)
    return i->data;
  
  return NULL;
}

/*******************************************************************************/
extern const log_appender_type_t* log_appender_type_set(
  const log_appender_type_t* a_type)
{
  sd_hash_iter_t* i = NULL;
  void* previous = NULL;
  
  if (!a_type)
    return NULL;

  if ( (i = sd_hash_lookadd(log_appender_types(), a_type->name)) == NULL)
    return NULL;

  previous = i->data;
  i->data  = (void*) a_type;

  return previous;
}

/*******************************************************************************/
extern log_appender_t* log_appender_get(const char* a_name)
{
  static const sd_factory_ops_t log_appender_factory_ops = {
    (void*) log_appender_new,
    (void*) log_appender_delete,
    (void*) log_appender_print,
  };
  
  if (!log_appender_factory) {
    log_appender_factory = sd_factory_new("log_appender_factory",
      &log_appender_factory_ops);
    
    /* build default appenders */
    log_appender_set_udata(log_appender_get("stderr"), stderr);
    log_appender_set_udata(log_appender_get("stdout"), stdout);
  }
  
  return sd_factory_get(log_appender_factory, a_name);
}

/*******************************************************************************/
extern log_appender_t* log_appender_new(const char* a_name)
{
  log_appender_t* this;
  
  if (!a_name)
    return NULL;
  
  this	     = calloc(1, sizeof(log_appender_t));
  this->app_name   = strdup(a_name);
  this->app_type   = &log_appender_type_stream;
  this->app_layout = log_layout_get("basic");
  this->app_isopen = 0;
  this->app_udata  = NULL;
  return this;
}

/*******************************************************************************/
extern void log_appender_delete(log_appender_t* this)
{
  sd_debug("log_appender_delete['%s'",
                      (this && this->app_name ? this->app_name: "(no name)"));
  if (!this){
    goto log_appender_delete_exit;
  }
  
  log_appender_close(this);
  if (this->app_name){
    free(this->app_name);  
  }
  free(this);
  
  log_appender_delete_exit:
  sd_debug("]");
}

/*******************************************************************************/
extern const char* log_appender_get_name(const log_appender_t* this)
{
  return (this ? this->app_name : "(nil)");
}

/*******************************************************************************/
extern const log_appender_type_t* log_appender_get_type(
  const log_appender_t* this)
{
  return (this ? this->app_type : NULL);
}

/*******************************************************************************/
extern const log_layout_t* log_appender_get_layout(const log_appender_t* this)
{
  return (this ? this->app_layout : NULL);
}

/*******************************************************************************/
extern void* log_appender_get_udata(const log_appender_t* this)
{
  return (this ? this->app_udata : NULL);
}

/*******************************************************************************/
extern const log_appender_type_t* log_appender_set_type(
  log_appender_t*			this,
  const log_appender_type_t*	a_type)
{
  const log_appender_type_t* previous;
  
  if (!this)
    return NULL;
  
  previous = this->app_type;
  this->app_type = a_type;
  return previous;
}

/*******************************************************************************/
extern const log_layout_t* log_appender_set_layout(
  log_appender_t*	this,
  const log_layout_t* a_layout)
{
  const log_layout_t* previous;
  
  if (!this)
    return NULL;
  
  previous = this->app_layout;
  this->app_layout = a_layout;
  return previous;
}

/*******************************************************************************/
extern void* log_appender_set_udata(log_appender_t* this, void* a_udata)
{
  void* previous;
  
  if (!this)
    return NULL;
  
  previous = this->app_udata;
  this->app_udata = a_udata;
  return previous;
}

/*******************************************************************************/
extern int log_appender_open(log_appender_t* this)
{
  int rc = 0;
  
  if (!this)
    return -1;
  
  if (this->app_isopen)
    return 0;
  
  if (!this->app_type)
    return 0;

  if (!this->app_type->open)
    return 0;

  if (this->app_type->open(this) == -1){
    rc = -1;
  }
  if (!rc) {
    this->app_isopen++;
  }

  return rc;
}

/**
* @bug is this the right place to open an appender ? 
*/

/*******************************************************************************/
extern int log_appender_append(
  log_appender_t*		this,
  log_logging_event_t*	a_event)
{
  if (!this)
    return -1;
  
  if (!this->app_type)
    return 0;
  
  if (!this->app_type->append)
    return 0;
  
  if (!this->app_isopen)
    if (log_appender_open(this) == -1)
     return -1;
	
    if ( (a_event->evt_rendered_msg = 
      log_layout_format(this->app_layout, a_event)) == NULL)
        a_event->evt_rendered_msg = a_event->evt_msg;

    return this->app_type->append(this, a_event);
}

/*******************************************************************************/
extern int log_appender_close(log_appender_t* this)
{
  if (!this)
    return -1;
  
  if (!this->app_isopen)
    return 0;
  
  if (!this->app_type)
    return 0;
  
  if (!this->app_type->close)
    return 0;
  
  if (this->app_type->close(this) == -1)
    return -1;
  
  this->app_isopen--;
  return 0;
}

/*******************************************************************************/
extern void log_appender_print(const log_appender_t* this, FILE* a_stream)
{
  if (!this) 
    return;
  
    fprintf(a_stream, "{ name:'%s' type:'%s' layout:'%s' isopen:%d udata:%p}",
	    this->app_name, 
	    this->app_type ? this->app_type->name : "(not set)",
        log_layout_get_name(this->app_layout),
	    this->app_isopen, 
	    this->app_udata);
}
