#ifndef log_rollingpolicy_h
#define log_rollingpolicy_h

/**
 * @file rollingpolicy.h
 *
 * @brief log rolling policy interface. Defines the interface for
 * managing and providing rolling policies.
 *
 * A rolling policy is used to confogure a rollingfile appender to tell
 * it when to trigger a rolover event.
*/ 

#include <stdio.h>

#include "layout.h"

#ifdef __cplusplus
extern "C" {
#endif

struct __log_rollingpolicy;

/**
 * log rollingpolicy type
 */
typedef struct __log_rollingpolicy log_rollingpolicy_t;


#define ROLLINGFILE_DEFAULT_LOG_DIR "."
#define ROLLINGFILE_DEFAULT_LOG_PREFIX "log"

typedef struct __rollingfile_udata rollingfile_udata_t; /* opaque */


/**
 * @brief log rollingpolicy type.  Defines the interface a specific policy
 * must provide to the rollingfile appender.
 *
 * Attributes description:
 * 
 * @li @c name rollingpolicy type name 
 * @li @c init() init the rollingpolicy
 * @li @c is_triggering_event()
 * @li @c rollover()
 *
 **/
typedef struct log_rollingpolicy_type {
  const char*	name;
  int (*init)(log_rollingpolicy_t *a_this, rollingfile_udata_t* rfudatap );
  int (*is_triggering_event)( log_rollingpolicy_t* a_policy,
                  const log_logging_event_t*,
			      long current_file_size );
  int (*rollover)(log_rollingpolicy_t* a_policy, FILE **);
  int (*fini)(log_rollingpolicy_t *a_this);
} log_rollingpolicy_type_t;

/**
 * Get a new rolling policy
 * @param policy_name a name for the policy
 * @return a new rolling policy, otherwise NULL.
 */ 
log_rollingpolicy_t* log_rollingpolicy_get(
                                  const char* policy_name);

/**
 * Use this function to register a rollingpolicy type with log.
 * Once this is done you may refer to this type by name both 
 * programmatically and in the log configuration file.
 *
 * @param a_type a pointer to the new rollingpolicy type to register.
 * @returns a pointer to the previous rollingpolicy type of same name.
 *
 * Example code fragment: 
 * @code
 * 
 * const log_rollingpolicy_type_t log_rollingpolicy_type_sizewin = {
 *   "sizewin",
 *   sizewin_init,
 *   sizewin_is_triggering_event,
 *   sizewin_rollover
 * };
 *
 * log_rollingpolicy_type_set(&log_rollingpolicy_type_sizewin);
 * @endcode
 * 
 */
const log_rollingpolicy_type_t* log_rollingpolicy_type_set(
                                    const log_rollingpolicy_type_t* a_type);
                                  
/**
 * Configure a rolling policy with a specific policy.
 * @param policyp pointer to the rolling policy
 * @param udatap a specific policy type, for example sizewin.
 * @return zero if successful, non-zero otherwise.
 */                                  
void log_rollingpolicy_set_udata(log_rollingpolicy_t* policyp,
					  void *udatap);
/**
 * Call the initialization code of a rolling policy.
 * @param policyp pointer to the rolling policy
 * @param app the rolling appender this policy is used with
 * @return zero if successful, non-zero otherwise.
*/
int log_rollingpolicy_init(log_rollingpolicy_t *policyp,
                                       rollingfile_udata_t* rfup );

/**
 * Call the un initialization code of a rolling policy.
 * This will call the fini routine of the particular rollingpolicy type
 * to allow it to free up resources.  If the call to fini in the 
 * rollingpolicy type fails then the rollingpolicy is not uninitialized.
 * Try again later model...
 * @param policyp pointer to the rolling policy
 * @return zero if successful, non-zero otherwise.
*/
int log_rollingpolicy_fini(log_rollingpolicy_t *a_this);

/**
 * Determine if a logging event should trigger a rollover according to
 * the given policy.
 * @param policyp pointer to the rolling policy
 * @param evtp the logging event pointer.
 * @param current_file_size the size of the current file being logged to.
 * @return non-zero if rollover required, zero otherwise.
 */ 
int log_rollingpolicy_is_triggering_event(
             log_rollingpolicy_t* policyp,
                     const log_logging_event_t* evtp,
		     long current_file_size );
/**
 * Effect a rollover according to policyp on the given file stream.
 * @param policyp pointer to the rolling policy
 * @param fp filestream to rollover.
 * @return zero if successful, non-zero otherwise.
 * The policy can return an indication that something went wrong but
 * that the rollingfile appender can stull go ahead and log by returning an
 * error code <= ROLLINGPOLICY_ROLLOVER_ERR_CAN_LOG.  Anything greater than
 * means that the rolling file appender will not try to log it's message.
 */        

#define  ROLLINGPOLICY_ROLLOVER_ERR_CAN_LOG 0x05
int log_rollingpolicy_rollover(log_rollingpolicy_t* policyp,
                                            FILE ** fp);

/**
 * sets the rolling policy type
 *
 * @param a_rollingpolicy the log_rollingpolicy_t object
 * @param a_type the new rollingpolicy type
 * @return the previous appender type
 **/
const log_rollingpolicy_type_t* log_rollingpolicy_set_type(
    log_rollingpolicy_t* a_rollingpolicy,
    const log_rollingpolicy_type_t* a_type);
    
/**
 * Get a pointer to an existing rollingpolicy type.
 *
 * @param a_name the name of the rollingpolicy type to return.  
 * @returns a pointer to an existing rollingpolicy type, or NULL if no 
 * rollingpolicy type with the specified name exists.
 */
const log_rollingpolicy_type_t* log_rollingpolicy_type_get(
    const char* a_name);
                                            
/**
 * Get the rolling policy configuration.
 * @param policyp pointer to the rolling policy
 * @return pointer to the rolling policy configuration.
*/                                             
void* log_rollingpolicy_get_udata(
                        const log_rollingpolicy_t* policyp);
                        
/**
 * Get the rollingfile appender associated with this policy.
 * @param policyp pointer to the rolling policy
 * @return pointer to the rolling file appender associated with this policy
*/                          
rollingfile_udata_t* log_rollingpolicy_get_rfudata(
                        const log_rollingpolicy_t* policyp);
                        
void* log_rollingpolicy_get_name(const log_rollingpolicy_t* a_this);

log_rollingpolicy_t* log_rollingpolicy_new(const char* a_name);
void log_rollingpolicy_delete(log_rollingpolicy_t* a_this);
void log_rollingpolicy_print(const log_rollingpolicy_t* a_this,
FILE* a_stream);

int log_rollingpolicy_is_initialized(log_rollingpolicy_t* a_this);
void log_rollingpolicy_types_print(FILE *fp);
/**
 * @internal
 **/
struct __sd_factory;
struct __sd_factory* log_rollingpolicy_factory;

#ifdef __cplusplus
}
#endif


#endif
