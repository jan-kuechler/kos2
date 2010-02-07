#ifndef KBC_H
#define KBC_H

#include <stdbool.h>

#define KBC_IRQ 0x01

void kbc_led(bool caps, bool num, bool scroll);
void kbc_reset_cpu(void);

int kbc_init(void);

#endif /*KBC_H*/
