#ifndef DRIVER_H
#define DRIVER_H

#include <compiler.h>

enum init_stage {
	INIT_PREPARE = 0x01,
	INIT_FULL   = 0x02,
	INIT_AFTER  = 0x04,
};

struct driver
{
	const char *name;
	int flags;
	int (*init)(int);
};

#define DECL_DRIVER(name,flags,init) \
	static char __drv_str[] __section(.init.data) = name; \
	static struct driver __drv \
	__section(.init.drv) __attribute__((used, aligned(1))) \
	= { __drv_str, flags, init};

#ifdef KERNEL

/**
 * Calls all driver init functions.
 * @return error code
 */
int init_driver(void);

linker_symbol(init_drv_start);
linker_symbol(init_drv_end);

#define drv_list_size  (init_drv_end - init_drv_start)
#define drv_list_count (drv_list_size / sizeof(struct driver))
#define drv_list       ((struct driver*)init_drv_start)

#endif // KERNEL

#endif /*DRIVER_H*/
