#include "kernel.h"
#include "multiboot.h"
#include "mm/mm.h"

int last_error; /* see error.h */

mb_info_t mb_info;

void kmain(int boot_magic, mb_info_t *boot_info)
{
	if (boot_magic != MB_BOOT_MAGIC) {
		printk(KERN_ERR "Bootloader not multiboot compatible. Could not load, sorry.");
		goto mainloop;
	}

	init_mm();

	printk(KERN_NOTICE "kOS booted.");

mainloop:
	for (;;) {
		asm("sti; hlt;");
	}
}

void panic(const char *fmt, ...)
{
	for (;;);
}
