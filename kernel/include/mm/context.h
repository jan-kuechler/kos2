#ifndef MM_CONTEXT_H
#define MM_CONTEXT_H

struct mm_context;

/**
 * Creates and initalizes a new context.
 * @return The context
 */
struct mm_context *mc_create(void);

/**
 * Destroys a context.
 * @param c The context
 */
void mc_destroy(struct mm_context *c);

#endif /*MM_CONTEXT_H*/
