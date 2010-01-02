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

typedef uint32_t pany_entry_t;
typedef uint32_t pdir_entry_t;
typedef uint32_t ptab_entry_t;

typedef pany_entry_t *pany_t;
typedef pdir_entry_t *pdir_t;
typedef ptab_entry_t *ptab_t;

struct addrspace
{
	paddr_t phys;
	pdir_t  pdir;
};

struct addrpair
{
	paddr_t phys;
	vaddr_t virt;
};

#endif /*MM_TYPES_H*/
