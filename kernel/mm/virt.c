#include <string.h>
#include <kos/config.h>
#include "bitop.h"
#include "error.h"
#include "kernel.h"
#include "mm/flags.h"
#include "mm/mm.h"
#include "mm/virt.h"

static struct mm_context _kernel_context;
struct mm_context *kernel_context = &_kernel_context;

ptab_t kernel_ptabs[NUM_KERNEL_PTABS];

static bool paging_enabled = false;

#ifdef CONF_SAFE_VMEM
#define _fail(fmt, ...) panic("[%s] "fmt, __func__, ##__VA_ARGS__);
#else
#define _fail(fmt, ...) printk(KERN_ERR "[%s] " fmt, __func__, ##__VA_ARGS__)
#endif

#define page2addr(page) ((paddr_t)(page * PAGE_SIZE))
#define addr2page(addr) ((uintptr_t)addr / PAGE_SIZE)

#define pdir_index(addr) (addr2page(addr) / PDIR_LEN)
#define ptab_index(addr) (addr2page(addr) % PTAB_LEN)

#define get_addr(pdi,pti,offs) (((pdi) << PE_ADDR_SHIFT) + ((pti) << PAGE_SHIFT) + (offs))

static inline paddr_t getaddr(pany_entry_t entry)
{
	return (paddr_t)bmask((uint32_t)entry, BMASK_PE_ADDR);
}

static inline uint32_t getflags(pany_entry_t entry)
{
	return (uint32_t)bmask((uint32_t)entry, BMASK_PE_FLAGS);
}

static inline void align_addr_and_size(vaddr_t *addr, size_t *size)
{
	*size = *size + PAGE_OFFSET(*addr);
	*addr = (vaddr_t)((uintptr_t)*addr - PAGE_OFFSET(*addr));
}

static inline vaddr_t get_ptab(pdir_t pdir, size_t pdi)
{
	if (!paging_enabled)
		return getaddr(pdir[pdi]);

	if (pdir == cpu_get_context()->pdir) {
		// this works, because the page directory is mapped into itself
		//printk(KERN_DEBUG "get_ptab => %p", (vaddr_t)(PTAB_BASE_ADDR +	(PAGE_SIZE * pdi)));
		return (vaddr_t)(PTAB_BASE_ADDR +	(PAGE_SIZE * pdi));
	}
	else {
		vaddr_t addr = vm_alloc_kernel_addr(getaddr(pdir[pdi]), PAGE_SIZE);
		//printk(KERN_DEBUG "get_ptab => mapping to %p", addr);
		return addr;
	}
}

static inline void put_ptab(pdir_t pdir, ptab_t ptab)
{
	if (!paging_enabled)
		return;
	if (pdir != cpu_get_context()->pdir) {
		vm_free_kernel_addr(ptab, PAGE_SIZE);
	}
}

static inline uint32_t make_entry(paddr_t addr, int flags)
{
	return ((uintptr_t)addr) | flags;
}

static inline void flush_tlb(vaddr_t vaddr)
{
	asm volatile("invlpg %0" : : "m"(*(char*)vaddr));
}

static vaddr_t find_addrs(pdir_t pdir, vaddr_t vstart, vaddr_t vend, int num);
static int do_map(pdir_t pdir, vaddr_t vaddr, paddr_t paddr,
                  int flags, bool override);

int init_vmem(void)
{
	// Create the kernel context
	kernel_context->phys = mm_alloc_page();
	if (kernel_context->phys == NO_PAGE) {
		return -E_NO_MEM;
	}
	memset(kernel_context->pdir, 0, PAGE_SIZE);
	kernel_context->pdir = kernel_context->phys;

	// Map the page directory into itself
	kernel_context->pdir[PTAB_BASE_INDEX] =
		make_entry(kernel_context->pdir, MMF_PRESENT | MMF_WRITE);

	// Create and map the kernel page tables
	int i=0;
	for (; i < NUM_KERNEL_PTABS; ++i) {
		kernel_ptabs[i] = mm_alloc_page();
		if (kernel_ptabs[i] == NO_PAGE)
			return -E_NO_MEM;
		memset(kernel_ptabs[i], 0, PAGE_SIZE);
		kernel_context->pdir[i] = make_entry(kernel_ptabs[i],
 		                                     MMF_PRESENT | MMF_WRITE);
	}

	// Map the kernel and important memory areas
	//TODO: Map the kernel

	printk(KERN_DEBUG "Mapping kernel");
	vm_map(kernel_context, kernel_start, kernel_start, MMF_KERNEL_FLAGS,
	       NUM_PAGES(kernel_end - kernel_start));
	vm_map(kernel_context, (paddr_t)0xB8000, (vaddr_t)0xB8000, MMF_KERNEL_FLAGS,
	       NUM_PAGES(80*25*2));
	vm_map(kernel_context, kernel_context->pdir, kernel_context->pdir,
	       MMF_KERNEL_FLAGS, 1);

	// Enable paging
	//TODO: Enable paging

	printk(KERN_DEBUG "Enabling paging...");
	asm volatile("movl %%eax, %%cr3       \n"
							 "movl %%cr0, %%eax       \n"
							 "or   $0x80000000, %%eax \n"
							 "mov  %%eax, %%cr0       \n"
							:
							: "a"(kernel_context->pdir)
						  );
	paging_enabled = true;
	printk("Done.");

	return OK;
}

