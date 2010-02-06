#ifndef KERNEL_H
#define KERNEL_H

#include <compiler.h>
#include <kos/config.h>
#include "multiboot.h"

#define LOGLVL_SILENT  -1
#define LOGLVL_DEFAULT  6

#define KERN_EMERG      "<0>"   /* system is unusable                   */
#define KERN_ALERT      "<1>"   /* action must be taken immediately     */
#define KERN_CRIT       "<2>"   /* critical conditions                  */
#define KERN_ERR        "<3>"   /* error conditions                     */
#define KERN_WARNING    "<4>"   /* warning conditions                   */
#define KERN_NOTICE     "<5>"   /* normal but significant condition     */
#define KERN_INFO       "<6>"   /* informational                        */
#define KERN_DEBUG      "<7>"   /* debug-level messages                 */

#define KERN_DEFAULT    "<d>"

#ifdef CONF_DEBUG
#define kassert(expr)\
	do {\
		if (!(expr))\
			panic("Assertion failed: '%s' %s:%s:%d", #expr, __func__,__FILE__,__LINE__);\
	}while(0)
#else
#define kassert(...)
#endif

extern mb_info_t mb_info;

linker_symbol(kernel_start);
linker_symbol(kernel_end);
linker_symbol(kernel_size);
linker_symbol(bss_start);
linker_symbol(bss_end);

/**
 * Prints a kernel message.
 * @param fmt Format string
 * @param ... Format arguments
 * @return The number of characters written
 */
int printk(const char *fmt, ...);

/**
 * Issues an emergency halt of the system.
 * @param fmt Format string
 * @param ... Format arguments
 */
void panic(const char *fmt, ...);

/** Type for panic handlers */
typedef void (*panic_handler_t)(void);

/**
 * Adds a function to be called on panic.
 * @param handler The function to call
 */
void add_panic_handler(panic_handler_t handler);

#endif /*KERNEL_H*/
