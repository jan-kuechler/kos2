#include <stdint.h>

#include <error.h>
#include <idt.h>
#include <kernel.h>
#include <driver.h>

#include "kbc.h"

static void intr_handler(int irq, uint32_t *esp)
{

}

static int init(int stage)
{
	printk(KERN_DEBUG "Installing keyboard interrupt handler...");

	idt_register_irq(KBC_IRQ, intr_handler);

	return OK;
}

DECL_DRIVER("keyboard", INIT_PREPARE, init);
