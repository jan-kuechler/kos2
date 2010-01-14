#include <stdarg.h>
#include <string.h>
#include <libutil.h>
#include "console.h"
#include "error.h"
#include "gdt.h"
#include "idt.h"
#include "kernel.h"
#include "multiboot.h"
#include "mm/mm.h"
#include "mm/virt.h"

int last_error; /* see error.h */

mb_info_t mb_info;

volatile int dummy;

#define verify_init(ok,what) \
	do { if ((ok) != OK) panic("Failed to initialize %s.", what); \
	else printk(KERN_INFO "Initialized %s.", what); } while(0)

void kmain(int boot_magic, mb_info_t *boot_info)
{
	memcpy(&mb_info, boot_info, sizeof(mb_info_t));

	if (init_bootcon() != OK) {
		// There is no way to print anything )-:
		// Beep??
		return;
	}

	if (boot_magic != MB_BOOT_MAGIC) {
		printk(KERN_ERR "Bootloader not multiboot compatible. Could not load, sorry.");
		return;
	}

	printk(KERN_NOTICE "kOS booting...");
	verify_init(init_gdt(), "GDT");
	verify_init(init_idt(), "IDT");
	verify_init(init_mm(), "memory manager");
	verify_init(init_vmem(), "virtual memory");
	printk(KERN_NOTICE "kOS booted.");

	//enable_intr();
	//printk(KERN_NOTICE "0/0:");
	//dummy = 0/0;
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

	printk(KERN_EMERG "Kernel panic: %s", buffer);

	for (;;) {
		asm("cli; hlt;");
	}
}
