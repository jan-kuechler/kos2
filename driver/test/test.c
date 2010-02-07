#include "driver.h"
#include "kernel.h"

static int init(int flags)
{
	printk("Hello from the test driver!");
	return 0;
}

DECL_DRIVER("test", INIT_FULL, init);
