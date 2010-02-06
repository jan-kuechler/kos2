#ifndef MM_VIRT_H
#define MM_VIRT_H

#include "cpu.h"
#include "mm/flags.h"
#include "mm/page.h"
#include "mm/types.h"

/* Kernel memory from 0-512mb */
#define KERNEL_MEM_START ((vaddr_t)0x00000000)
#define KERNEL_MEM_END   ((vaddr_t)0x20000000)
/* Shared user memory from 512-1024mb */
#define SHARED_MEM_START ((vaddr_t)0x20000000)
#define SHARED_MEM_END   ((vaddr_t)0x40000000)
/* User memory from 1-4GB */
#define USER_MEM_START   ((vaddr_t)0x40000000)
#define USER_MEM_END     ((vaddr_t)0xFFFFFFFF)

#define USER_STACK_ADDR  USER_MEM_END

#define NUM_KERNEL_PTABS  ((((uintptr_t)KERNEL_MEM_END - (uintptr_t)KERNEL_MEM_START) / PTAB_MEM) - 1)

#define PTAB_BASE_INDEX  0x7F
#define PTAB_BASE_ADDR   0x1FC00000

/** The kernel address space */
extern struct mm_context *kernel_context;
/** The page tables for the kernel address space */
extern ptab_t kernel_ptabs[NUM_KERNEL_PTABS];

/**
 * Initializes the virtual memory manager.
 * @return error code
 */
int init_vmem(void);

/**
 * Maps memory pages in a context.
 * @param context The context
 * @param vaddr   Starting vaddr of the pages
 * @param paddr   Starting paddr of the pages
 * @param flags   Flags for the mapping
 * @param num     Number of pages to map
 * @return error code
 */
int vm_map(struct mm_context *context, vaddr_t vaddr, paddr_t paddr,
           int flags, size_t num);

/**
 * Unmaps memory pages in a context.
 * @param context The context
 * @param vaddr   Starting vaddr of the pages
 * @param num     Number of pages to unmap
 * @return error code
 */
int vm_unmap(struct mm_context *context, vaddr_t vaddr, size_t num);

/**
 * Finds a free address betweeen vstart and vend and maps length bytes
 * from paddr there.
 * @param context The context
 * @param paddr   Starting paddr of the memory
 * @param length  Length in bytes of the memory
 * @param vstart  Start of the virtual range
 * @param vend    End of the virtual range
 * @param flags   Flags for the mapping
 * @return The virtual address of the memory or NULL as an error code
 */
vaddr_t vm_alloc_addr(struct mm_context *context, paddr_t paddr, size_t length,
                      vaddr_t vstart, vaddr_t vend, int flags);

/**
 * Unmaps a memory area mapped with vm_alloc_addr.
 * @param context The context
 * @param vaddr   Virtual address of the memory area
 * @param length  Length of the memory in bytes
 * @return error code
 */
int vm_free_addr(struct mm_context *context, vaddr_t vaddr, size_t length);

/**
 * Shortcut to allocate a kernel space address.
 * @param paddr  Starting paddr of the memory
 * @param length Length of the memory in bytes
 * @return Virtual address of the mapped memory.
 */
inline vaddr_t vm_alloc_kernel_addr(paddr_t paddr, size_t length)
{
	return vm_alloc_addr(cpu_get_context(), paddr, length, KERNEL_MEM_START,
	                     KERNEL_MEM_END, MMF_KERNEL_FLAGS);
}

/**
 * Unmaps a memory area mapped with vm_alloc_kernel_addr.
 * @param vaddr  Virtual address of the memory
 * @param length Length in bytes
 * @return error code
 */
inline int vm_free_kernel_addr(vaddr_t vaddr, size_t length)
{
	return vm_free_addr(cpu_get_context(), vaddr, length);
}

#endif /*MM_VIRT_H*/
