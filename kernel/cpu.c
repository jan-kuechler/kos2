#include "cpu.h"
#include "kernel.h"
#include "proc.h"

struct proc *current;

void cpu_set_proc(struct proc *proc, uint32_t *stackptr)
{
	struct proc *prev = current;
	if (current != proc) {
		current = proc;
		*stackptr = proc->kstack.virt;
	}
	if (!prev || prev->context != proc->context) {
		cpu_set_context(current->context);
	}
}

void cpu_set_context(struct mm_context *ctx)
{
	//TODO: dummy
}

void cpu_print_stack(struct stack_frame *stack)
{
	printk("Stack frame at %p", stack);
	printk("cs:eip: %06x:%p ss:esp: %06x:%p",
	       stack->cs, stack->eip, stack->u_ss, stack->u_esp);
	printk("error code: %016b cr2: %010x",
	       stack->errc, cpu_get_cr2());
	printk("eax: %p ebx: %p ecx: %p edx: %p",
	       stack->eax, stack->ebx, stack->ecx, stack->edx);
	printk("ebp: %p esp: %p esi: %p edi: %p",
	       stack->ebp, stack->esp, stack->esi, stack->edi);
	printk("eflags: %010x ds: %06x es: %06x fs: %06x gs: %06x",
	       stack->eflags, stack->ds, stack->es, stack->fs, stack->gs);
}

