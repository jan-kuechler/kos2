#include <stdarg.h>
#include <string.h>
#include <libutil.h>
#include "console.h"
#include "kernel.h"
#include "multiboot.h"
#include "mm/mm.h"

int last_error; /* see error.h */

mb_info_t mb_info;

void kmain(int boot_magic, mb_info_t *boot_info)
{
	memcpy(&mb_info, boot_info, sizeof(mb_info_t));

	init_bootcon();

	if (boot_magic != MB_BOOT_MAGIC) {
		printk(KERN_ERR "Bootloader not multiboot compatible. Could not load, sorry.");
		return;
	}

	printk(KERN_NOTICE "kOS booting...");
	init_mm();

	printk(KERN_NOTICE "kOS booted.");
	return;

	for (;;) {
		asm("sti; hlt;");
	}
}

void panic(const char *fmt, ...)
{
	char buffer[2048] = "";
	va_list args;
	va_start(args, fmt);
	strafmt(buffer, fmt, args);
	va_end(args);

	printk(KERN_EMERG "Kernel panik: %s", buffer);

	for (;;) {
		asm("cli; hlt;");
	}
}
