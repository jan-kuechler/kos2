#ifndef CPU_H
#define CPU_H

#include <stdint.h>

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

inline struct mm_context *cpu_get_context(void)
{
	/* TODO: dummy! */
	extern struct mm_context *kernel_context;
	return kernel_context;
}

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
