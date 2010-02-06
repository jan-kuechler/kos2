#ifndef LIBUTIL_H
#define LIBUTIL_H

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

int strfmt(char *buffer, const char *fmt, ...);
int strafmt(char *buffer, const char *fmt, va_list args);

char *strdup(const char *str);

void *memsetw(void *s, int16_t c, size_t n);

#endif /*LIBUTIL_H*/
