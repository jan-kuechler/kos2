#ifndef CPU_H
#define CPU_H

#include <stdint.h>

struct proc;    /* See proc.h */
struct mm_context; /* See mm/types.h */

/** The stack frame layout after an interrupt */
struct stack_frame {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;

	uint32_t  ds;
	uint32_t  es;
	uint32_t  fs;
	uint32_t  gs;

	uint32_t intr;
	uint32_t errc;

	uint32_t eip;
	uint32_t  cs;
	uint32_t eflags;
	uint32_t u_esp;
	uint32_t u_ss;
} __attribute__((packed));

// TODO:
enum sysflags
{
		SF_TRAP   = 0x0100,
		SF_INTR,
		SF_TASK,
		SF_RESUME,
		SF_VM86,
		SF_ALIGN,
		SF_VIRTIF,
		SF_VIPEND,
		SF_CPUID,
};

#define PROC_INIT_FLAGS 0x0202

extern struct proc *current;

inline struct mm_context *cpu_get_context(void)
{
	/* TODO: dummy! */
	extern struct mm_context *kernel_context;
	return kernel_context;
}

void cpu_set_context(struct mm_context *c);

/**
 * Returns the current proces.
 * @return Process struct
 */
inline struct proc *cpu_cur_proc(void)
{
	return current;
}

/**
 * Sets the next active process
 * @param proc The process
 */
void cpu_set_proc(struct proc *proc, uint32_t *stackptr);

/**
 * Reads the CR2 status register
 * @return content of cr2
 */
inline uint32_t cpu_get_cr2(void)
{
	uint32_t val = 0;
	asm volatile("mov %%cr2, %0" : "=r"(val));
	return val;
}

/**
 * Prints the stack content.
 * @param stack Pointer to the stack
 */
void cpu_print_stack(struct stack_frame *stack);

#endif /*CPU_H*/
