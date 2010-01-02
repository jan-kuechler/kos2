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
static int default_set_cursor(unsigned int, unsigned int);
static int default_get_cursor(unsigned int *, unsigned int *);
static int default_get_state(struct con_state *);

#define OPT_DEFAULT(con,func) if (!(con)->func) (con)->func = default_##func;

int con_set(struct console *con)
{
	struct con_state state = {};
	if (cur_console) {
		cur_console->get_state(&state);
		cur_console->free();
	}

	OPT_DEFAULT(con, init);
	OPT_DEFAULT(con, free);
	OPT_DEFAULT(con, status);
	OPT_DEFAULT(con, puts);
	OPT_DEFAULT(con, putc);
	OPT_DEFAULT(con, loglevel);
	OPT_DEFAULT(con, set_cursor);
	OPT_DEFAULT(con, get_cursor);
	OPT_DEFAULT(con, get_state);

	cur_console = con;
	con->init(&state);

	return con->status();
}

static int default_init(struct con_state *dummy)
{
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

static int default_puts(const char *dummy)
{
	return 0;
}

static int default_putc(char dummy)
{
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

static int default_set_cursor(unsigned int dummy1, unsigned int dummy2)
{
	return -E_IMPL;
}

static int default_get_cursor(unsigned int *dummy1, unsigned int *dummy2)
{
	return -E_IMPL;
}

static int default_get_state(struct con_state *dummy)
{
	return -E_IMPL;
}
