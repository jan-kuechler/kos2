#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define TIMER_HZ 100
#define TIMER_DEFAULT 1193180

#define msec_to_tick(ms) ((ms) * TIMER_HZ/1000)
#define sec_to_tick(s)   ((s)  * TIMER_HZ)

extern volatile uint64_t timer_ticks;

int init_timer(void);

#endif /*TIMER_H*/
