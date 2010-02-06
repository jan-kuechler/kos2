#include "bitop.h"
#include "cpu.h"
#include "error.h"
#include "gdt.h"
#include "idt.h"
#include "kernel.h"
#include "ports.h"
#include "proc.h"
#include "util/list.h"

#define PIC1      0x20
#define PIC2      0xA0

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

#define PIC_EOI   0x20

#define ICW1_ICW4	0x01		   /* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		 /* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		 /* Level triggered (edge) mode */
#define ICW1_INIT	0x10       /* Initialization - required! */

#define ICW4_8086	0x01		   /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		   /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08 /* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C /* Buffered mode/master */
#define ICW4_SFNM	0x10		   /* Special fully nested (not) */

#define GATE_PRESENT 0x80 /* 1000000b */
#define INTR_GATE    0xE  /* for 32 bit */
#define TRAP_GATE    0xF  /* for 32 bit */
#define TASK_GATE    0x5

struct idt_entry idt[IDT_SIZE] __attribute__((aligned(8)));

list_t irq_handler[NUM_IRQ];

/* assembler stubs exported by int.s */
extern void isr_null_handler(void);

extern void isr_stub_0(void);
extern void isr_stub_1(void);
extern void isr_stub_2(void);
extern void isr_stub_3(void);
extern void isr_stub_4(void);
extern void isr_stub_5(void);
extern void isr_stub_6(void);
extern void isr_stub_7(void);
extern void isr_stub_8(void);
extern void isr_stub_9(void);
extern void isr_stub_10(void);
extern void isr_stub_11(void);
extern void isr_stub_12(void);
extern void isr_stub_13(void);
extern void isr_stub_14(void);
extern void isr_stub_15(void);
extern void isr_stub_16(void);
extern void isr_stub_17(void);
extern void isr_stub_18(void);
extern void isr_stub_19(void);

extern void irq_stub_0(void);
extern void irq_stub_1(void);
extern void irq_stub_2(void);
extern void irq_stub_3(void);
extern void irq_stub_4(void);
extern void irq_stub_5(void);
extern void irq_stub_6(void);
extern void irq_stub_7(void);
extern void irq_stub_8(void);
extern void irq_stub_9(void);
extern void irq_stub_10(void);
extern void irq_stub_11(void);
extern void irq_stub_12(void);
extern void irq_stub_13(void);
extern void irq_stub_14(void);
extern void irq_stub_15(void);

extern void syscall_stub(void);

/* messages for (nearly) all exceptions */
static const char *fault_name[] = {
	"Devide Error",
	"Reserved",
	"Nonmaskable Interrupt",
	"Breakpoint",
	"Overflow",
	"Bound Range Exceeded",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Invalid TSS",
	"Segment Not Present",
	"Stack-Segment Fault",
	"General Protection Fault",
	"Page Fault",
	"Reserved",
	"x87 Floating-Point Exception",
	"Alignment Check",
	"Machine Check",
	"SIMD Floating-Point Exception",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
};

int idt_register_irq(int irq, irq_handler_t handler)
{
	if (irq < 0 || irq >= NUM_IRQ)
		return -E_INVALID;
	list_add_back(&irq_handler[irq], handler);
	return OK;
}

void idt_unregister_irq(int irq, irq_handler_t handler)
{
	if (irq < 0 || irq >= NUM_IRQ)
		return;

	list_entry_t *entry;
	list_iterate(entry, &irq_handler[irq]) {
		if (entry->data == handler) {
			list_del_entry(&irq_handler[irq], entry);
			return;
		}
	}
}

static void idt_handle_exception(uint32_t *esp)
{
	struct stack_frame *stack = (struct stack_frame*)*esp;

	printk(KERN_CRIT "kOS triggered an exception!");
	printk(KERN_CRIT "Exception: #%02d (%s)", stack->intr, fault_name[stack->intr]);
	cpu_print_stack(stack);
	panic("Cannot resume.");
}

