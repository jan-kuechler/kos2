#include "bitop.h"
#include "error.h"
#include "kernel.h"
#include "gdt.h"
#include "tss.h"

#define GDT_BYTE_GRAN 0x40 /* 0100b */
#define GDT_PAGE_GRAN 0xC0 /* 1100b */

struct gdt_entry gdt[GDT_SIZE] __attribute__((aligned(8)));

int init_gdt(void)
{
	/* NOTE: The order of the descriptors is important, see gdt.h. */

	/* NULL descriptor */
	gdt_set_desc(0, 0x00, 0x00, GDT_NULL, KERN_DPL, false);

	/* Kernel code and data segments, both 4GB with base addr 0 */
	gdt_set_desc(1, 0xFFFFFFFF, 0x00000000,
	             GDT_PRESENT | GDT_SEGMENT | GDT_CODE, KERN_DPL, false);
	gdt_set_desc(2, 0xFFFFFFFF, 0x00000000,
	             GDT_PRESENT | GDT_SEGMENT | GDT_DATA, KERN_DPL, false);

	/* User code and data segments, both 4GB with base addr 0 */
	gdt_set_desc(3, 0xFFFFFFFF, 0x00000000,
	             GDT_PRESENT | GDT_SEGMENT | GDT_CODE, USER_DPL, false);
	gdt_set_desc(4, 0xFFFFFFFF, 0x00000000,
               GDT_PRESENT | GDT_SEGMENT | GDT_DATA, USER_DPL, false);

	/* TSS segment */
	gdt_set_desc(5, sizeof(struct tss)-1, (uint32_t)&tss,
	             GDT_PRESENT | GDT_TSS, USER_DPL, true);

	struct
	{
		uint16_t size;
		uint32_t base;
	} __attribute__((packed)) gdt_ptr = {
		.size = GDT_SIZE*8-1,
		.base = (uint32_t)gdt,
	};

	printk(KERN_DEBUG "Loading GDT");
	asm volatile(
		"lgdtl %0           \n\t" /* load the gdt */
		"ljmpl $0x08, $1f   \n\t" /* jump to new code segment (loads cs)
		                             (0x08 => 2. entry in the gdt) */
		"1:                 \n\t"
		"mov   $0x10, %%eax \n\t" /* 0x10 is the new data selector
		                            (3. entry in the gdt) */
		"mov   %%eax, %%ds  \n\t" /* reload all segment registers */
		"mov   %%eax, %%es  \n\t"
		"mov   %%eax, %%fs  \n\t"
		"mov   %%eax, %%gs  \n\t"
		"mov   %%eax, %%ss  \n\t"
		: : "m"(gdt_ptr) : "eax");

	printk(KERN_DEBUG "Loading TSS");
	asm volatile("ltr %%ax \n\t" : : "a"(GDT_SEL_TSS));

	return OK;
}

void gdt_set_desc(int segment, uint32_t size, uint32_t base, uint8_t access,
                  uint8_t dpl, bool byte_gran)
{
	kassert(segment < GDT_SIZE);

	gdt[segment].size      = bmask(size, BMASK_WORD);
	gdt[segment].base_low  = bmask(base, BMASK_WORD);
	gdt[segment].base_mid  = bmask(base >> 16, BMASK_BYTE);
	gdt[segment].base_high = bmask(base >> 24, BMASK_BYTE);
	gdt[segment].access    = access | (bmask(dpl, BMASK_2BIT) << 5);
	gdt[segment].flags     = bmask(size >> 16, BMASK_4BIT);

	if (byte_gran) {
		gdt[segment].flags |= GDT_BYTE_GRAN; /* => 0100xxxx */
	}
	else {
		gdt[segment].flags |= GDT_PAGE_GRAN; /* => 1100xxxx */
	}
}
