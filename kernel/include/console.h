#ifndef CONSOLE_H
#define CONSOLE_H

/**
 * Console state
 */
struct con_state
{
	unsigned int width, height; /**< Console dimension */
	unsigned int x, y;          /**< Cursor position */
	const char *buffer;         /**< Old output */
};

/**
 * A console driver
 */
struct console
{
	/** Initializes the console */
	int (*init)(struct con_state *state);

	/** Frees the console */
	int (*free)(void);

	int (*status)(void);

	/** Prints a string */
	int (*puts)(const char *str);

	 /** Prints a character */
	int (*putc)(char c);

	int (*clear)(void);

	/**< Returns the current loglevel */
	int (*loglevel)(void);

	int (*set_cursor)(unsigned int x, unsigned int y);
	int (*get_cursor)(unsigned int *x, unsigned int *y);
	int (*get_state)(struct con_state *state);
};

extern struct console *cur_console;

int con_set(struct console *con);

#endif /*CONSOLE_H*/
