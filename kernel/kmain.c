#include <stdarg.h>
#include <string.h>
#include <libutil.h>
#include "console.h"
#include "debug.h"
#include "driver.h"
#include "error.h"
#include "gdt.h"
#include "idt.h"
#include "kernel.h"
#include "multiboot.h"
#include "timer.h"
#include "proc.h"
#include "mm/mm.h"
#include "mm/virt.h"

int last_error; /* see error.h */

mb_info_t mb_info;

volatile int dummy;

#define DUMMY_WAIT 500000

static void task_A(void)
{
	while (true) {
		cur_console->putc('A');
		volatile int i=0;
		for (; i < DUMMY_WAIT; ++i)
			;
	}
}

static void task_B(void)
{
	while (true) {
		cur_console->putc('B');
		volatile int i=0;
		for (; i < DUMMY_WAIT; ++i)
			;
	}
}

#define verify_init(ok,what) \
	do { if ((ok) != OK) panic("Failed to initialize %s.", what); \
	else printk(KERN_INFO "Initialized %s.", what); } while(0)

void kmain(int boot_magic, mb_info_t *boot_info)
{
	memset(bss_start, 0, bss_end-bss_start); // .bss has to be 0 initialized

	mb_info = *boot_info;

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
	verify_init(init_timer(), "timer");
	verify_init(init_pm(), "process manager");
	verify_init(init_driver(), "driver");
	printk(KERN_NOTICE "kOS booted.");

	//enable_intr();
	//printk(KERN_NOTICE "0/0:");
	//dummy = 0/0;
	//return;

	struct proc *pA = proc_create(proc_idle, "A", task_A, NULL, false);
	struct proc *pB = proc_create(proc_idle, "B", task_B, NULL, false);
	proc_unblock(pA);
	proc_unblock(pB);

	enable_intr();
	asm volatile("int $0x20");

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

	printk(KERN_EMERG "** Kernel panic **\n%s", buffer);

	for (;;) {
		asm("cli; hlt;");
	}
}
