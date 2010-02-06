#include <string.h>
#include "kernel.h"
#include "mm.h"

struct mm_context *mc_create(void)
{
	struct mm_context *c = kmalloc(sizeof(*c));
	if (!c) {
		printk(KERN_ERR "mc: failed to alloc memory for another context.");
		return NULL;
	}
	memset(c, 0, sizeof(*c));

	c->phys = mm_alloc_page();
	if (c->phys == NO_PAGE) {
		printk(KERN_ERR "mc: failed to allocate a page for the page directory.");
		kfree(c);
		return NULL;
	}

	c->pdir = vm_alloc_kernel_addr(c->phys, PAGE_SIZE);
	if (!c->pdir) {
		printk(KERN_ERR "mc: failed to map page directory.");
		mm_free_page(c->phys);
		kfree(c);
		return NULL;
	}

	memset(c->pdir, 0, PAGE_SIZE);
	memcpy(c->pdir, kernel_context, NUM_KERNEL_PTABS * sizeof(pdir_entry_t));

	return c;
}

void mc_destroy(struct mm_context *c)
{
	if (cpu_get_context() == c) {
		panic("Cannot destroy the current context!");
	}

	vm_free_kernel_addr(c->pdir, PAGE_SIZE);
	mm_free_page(c->phys);
	kfree(c);
}
