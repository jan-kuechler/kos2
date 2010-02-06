#ifndef PROC_H
#define PROC_H

#include <stdbool.h>
#include <stdint.h>
#include "mm/types.h"

enum {
	PROC_TICKS = 5,
};

/** Process id type */
typedef uint32_t pid_t;

/** Process status */
enum proc_status
{
	PROC_READY,   /**< Process is ready, but does not run */
	PROC_BLOCKED, /**< Process is blocked */
	PROC_RUNNING, /**< Process runs at the moment */
};

/** Process information */
struct proc
{
	pid_t pid;
	struct proc *parent;
	enum proc_status status;

	int priority;
	int ticks;

	char *cmdline;

	struct mm_context *context;

	struct addrpair kstack;
	struct addrpair ustack;
};

extern struct proc *proc_idle;

/**
 * Initializes the process manager
 * @return error code
 */
int init_pm(void);

/**
 * Creates a new process
 * @param parent   The parent process
 * @param cmdline  Commandline for the process
 * @param entry    Address of the process' entry
 * @param context  The memory context for the process, or NULL if a new one should be created
 * @param usermode True, if the process shall run in usermode
 * @return Process struct
 */
struct proc *proc_create(struct proc *parent, const char *cmdline, uintptr_t entry,
                         struct mm_context *context, bool usermode);

/**
 * Destroys a process.
 * @param proc The process to destroy
 */
void  proc_destroy(struct proc *proc);

/**
 * Blocks a process.
 * @param proc The process
 * @return success
 */
bool proc_block(struct proc *proc);

/**
 * Unblocks a process
 * @param proc The process
 * @return success
 */
bool proc_unblock(struct proc *proc);

/**
 * Adds a new process to the schedulers list
 * @param proc The process
 */
void sched_add(struct proc *proc);

/**
 * Removes a process from the scheduler list
 * @param proc The process
 */
void sched_remove(struct proc *proc);

/**
 * Should be called frequently...
 */
void sched_update(void);

/**
 * Puts a process at the beginning of an int. handler
 * @param proc The process
 */
void sched_put(struct proc *proc);

/**
 * Gets the next process to start
 */
struct proc *sched_get(void);

#endif /*PROC_H*/
