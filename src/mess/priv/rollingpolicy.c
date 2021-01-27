
#include <string.h>

#include "appender.h"
#include "appender_type_rollingfile.h"
#include "rollingpolicy.h"
#include "rollingpolicy_type_sizewin.h"
#include "factory.h"
#include "hash.h"
#include "factory.h"
#include "error.h"

/* Internal struct that defines the conf and the state info
* for an instance of the appender_type_fileroller type.
*/    
struct __log_rollingpolicy
{
  char*				        policy_name;
  const log_rollingpolicy_type_t*     policy_type;
  void *                                policy_udata;
  rollingfile_udata_t*                  policy_rfudatap;
  #define PFLAGS_IS_INITIALIZED 0x0001
  int					policy_flags;
};

sd_factory_t* log_rollingpolicy_factory = NULL;

/*******************************************************************************/
static sd_hash_t* log_rollingpolicy_types(void){
  static sd_hash_t* types = NULL;
  
  if (!types)
    types = sd_hash_new(20, NULL);
  
  return types;
}

extern void log_rollingpolicy_types_print(FILE *fp)
{
  sd_hash_iter_t* i;
  
  fprintf(fp, "rollingpolicy types:");
  for (i = sd_hash_begin(log_rollingpolicy_types());
    i != sd_hash_end(log_rollingpolicy_types());
    i = sd_hash_iter_next(i) ) 
  {
    fprintf(fp, "'%s' ",((log_rollingpolicy_type_t *)(i->data))->name );
  }
  fprintf(fp, "\n");
}

/*******************************************************************************/
log_rollingpolicy_t* log_rollingpolicy_get(const char* a_name)
{
  static const sd_factory_ops_t log_rollingpolicy_factory_ops = {
    (void*) log_rollingpolicy_new,
    (void*) log_rollingpolicy_delete,
    (void*) log_rollingpolicy_print,
  };
  
  if (!log_rollingpolicy_factory) {
    log_rollingpolicy_factory =
    sd_factory_new("log_rollingpolicy_factory",
      &log_rollingpolicy_factory_ops);
    
    /* build default rollingpolicy 
    log_rollingpolicy_set_udata(log_appender_get("stderr"), stderr);
    log_appender_set_udata(log_appender_get("stdout"), stdout);*/
  }
  
  return sd_factory_get(log_rollingpolicy_factory, a_name);
}

/****************************************************************************/
log_rollingpolicy_t* log_rollingpolicy_new(const char* a_name){
  log_rollingpolicy_t* this;
  
  sd_debug("log_rollingpolicy_new[ ");
  if (!a_name)
    return NULL;
  sd_debug("new policy name='%s'", a_name);
  this	          = calloc(1, sizeof(log_rollingpolicy_t));
  this->policy_name     = strdup(a_name);
  this->policy_type     = &log_rollingpolicy_type_sizewin;
  this->policy_udata    = NULL;
  this->policy_rfudatap  = NULL;
  this->policy_flags = 0; 
  
  sd_debug("]");
  
  return this;
}

/*******************************************************************************/
void log_rollingpolicy_delete(log_rollingpolicy_t* this)
{
  
  sd_debug("log_rollingpolicy_delete['%s'",
    (this && this->policy_name ? this->policy_name: "(no name)"));
  if (!this){
    goto log_rollingpolicy_delete_exit;
  }
  
  if (log_rollingpolicy_fini(this)){
    sd_error("failed to fini rollingpolicy");
    goto log_rollingpolicy_delete_exit;
  }
 
  if (this->policy_name){
    sd_debug("freeing policy name");
    free(this->policy_name);
    this->policy_name = NULL;
  };
  sd_debug("freeing this rolling policy instance");
  free(this);

log_rollingpolicy_delete_exit:
  sd_debug("]");
}

/*******************************************************************************/

int log_rollingpolicy_init(log_rollingpolicy_t *this, rollingfile_udata_t* rfup){
  
  int rc = 0;
  
  if (!this)
    return -1;
  
  this->policy_rfudatap = rfup;
  
  if (!this->policy_type)
    return 0;
  
  if (!this->policy_type->init)
    return 0;
  
  rc = this->policy_type->init(this, rfup);
  
  this->policy_flags |= PFLAGS_IS_INITIALIZED;
  
  return rc;  
  
}

