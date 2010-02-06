//#include <mm/kmalloc.h>
#include <string.h>

extern void *kmalloc(unsigned int);

char *strdup(const char *str)
{
	char *dup = kmalloc(strlen(str) + 1);
	if (!dup)
		return 0;
	strcpy(dup, str);
	return dup;
}

