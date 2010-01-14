#include "gdt.h"
#include "tss.h"

// the TSS used by the system for ring-changing-interrupts
struct tss tss = {
	.ss0   = GDT_SEL_DATA,
	.iomap = sizeof(struct tss) + 1, // invalid iomap, as it is not used
};
