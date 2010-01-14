#ifndef MM_PAGE_H
#define MM_PAGE_H

#include <stdint.h>

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE - 1))

#define PE_ADDR_SHIFT 22

#define PDIR_LEN 1024
#define PTAB_LEN 1024

#define PTAB_MEM (PTAB_LEN * PAGE_SIZE)

#define BMASK_4K_ALIGN 0x00000FFF
#define BMASK_PE_ADDR  0xFFFFF000
#define BMASK_PE_FLAGS BMASK_4K_ALIGN

#define NO_PAGE ((paddr_t)1) /* 1 is not a valid addr for a page, as they must be 4k-aligned */

#define NUM_PAGES(n) ((((n) + ~PAGE_MASK) & PAGE_MASK) / PAGE_SIZE)

#define PAGE_OFFSET(addr) ((uintptr_t)(addr) % PAGE_SIZE)

#define PAGE_ALIGN_ROUND_UP(n) ((void*)(((uintptr_t)(n) + ~PAGE_MASK) & PAGE_MASK))
#define PAGE_ALIGN_ROUND_DOWN(n) ((void*)((uintptr_t)(n) & PAGE_MASK))

/**
 * Checks whether the address is page aligned.
 * @param addr The address
 * @return true for aligned addresses.
 */
inline bool page_aligned(void *addr)
{
	return ((uintptr_t)addr & BMASK_4K_ALIGN) == 0;
}

#endif /*MM_PAGE_H*/