static void idt_handle_irq(uint32_t *esp)
{
	struct stack_frame *stack = (struct stack_frame*)*esp;

	int irq = stack->intr - IRQ_BASE;

	if (irq == 7 || irq == 15) {
		// these irqs may be fake ones, test it
		uint8_t pic = (irq < 8) ? PIC1 : PIC2;
		outb(pic + 3, 0x03);
		if ((inb(pic) & 0x80) != 0) {
			goto irq_handeled;
		}
	}

	list_entry_t *entry;
	list_iterate(entry, &irq_handler[irq]) {
		irq_handler_t handler = entry->data;
		handler(irq, esp);
	}

irq_handeled:
	if (irq >= 8)
		outb(PIC2_CMD, PIC_EOI);
	outb(PIC1_CMD, PIC_EOI);
}

uint32_t idt_handle_int(uint32_t esp)
{
	struct stack_frame *stack = (struct stack_frame*)esp;

	sched_put(cpu_cur_proc());

	if (stack->intr < IRQ_BASE) {
		idt_handle_exception(&esp);
	}
	else if (stack->intr < SYSCALL) {
		idt_handle_irq(&esp);
	}
	else if (stack->intr == SYSCALL) {
		printk(KERN_WARNING "Syscall.");
	}
	else {
		panic("Unhandled interrupt: 0x%x", stack->intr);
	}

	struct proc *next = sched_get();
	if (!next) {
		panic("no proc to activate...");
	}
	cpu_set_proc(next, &esp);

	return esp;
}

