#ifndef DEBUG_H
#define DEBUG_H

#define dbg_bochs_break() asm volatile("xchg %ebx, %ebx")

#endif /*DEBUG_H*/
