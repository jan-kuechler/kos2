#include <stdarg.h>
#include <stdbool.h>
#include "error.h"
#include "console.h"
#include "libutil.h"
#include "kernel.h"

static int getlvl(char lvl)
{
	if (lvl >= '0' && lvl <= '7')
		return lvl - '0';
	if (lvl == 'd')
		return LOGLVL_DEFAULT;
	return -1;
}

int vprintk(const char *fmt, va_list args)
{
	int loglvl = LOGLVL_DEFAULT;
	if (fmt[0] == '<') {
		if (!fmt[1])
			return -E_INVALID;
		if (fmt[2] != '>')
				return -E_INVALID;

		loglvl = getlvl(fmt[1]);
		fmt += 3;
	}

	char buffer[2048] = "";
	int num = strafmt(buffer, fmt, args);
	bool put_nl = buffer[num - 1] == '\n';

	int written = num;
	if (loglvl <= cur_console->loglevel()) {
		written = cur_console->puts(buffer);
		if (put_nl)
			cur_console->putc('\n');
	}
	if (num != written)
		return -geterr();
	return num;
}

int printk(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int r = vprintk(fmt, args);
	va_end(args);
	return r;
}