int vm_map(struct mm_context *context, vaddr_t vaddr, paddr_t paddr,
           int flags, size_t num)
{
	if (!vaddr || !page_aligned(vaddr) || !page_aligned(paddr) || !context ||
	    !num || bnotset(flags, MMF_PRESENT)) {
		printk(KERN_ERR "[vm_map] invalid parameters");
		printk(KERN_ERR "context: %p vaddr: %p paddr: %p", context, vaddr, paddr);
		printk(KERN_ERR "flags: %b num: %d", flags, num);
		return -E_INVALID;
	}

	int i=0;
	int err = OK;
	for (; i < num; ++i) {
		vaddr_t v = (vaddr_t)((uintptr_t)vaddr + (i * PAGE_SIZE));
		paddr_t p = (paddr_t)((uintptr_t)paddr + (i * PAGE_SIZE));
		err = do_map(context->pdir, v, p, flags, false);
		if (err != OK) {
			_fail("do_map(%p, %p, %p, %b, false) failed with error %s",
			      context->pdir, v, p, flags, strerr(err));
			break;
		}
	}

	if (err != OK) {
		vm_unmap(context, vaddr, i);
		return err;
	}
	return OK;
}

int vm_unmap(struct mm_context *context, vaddr_t vaddr, size_t num)
{
	if (!context || !vaddr || !page_aligned(vaddr) || !num)
		return -E_INVALID;

	int i=0;
	for (; i < num; ++i) {
		vaddr_t v = (vaddr_t)((uintptr_t)vaddr + (i * PAGE_SIZE));
		do_map(context->pdir, v, NULL, 0, true);
	}
	return OK;
}

vaddr_t vm_alloc_addr(struct mm_context *context, paddr_t paddr, size_t length,
                      vaddr_t vstart, vaddr_t vend, int flags)
{
	if (!context || !paddr || !length || bnotset(flags, MMF_PRESENT)) {
		_fail("invalid parameters");
		seterr(E_INVALID);
		return NULL;
	}

	align_addr_and_size(&paddr, &length);
	size_t pages = NUM_PAGES(length);

	vaddr_t vaddr = find_addrs(context->pdir, vstart, vend, pages);
	if (!vaddr) {
		_fail("no free address for %d page%s in [%p, %p]", pages, pages>1?"s":"", vstart, vend);
		seterr(E_NO_MEM);
		return NULL;
	}

	int result = vm_map(context, vaddr, paddr, flags, NUM_PAGES(length));
	if (result != OK) {
		_fail("failed to map pages.");
		seterr(result);
		return NULL;
	}
	return vaddr;
}

int vm_free_addr(struct mm_context *context, vaddr_t vaddr, size_t length)
{
	if (!context || !vaddr || !length) {
		return -E_INVALID;
	}

	align_addr_and_size(&vaddr, &length);

	return vm_unmap(context, vaddr, NUM_PAGES(length));
}

static vaddr_t find_addrs(pdir_t pdir, vaddr_t vstart, vaddr_t vend, int num)
{
	uintptr_t lower = (uintptr_t)vstart;
	uintptr_t upper = (uintptr_t)vend;

	if (!pdir || !num || ((upper - lower) / PAGE_SIZE) < num) {
		return NULL;
	}

	int count = 0;
	uintptr_t result = lower;
	uintptr_t pti = ptab_index(lower);
	uintptr_t pdi = pdir_index(lower);

	while (count < num && pdi < pdir_index(upper)) {
		if (bisset(getflags(pdir[pdi]), MMF_PRESENT)) {
			ptab_t ptab = get_ptab(pdir, pdi);

			for (; pti < PTAB_LEN; ++pti) {
				if (bnotset(getflags(ptab[pti]), MMF_PRESENT)) {
					count++;
				}
				else {
					count = 0;
					result = get_addr(pdi,pti+1,0);
				}
			}

			put_ptab(pdir, ptab);
		}
		else {
			// the page table is not present, so there are many free addresses!
			count += PTAB_LEN;
		}

		pti = 0;
		pdi++;
	}

	if (count >= num && (result + (num * PAGE_SIZE)) < upper)
		return (vaddr_t)result;
	return NULL;
}

static int do_map(pdir_t pdir, vaddr_t vaddr, paddr_t paddr,
                  int flags, bool override)
{
	if (!pdir || vaddr == NULL || !page_aligned(vaddr) || !page_aligned(paddr))
		return -E_INVALID;

	size_t pdi = pdir_index(vaddr);

	bool new_ptab = false;
	if (bnotset(getflags(pdir[pdi]), MMF_PRESENT)) {
		// There is no page table for this adress, create a new one
		paddr_t page = mm_alloc_page();
		if (page == NO_PAGE) {
			return -E_NO_MEM;
		}
		pdir[pdi] = make_entry(page, flags);
		new_ptab = true;
	}

	// Get the virtual address of the page table
	ptab_t ptab = get_ptab(pdir, pdi);
	if (new_ptab) {
		memset(ptab, 0, PAGE_SIZE);
	}

	size_t pti = ptab_index(vaddr);

	bool is_mapped = bisset(getflags(ptab[pti]), MMF_PRESENT);
	if (is_mapped && !override) {
		put_ptab(pdir, ptab);
		return -E_PRESENT;
	}

	ptab[pti] = make_entry(paddr, flags);

	if (cpu_get_context()->pdir == pdir) {
		// We're working on the active page directory,
		// remove any old cached addresses
		flush_tlb(vaddr);
	}

	put_ptab(pdir, ptab);

	return OK;
}