int log_rollingpolicy_fini(log_rollingpolicy_t *this){
  
  int rc = 0;
  
  sd_debug("log_rollingpolicy_fini['%s'",
    (this && this->policy_name ? this->policy_name: "(no name")) ;
  
  if (!this){
    rc = 0;
  } else {
    if (this->policy_flags & PFLAGS_IS_INITIALIZED){
      if (this->policy_type){
        rc = this->policy_type->fini(this);
      }
    }
    
    if (!rc){
      this->policy_flags &= ~PFLAGS_IS_INITIALIZED;
    }else{
      sd_debug("Call to rollingpolicy fini failed");
    }
  }
  
  sd_debug("]");
  return rc;  
}

/*******************************************************************************/

int log_rollingpolicy_is_triggering_event(log_rollingpolicy_t* this, const log_logging_event_t* a_event, long current_fs){
  if (!this)
    return -1;
  
  if (!this->policy_type)
    return 0;
  
  if (!this->policy_type->is_triggering_event)
    return 0;
  
  return this->policy_type->is_triggering_event(this, a_event, current_fs);  
}
/*******************************************************************************/

int log_rollingpolicy_rollover(log_rollingpolicy_t* this, FILE **fpp){
  
  if (!this)
    return -1;
  
  if (!this->policy_type)
    return 0;
  
  if (!this->policy_type->rollover)
    return 0;
  
  return this->policy_type->rollover(this, fpp);
}
/*******************************************************************************/

void* log_rollingpolicy_get_udata(const log_rollingpolicy_t* this){
  return (this ? this->policy_udata : NULL);
}
/*******************************************************************************/

void* log_rollingpolicy_get_name(const log_rollingpolicy_t* this){
  return (this ? this->policy_name : NULL);
}
/*******************************************************************************/

rollingfile_udata_t* log_rollingpolicy_get_rfudata(const log_rollingpolicy_t* this){
  return (this ? this->policy_rfudatap : NULL);
}

/*******************************************************************************/

const log_rollingpolicy_type_t* log_rollingpolicy_type_set( const log_rollingpolicy_type_t* a_type){
  
  sd_hash_iter_t* i = NULL;
  void* previous = NULL;
  
  if (!a_type)
    return NULL;
  
  if ( (i = sd_hash_lookadd(log_rollingpolicy_types(), a_type->name)) == NULL)
    return NULL;
  
  previous = i->data;
  i->data  = (void*) a_type;
  
  return previous;
}
/*****************************************************************************/
const log_rollingpolicy_type_t* log_rollingpolicy_type_get( const char* a_name){
  sd_hash_iter_t* i;
  
  if (!a_name)
    return NULL;
  
  if ( (i = sd_hash_lookup(log_rollingpolicy_types(), a_name)) != NULL)
    return i->data;
  
  return NULL;
}
/*******************************************************************************/

void log_rollingpolicy_set_udata(log_rollingpolicy_t* this, void *udatap){
  if ( this) {
    this->policy_udata = udatap;
  }
}

/*******************************************************************************/
const log_rollingpolicy_type_t* log_rollingpolicy_set_type( log_rollingpolicy_t* a_rollingpolicy, const log_rollingpolicy_type_t* a_type){
  
  const log_rollingpolicy_type_t* previous;
  
  if (!a_rollingpolicy)
    return NULL;
  
  previous = a_rollingpolicy->policy_type;
  a_rollingpolicy->policy_type = a_type;
  return previous;
}

/*******************************************************************************/
void log_rollingpolicy_print(const log_rollingpolicy_t* this, FILE* a_stream)
{
  if (!this) 
    return;
  
    fprintf(a_stream, "{ name:'%s' udata:%p}",
	    this->policy_name, 	     
	    this->policy_udata);
}


int log_rollingpolicy_is_initialized(log_rollingpolicy_t* this){
  
	if (!this) 
		return(0);	
  
	return( this->policy_flags & PFLAGS_IS_INITIALIZED);
  
}
