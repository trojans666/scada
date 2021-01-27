
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "layout.h"
#include "layout_type_basic.h"
#include "layout_type_dated.h"
#include "priority.h"
#include "hash.h"
#include "factory.h"

struct __log_layout
{
    char*			lo_name;
    const log_layout_type_t*	lo_type;
    void*			lo_udata;
};

sd_factory_t* log_layout_factory = NULL;

/**
 * @bug log_appender_type hash is not freed in destructor
 */

/*******************************************************************************/
static sd_hash_t* log_layout_types(void)
{
    static sd_hash_t* types = NULL;

    if (!types)
	types = sd_hash_new(20, NULL);
    
    return types;
}


extern void log_layout_types_print(FILE *fp)
{
   sd_hash_iter_t* i;
 
  fprintf(fp, "layout types:");
   for (i = sd_hash_begin(log_layout_types());
         i != sd_hash_end(log_layout_types());
	 i = sd_hash_iter_next(i) ) 
   {
      fprintf(fp, "'%s' ",((log_layout_type_t *)(i->data))->name );
   }
  fprintf(fp, "\n");
}

/*******************************************************************************/
extern const log_layout_type_t* log_layout_type_get(const char* a_name)
{
    sd_hash_iter_t* i;
    
    if (!a_name)
	return NULL;

    if ( (i = sd_hash_lookup(log_layout_types(), a_name)) != NULL)
	return i->data;

    return NULL;
}

/*******************************************************************************/
extern const log_layout_type_t* log_layout_type_set(
    const log_layout_type_t* a_type)
{
    sd_hash_iter_t* i = NULL;
    void* previous = NULL;

    if (!a_type)
	return NULL;

    if ( (i = sd_hash_lookadd(log_layout_types(), a_type->name)) == NULL)
	return NULL;

    previous = i->data;
    i->data  = (void*) a_type;
    
    return previous;
}
/*******************************************************************************/
extern log_layout_t* log_layout_get(const char* a_name)
{
    static const sd_factory_ops_t log_layout_factory_ops = {
    (void*) log_layout_new,
    (void*) log_layout_delete,
    (void*) log_layout_print,
    };

    if (!log_layout_factory) {
    log_layout_factory = sd_factory_new("log_layout_factory",
                          &log_layout_factory_ops);
	/* build default layouts */
    log_layout_set_type(log_layout_get("dated"), &log_layout_type_dated);
    log_layout_set_type(log_layout_get("basic"), &log_layout_type_basic);
    }

    return sd_factory_get(log_layout_factory, a_name);
}

/*******************************************************************************/
extern log_layout_t* log_layout_new(const char* a_name)
{
    log_layout_t* this;
    
    if (!a_name)
	return NULL;

    this	    = calloc(1, sizeof(log_layout_t));
    this->lo_name   = strdup(a_name);
    this->lo_type   = &log_layout_type_basic;
    this->lo_udata  = NULL;

    return this;
}

/*******************************************************************************/
extern void log_layout_delete(log_layout_t* this)
{
    if (!this)
	return;

    free(this->lo_name);
    free(this);
}

/*******************************************************************************/
extern const char* log_layout_get_name(const log_layout_t* this)
{
    return (this ? this->lo_name : NULL);
}

/*******************************************************************************/
extern const log_layout_type_t* log_layout_get_type(const log_layout_t* this)
{
    return (this ? this->lo_type : NULL);
}

/*******************************************************************************/
extern const log_layout_type_t* log_layout_set_type(
    log_layout_t*		this,
    const log_layout_type_t*	a_type)
{
    const log_layout_type_t* previous;

    if (!this)
	return NULL;

    previous = this->lo_type;
    this->lo_type = a_type;
    return previous;
}

/*******************************************************************************/
extern void* log_layout_get_udata(const log_layout_t* this)
{
    return (this ? this->lo_udata : NULL);
}

/*******************************************************************************/
extern void* log_layout_set_udata(log_layout_t* this, void* a_udata)
{
    void* previous;

    if (!this)
	return NULL;

    previous = this->lo_udata;
    this->lo_udata = a_udata;
    return previous;
}

/*******************************************************************************/
extern const char* log_layout_format(
    const log_layout_t*		this,
    const log_logging_event_t*a_event)
{
    if (!this)
	return NULL;
    
    if (!this->lo_type)
	return NULL;

    if (!this->lo_type->format)
	return NULL;

    return this->lo_type->format(this, a_event);
}

/*******************************************************************************/
extern void log_layout_print(const log_layout_t* this, FILE* a_stream)
{
    if (!this) 
	return;
    
    fprintf(a_stream, "{ name:'%s' type:'%s' udata:%p }", 
	    this->lo_name, 
	    this->lo_type ? this->lo_type->name : "(no set)", 
	    this->lo_udata);
}
