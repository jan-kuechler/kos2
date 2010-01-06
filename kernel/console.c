#include <compiler.h>
#include <stdlib.h>
#include "error.h"
#include "console.h"
#include "kernel.h"

struct console *cur_console;

static int default_init(struct con_state *);
static int default_free(void);
static int default_status(void);
static int default_puts(const char *);
static int default_putc(char);
static int default_clear(void);
static int default_loglevel(void);
static int default_get_state(struct con_state *);

#define OPT_DEFAULT(con,func) if (!(con)->func) (con)->func = default_##func;

int con_set(struct console *con)
{
	struct con_state state = {0};
	if (cur_console) {
		cur_console->get_state(&state);
		cur_console->free();
	}

	OPT_DEFAULT(con, init);
	OPT_DEFAULT(con, free);
	OPT_DEFAULT(con, clear);
	OPT_DEFAULT(con, status);
	OPT_DEFAULT(con, puts);
	OPT_DEFAULT(con, putc);
	OPT_DEFAULT(con, loglevel);
	OPT_DEFAULT(con, get_state);

	cur_console = con;
	con->init(&state);

	return con->status();
}

static int default_init(struct con_state *dummy)
{
	__unused(dummy);
	return OK;
}

static int default_free(void)
{
	return OK;
}

static int default_status(void)
{
	return OK;
}

static int default_puts(const char *str)
{
	int err;
	int count = 0;
	while (*str) {
		err = cur_console->putc(*str++);
		if (err != OK) {
			seterr(err);
			return count;
		}
		count++;
	}
	return count;
}

static int default_putc(char dummy)
{
	__unused(dummy);
	return -E_IMPL;
}

static int default_clear(void)
{
	return -E_IMPL;
}

static int default_loglevel(void)
{
	return LOGLVL_SILENT;
}

static int default_get_state(struct con_state *dummy)
{
	__unused(dummy);
	return -E_IMPL;
}
