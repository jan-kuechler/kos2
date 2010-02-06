#ifndef COMPILER_H
#define COMPILER_H

#define linker_symbol(name) extern void name(void)

#define __unused(x) (void)x;

#define __section(s) __attribute__((section(#s)))

#endif /*COMPILER_H*/
