#ifndef IDT_H
#define IDT_H

#include <stdbool.h>
#include <stdint.h>

/* IMPORTANT: Change int.s if you change the following values */

/** The first IRQ */
#define IRQ_BASE 0x20

/** Syscall interrupt number */
#define SYSCALL  0x30

/** Number of IRQs */
#define NUM_IRQ  0x10

/** Number of interrupts total */
#define IDT_SIZE 256

/** An IDT entry */
struct idt_entry {
	uint16_t base_low;
	uint16_t selector;
	uint8_t  zero;
	uint8_t  type;
	uint16_t base_high;
} __attribute__((packed));

/** Type of an interrupt gate */
enum idt_gate_type {
	IDT_INTR_GATE, /**< Interrupts are deactivated in the handler */
	IDT_TRAP_GATE, /**< Interrupts are activated in the handler */
	IDT_TASK_GATE, /**< Hardware taskswitch */
};

/**
 * Initializes the interrupt descriptor table.
 * @return error code
 */
int init_idt(void);

/**
 * Sets an gate in the IDT.
 * @param intr     The interrupt number
 * @param selector The gdt selector
 * @param handler  Pointer to the handler function
 * @param dpl      Descriptor privilege level
 * @param type     The gate type
 */
void idt_set_gate(int intr, uint16_t selector, void *handler,
                  uint8_t dpl, enum idt_gate_type type);

/**
 * Enables interrupts.
 */
inline void enable_intr(void)
{
		asm volatile("sti");
}

/**
 * Disables interrupts.
 */
inline void disable_intr(void)
{
	asm volatile("cli");
}

#if 0
enum excpt_policy
{
	EP_DEFAULT,
	EP_PANIC,
	EP_ABORT,
	EP_RETRY,
	EP_GO_ON,
	EP_UNKNOWN,
};

typedef void (*irq_handler_t)(int, dword*);
typedef enum excpt_policy (*exception_handler_t)(uint32_t*);

bool idt_set_irq_handler(uint8_t irq, irq_handler_t handler);
bool idt_clr_irq_handler(uint8_t irq);

void idt_set_exception_handler(uint8_t intr, exception_handler_t handler);

void idt_reset_irq_counter(uint8_t irq);
bool idt_wait_irq(uint8_t irq, bool since_reset, uint32_t timeout);
#endif

#endif /*IDT_H*/