int init_idt(void)
{
	int i = 0;

	for (; i < IDT_SIZE; ++i) {
		idt_set_gate(i, GDT_SEL_CODE, isr_null_handler, 0, IDT_INTR_GATE);
	}

	for (i=0; i < NUM_IRQ; ++i) {
		list_init(&irq_handler[i]);
	}

	// there should be some sort of compile time 'for' with macros )-:
	idt_set_gate( 0, GDT_SEL_CODE, isr_stub_0,  0, IDT_INTR_GATE);
	idt_set_gate( 1, GDT_SEL_CODE, isr_stub_1,  0, IDT_INTR_GATE);
	idt_set_gate( 2, GDT_SEL_CODE, isr_stub_2,  0, IDT_INTR_GATE);
	idt_set_gate( 3, GDT_SEL_CODE, isr_stub_3,  0, IDT_INTR_GATE);
	idt_set_gate( 4, GDT_SEL_CODE, isr_stub_4,  0, IDT_INTR_GATE);
	idt_set_gate( 5, GDT_SEL_CODE, isr_stub_5,  0, IDT_INTR_GATE);
	idt_set_gate( 6, GDT_SEL_CODE, isr_stub_6,  0, IDT_INTR_GATE);
	idt_set_gate( 7, GDT_SEL_CODE, isr_stub_7,  0, IDT_INTR_GATE);
	idt_set_gate( 8, GDT_SEL_CODE, isr_stub_8,  0, IDT_INTR_GATE);
	idt_set_gate( 9, GDT_SEL_CODE, isr_stub_9,  0, IDT_INTR_GATE);
	idt_set_gate(10, GDT_SEL_CODE, isr_stub_10, 0, IDT_INTR_GATE);
	idt_set_gate(11, GDT_SEL_CODE, isr_stub_11, 0, IDT_INTR_GATE);
	idt_set_gate(12, GDT_SEL_CODE, isr_stub_12, 0, IDT_INTR_GATE);
	idt_set_gate(13, GDT_SEL_CODE, isr_stub_13, 0, IDT_INTR_GATE);
	idt_set_gate(14, GDT_SEL_CODE, isr_stub_14, 0, IDT_INTR_GATE);
	idt_set_gate(15, GDT_SEL_CODE, isr_stub_15, 0, IDT_INTR_GATE);
	idt_set_gate(16, GDT_SEL_CODE, isr_stub_16, 0, IDT_INTR_GATE);
	idt_set_gate(17, GDT_SEL_CODE, isr_stub_17, 0, IDT_INTR_GATE);
	idt_set_gate(18, GDT_SEL_CODE, isr_stub_18, 0, IDT_INTR_GATE);
	idt_set_gate(19, GDT_SEL_CODE, isr_stub_19, 0, IDT_INTR_GATE);

	idt_set_gate(IRQ_BASE +  0, GDT_SEL_CODE, irq_stub_0,  0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE +  1, GDT_SEL_CODE, irq_stub_1,  0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE +  2, GDT_SEL_CODE, irq_stub_2,  0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE +  3, GDT_SEL_CODE, irq_stub_3,  0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE +  4, GDT_SEL_CODE, irq_stub_4,  0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE +  5, GDT_SEL_CODE, irq_stub_5,  0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE +  6, GDT_SEL_CODE, irq_stub_6,  0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE +  7, GDT_SEL_CODE, irq_stub_7,  0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE +  8, GDT_SEL_CODE, irq_stub_8,  0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE +  9, GDT_SEL_CODE, irq_stub_9,  0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE + 10, GDT_SEL_CODE, irq_stub_10, 0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE + 11, GDT_SEL_CODE, irq_stub_11, 0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE + 12, GDT_SEL_CODE, irq_stub_12, 0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE + 13, GDT_SEL_CODE, irq_stub_13, 0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE + 14, GDT_SEL_CODE, irq_stub_14, 0, IDT_INTR_GATE);
	idt_set_gate(IRQ_BASE + 15, GDT_SEL_CODE, irq_stub_15, 0, IDT_INTR_GATE);

	idt_set_gate(SYSCALL, GDT_SEL_CODE, syscall_stub, 3, IDT_INTR_GATE);

	/* PIC */
	printk(KERN_DEBUG "Initializing PIC\n");
	/* start initialization */
	outb_wait(PIC1_CMD, ICW1_INIT + ICW1_ICW4);
	outb_wait(PIC2_CMD, ICW1_INIT + ICW1_ICW4);
	/* define PIC vectors */
	outb_wait(PIC1_DATA, IRQ_BASE);
	outb_wait(PIC2_DATA, IRQ_BASE + 8);
	/* continue initialization */
	outb_wait(PIC1_DATA, 4);
	outb_wait(PIC2_DATA, 2);
	/* mode: 8086 */
	outb_wait(PIC1_DATA, ICW4_8086);
	outb_wait(PIC2_DATA, ICW4_8086);
	/* nothing masked */
	outb_wait(PIC1_DATA, 0x00);
	outb_wait(PIC2_DATA, 0x00);

	struct {
		uint16_t  size;
		uint32_t base;
	} __attribute__((packed)) idt_ptr = {
		.size = IDT_SIZE * 8 - 1,
		.base = (uint32_t)idt,
	};

	printk(KERN_DEBUG "Loading IDT\n");
	asm volatile("lidt %0" : : "m"(idt_ptr));

	disable_intr();

	return OK;
}

void idt_set_gate(int intr, uint16_t selector, void *handler,
                  uint8_t dpl, enum idt_gate_type type)
{
	uint8_t tbyte;
	switch (type) {
	case IDT_INTR_GATE: tbyte = INTR_GATE; break;
	case IDT_TRAP_GATE: tbyte = TRAP_GATE; break;
	case IDT_TASK_GATE: tbyte = TASK_GATE; break;
	default:
		panic("Unknown gate type %d", type);
		return; /* compiler warning about unitialized tbyte */
	}

	idt[intr].base_low  = bmask((uint32_t)handler, BMASK_WORD);
	idt[intr].selector  = selector;
	idt[intr].zero      = 0x00;
	idt[intr].type      = GATE_PRESENT | (bmask(dpl, BMASK_2BIT) << 5) |
	                      bmask(tbyte,BMASK_4BIT);
	idt[intr].base_high = bmask(((uint32_t)handler >> 16), BMASK_WORD);
}

#if 0

enum excpt_type
{
	FAULT, TRAP, ABORT, RSVD, BOTH,
};

