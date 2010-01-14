#ifndef MM_FLAGS_H
#define MM_FLAGS_H

#define MMF_PRESENT       0x001
#define MMF_WRITE         0x002
#define MMF_USER          0x004
#define MMF_WRITE_THROUGH 0x008
#define MMF_NO_CACHE      0x010
#define MMF_ACCESSED      0x020
#define MMF_DIRTY         0x040
#define MMF_PAGE_SIZE     0x080

#define MMF_KERNEL_FLAGS (MMF_PRESENT | MMF_WRITE)
#define MMF_USER_FLAGS (MMF_PRESENT | MMF_WRITE | MMF_USER)

#endif /*MM_FLAGS_H*/
