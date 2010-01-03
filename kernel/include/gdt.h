#ifndef GDT_H
#define GDT_H

#include <stdbool.h>
#include <stdint.h>

#define GDT_NULL    0x00
#define GDT_SEGMENT 0x10
#define GDT_PRESENT 0x80

#define GDT_CODE 0x0A
#define GDT_DATA 0x02
#define GDT_TSS  0x09

#define GDT_SIZE 6 /* Number of gdt entries */

#define USER_DPL 3
#define KERN_DPL 0

/* IMPORTANT:
 * Change these whenever you change the order
 * of the gdt_set_desc calls in init_gdt!
 * If anything is changed edit int.s!
 */
#define GDT_SEL_NULL  0x00
#define GDT_SEL_CODE  0x08
#define GDT_SEL_DATA  0x10
#define GDT_SEL_UCODE 0x18
#define GDT_SEL_UDATA 0x20
#define GDT_SEL_TSS   0x28

struct gdt_entry {
	uint16_t size;			/**< contains lower 16 bits of the size */
	uint16_t base_low;  /**< contains lower 16 bits of the base */
	uint8_t base_mid;   /**< contains middle 8 bits of the base */
	uint8_t access;     /**< contains the access bits */
	uint8_t flags;      /**< contains flags and upper 4 bits of the size */
	uint8_t base_high;  /**< contains upper 8 bits of the base */
} __attribute__((packed));

/**
 * Initializes the global descriptor table.
 * @return error code
 */
int  init_gdt(void);

/**
 * Sets a GDT gate.
 * @param segment Segment number, (0 to GDT_SIZE-1)
 * @param size    Size of the segment
 * @param base    Base of the segment
 * @param access  Access flags
 * @param dpl     Descriptor privilege level
 * @param byte_gran Using byte granularity
 */
void gdt_set_desc(int segment, uint32_t size, uint32_t base, uint8_t access,
                  uint8_t dpl, bool byte_gran);

/**
 * Checks whether the given selector is in usermode.
 * @param selector The segment selector
 * @return Is usermode
 */
inline bool gdt_is_user(uint8_t selector)
{
	return (selector & 0x03) == USER_DPL;
}

#endif /*GDT_H*/
