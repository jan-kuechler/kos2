#include <compiler.h>
#include <stdint.h>
#include <string.h>
#include <libutil.h>
#include "console.h"
#include "error.h"
#include "ports.h"

#define VIDEO_MEM ((uint16_t*)0xB8000)

#define HEIGHT 25
#define WIDTH  80
#define NUM_CHARS (WIDTH * HEIGHT)
#define MEM_SIZE  (NUM_CHARS * 2)

#define CPOS(x,y) ((x) + (y) * WIDTH)

static int init(struct con_state *s);
static int clear(void);
static int putc(char c);
static int loglevel(void);
static int get_state(struct con_state *s);

static struct console boot_con = {
	.init      = init,
	.clear     = clear,
	.putc      = putc,
	.loglevel  = loglevel,
	.get_state = get_state,
};

static struct con_state state;

static uint16_t *vmem = VIDEO_MEM;

static inline uint16_t CH(char c)
{
	return (c | (state.color << 8));
}

static int init(struct con_state *s)
{
	// ignore old state, this is the boot console it
	// will be empty anyways
	 __unused(s);

	state.height = HEIGHT;
	state.width  = WIDTH;
	state.x      = 0;
	state.y      = 0;
	state.color  = 0x07; // bright grey on black
	state.buffer = VIDEO_MEM;

	// hide cursor (put it in 0x07D0 = 26. row)
	outb(0x3D4, 14);
	outb(0x3D5, 0x07);
	outb(0x3D4, 15);
	outb(0x3D4, 0xD0);

	clear();

	return OK;
}

static int clear(void)
{
	memsetw(vmem, CH(' '), NUM_CHARS);
	return OK;
}

static void scroll()
{
	if (state.y > 0) {
		memmove(vmem, vmem + WIDTH, (MEM_SIZE - (2 * WIDTH)));
		memsetw(vmem + (NUM_CHARS - WIDTH), CH(' '), WIDTH);
		state.y--;
	}
}

static int putc(char c)
{
	switch (c) {
	case '\n':
		state.x = 0;
		state.y++;
		break;

	case '\r':
		state.x = 0;
		break;

	case '\b':
		if (state.x > 0) {
			state.x--;
			vmem[CPOS(state.x,state.y)] = CH(' ');
		}
		break;

	case '\t':
		do {
			putc(' ');
		} while (state.x % 8);
		break;

	default:
		vmem[CPOS(state.x,state.y)] = CH(c);
		state.x++;
	};

	if (state.x >= WIDTH) {
		state.y++;
		state.x = 0;
	}

	while (state.y >= HEIGHT) {
		scroll();
	}

	return OK;
}

static int loglevel(void)
{
	return 7; // everything
}

static int get_state(struct con_state *s)
{
	if (!s) return -E_INVALID;
	*s = state;
	return OK;
}

int init_bootcon(void)
{
	return con_set(&boot_con);
}
