#ifndef KOS_CONFIG_H
#define KOS_CONFIG_H

/**
 *  CONF_DEBUG
 *
 * This controls debug builds
 */
#define CONF_DEBUG 1

/**
 *  CONF_SAFE_KMALLOC
 *
 * If this is defined a kmalloc return of NULL results in
 * a panic.
 */
#define CONF_SAFE_KMALLOC 1

#define CONF_SAFE_KMEM
#define CONF_SAFE_VMEM

#endif /*KOS_CONFIG_H*/
