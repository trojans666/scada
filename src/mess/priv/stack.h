#ifndef __sd_stack_h
#define __sd_stack_h

/**
 * @file stack.h @ingroup sd
 *
 * @brief Generic stack object.
 *
 * @todo documentation
 * @todo API homogeneity with sd_list and sd_hash
 */

#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct __sd_stack sd_stack_t;

extern sd_stack_t* sd_stack_new(size_t max);
extern void	sd_stack_delete(sd_stack_t* astack, void (*free_data_fn)(void *));
extern size_t	sd_stack_get_nelem(const sd_stack_t* astack);

extern void	sd_stack_clear(sd_stack_t* astack, void (*free_data_fn)(void *));
extern int	sd_stack_push(sd_stack_t* astack, void *data);
extern void*	sd_stack_pop(sd_stack_t* astack);
extern void*	sd_stack_begin(sd_stack_t* astack);
extern void*	sd_stack_next(sd_stack_t* astack);
extern void*	sd_stack_end(sd_stack_t* astack);
extern void*	sd_stack_peek(sd_stack_t* astack);

#ifdef __cplusplus
}
#endif

#endif
