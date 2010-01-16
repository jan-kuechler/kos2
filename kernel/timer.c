#include "bitop.h"
#include "idt.h"
#include "kernel.h"
#include "ports.h"
#include "timer.h"

#define TIMER_DATA 0x43
#define TIMER_CMD  0x40

#define TIMER_IRQ 0

volatile uint64_t timer_ticks; // timer ticks since system start

static void timer_irq(int irq, uint32_t *esp)
{
	timer_ticks++;
	//printk("Tick %d", (uint32_t)timer_ticks);
}

int init_timer(void)
{
	timer_ticks = 0;

	int div = TIMER_DEFAULT / TIMER_HZ;
	outb(TIMER_DATA, 0x36);
	outb(TIMER_CMD,  bmask(div, BMASK_BYTE));
	outb(TIMER_CMD,  div >> 8);

	return idt_register_irq(TIMER_IRQ, timer_irq);
}