static enum excpt_type exception_type[] =
{
	FAULT, BOTH, ABORT, TRAP,
	TRAP, FAULT, FAULT, FAULT,
	ABORT, FAULT, FAULT, FAULT,
	FAULT, FAULT, FAULT, RSVD,
	FAULT, FAULT, ABORT, FAULT,
	RSVD, RSVD, RSVD, RSVD,
	RSVD, RSVD, RSVD, RSVD,
	RSVD, RSVD, RSVD, RSVD,
};

static void mask_irq(uint8_t irq, bool disable)
{
	uint32_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
	uint8_t offs = irq < 8 ? 0 : 8;
	uint8_t val = inb(port);
	if (disable) {
		bsetn(val, irq - offs);
	}
	else {
		bclrn(val, irq - offs);
	}
	outb(port, val);
}

static void idt_handle_exception(dword *esp)
{
	regs_t *regs = (regs_t*)*esp;
	uint8_t ex = regs->intr;

	enum excpt_policy policy = EP_UNKNOWN;
	if (exception_handler[ex]) {
		policy = exception_handler[ex]((uint32_t*)esp);
	}
	else {
		policy = default_exception_handler((uint32_t*)esp);
	}

	if (policy == EP_DEFAULT) {
		policy = IS_USER(regs->ds) ? EP_ABORT : EP_PANIC;
	}

	enum excpt_type type = exception_type[regs->intr];

	switch (policy) {
	case EP_PANIC:
		panic("Critical exception!");
		break;

	case EP_ABORT:
		pm_destroy(cur_proc);
		break;

	case EP_RETRY: /* IP one step back and return */
		if (type == FAULT) {
			return;
		}
		panic("EP_RETRY is not implemented for non-faults...");
		break;

	case EP_GO_ON: /* IP to next instr and return */
		if (type == TRAP) {
			return;
		}
		panic("EP_GO_ON is not implemented for non-traps...");
		break;

	default:
		panic("This one is so fucked up, I even don't know how to go on...");
	}
}

static enum excpt_policy default_exception_handler(uint32_t *esp)
{
	regs_t *regs = (regs_t*)*esp;

	kout_select();

	if (IS_USER(regs->ds)) {
		dbg_error("%s triggered an exception.\n", cur_proc->cmdline);
	}
	else {
		dbg_error("kOS triggered an exception.\n");
	}

	dbg_error("Exception: #%02d (%s)\n",
	          regs->intr, fault_msg[regs->intr]);

	print_state(regs);

	dbg_error("\n");

	if (!IS_USER(regs->ds)) {
		char *sym = dbg_get_sym(regs->eip);
		if (!sym)
			sym = "<unknown>";
		dbg_error("Error at: %p %s\n", regs->eip, sym);
	}

	return EP_DEFAULT;
}

bool idt_set_irq_handler(uint8_t irq, irq_handler_t handler)
{
	kassert(irq < NUM_IRQ);

	if (irq_handlers[irq] != 0)
		return false;

	irq_handlers[irq] = handler;
	return true;
}

bool idt_clr_irq_handler(uint8_t irq)
{
	kassert(irq < NUM_IRQ);

	irq_handlers[irq] = 0;
	return true;
}

void idt_set_exception_handler(uint8_t intr, exception_handler_t handler)
{
	kassert(intr < IRQ_BASE);

	exception_handler[intr] = handler;
}

void idt_reset_irq_counter(uint8_t irq)
{
	kassert(irq < NUM_IRQ);
	irq_counter[irq] = 0;
}

bool idt_wait_irq(uint8_t irq, bool since_reset, uint32_t timeout)
{
	kassert(irq < NUM_IRQ);
	assert_allowed(A_DELAY_EXEC);

	uint32_t should = since_reset ? 1 : irq_counter[irq] + 1;
	uint64_t ticks = timeout ? timer_ticks + timeout : ULLONG_MAX;

	while (irq_counter[irq] < should && timer_ticks < ticks)
		;
	return irq_counter[irq] >= should;
}
#endif
