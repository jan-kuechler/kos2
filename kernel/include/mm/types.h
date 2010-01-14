#ifndef MM_TYPES_H
#define MM_TYPES_H

#include <stdint.h>

#define _aligned_
#define _unaligned_

#define _phys_
#define _virt_

#define KILOBYTE (1024)
#define MEGABYTE (1024*1024)

#define PAGES_TO_KILOBYTE(p) ((p)*4)
#define PAGES_TO_MEGABYTE(p) (((p)*4)/1024)

/** Physical address */
typedef void *paddr_t;
/** Virtual address */
typedef void *vaddr_t;

/** A page directory/table entry */
typedef uint32_t pany_entry_t;
/** A page directory entry */
typedef uint32_t pdir_entry_t;
/** A page table entry */
typedef uint32_t ptab_entry_t;

/** A page directory or table */
typedef pany_entry_t *pany_t;
/** A page directory */
typedef pdir_entry_t *pdir_t;
/** A page table */
typedef ptab_entry_t *ptab_t;

/**
 * An address space
 */
struct mm_context
{
	paddr_t phys; /**< Physical address of the page directory */
	pdir_t  pdir; /**< Virtual address of the page directory */
};

/**
 * A pair of a virtual and a physical address.
 */
struct addrpair
{
	paddr_t phys;
	vaddr_t virt;
};

#endif /*MM_TYPES_H*/
