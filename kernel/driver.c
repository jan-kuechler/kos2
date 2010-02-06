#include <stdbool.h>
#include "bitop.h"
#include "driver.h"
#include "error.h"
#include "kernel.h"

static bool loaded[128];

static void loadmsg(int i)
{
	if (!loaded[i]) {
		printk(KERN_INFO "Loading driver: %s", drv_list[i].name);
		loaded[i] = true;
	}
}

static int call_init(int flag)
{
	int i=0;
	for (; i < drv_list_count; ++i) {
		if (bisset(drv_list[i].flags, flag)) {
			loadmsg(i);
			int err = drv_list[i].init(flag);
			if (err != OK)
				return err;
		}
	}
	return OK;
}

int init_driver(void)
{
	int err;
	int i;
	for (i=0; i < drv_list_count; ++i) {
		loaded[i] = false;
	}

	err = call_init(INIT_PREPARE);
	if (err) return err;

	err = call_init(INIT_FULL);
	if (err) return err;

	err = call_init(INIT_AFTER);
	if (err) return err;

	return OK;
}

