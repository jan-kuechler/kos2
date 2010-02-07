#include <stdint.h>
#include <bitop.h>
#include <error.h>
#include <ports.h>

#include "kbc.h"

#define KBC_DATA 0x60
#define KBC_CMD  0x64

#define STATUS_OUTB  0x01
#define STATUS_INB   0x02
#define STATUS_TEST  0x04
#define STATUS_LASTP 0x08
#define STATUS_LOCK  0x10
#define STATUS_AUX   0x20
#define STATUS_TOUT  0x40
#define STATUS_PERR  0x80

#define CMD_WRITE_OUTP 0xD1

#define SET_LED   0xED
#define SET_SPEED 0xF3
#define ACTIVATE  0xF4
#define RESET_CPU 0xFE


// this translates scancodes to keycodes
static int keycode[][128] = {
	{ // normal scancodes
		 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
		10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
		20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
		30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
		40,  41,  42,  43,  44,  45,  46,  47,  48,  49,
		50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
		60,  61,  62,  63,  64,  65,  66,  67,  68,  69,
		70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
		80,  81,  82,  84,  00,  00,  86,  87,  88,  00,
		00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
		00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
		00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
		00,  00,  00,  00,  00,  00,  00,  00
	},
	{ // E0 scancodes
		00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
		00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
		00,  00,  00,  00,  00,  00,  00,  00,  00,  97,
		00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
		00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
		00,  00,  00,  00,  00,  00, 100,  00,  00,  00,
		00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
		00, 102, 103, 104,  00, 105,  00, 106,  00, 107,
		108, 109, 110, 111,  00,  00,  00,  00,  00,  00,
		00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
		00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
		00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
		00,  00,  00,  00,  00,  00,  00,  00
	},
};

uint8_t kbc_scan_to_keycode(uint8_t elvl, uint16_t scancode)
{
	switch (elvl) {
	case 0:
		return keycode[0][scancode];

	case 1:
		return keycode[1][scancode];

	case 2:
		// special case for PAUSE key, witch results
		// in a E1 E0 45 1D make code (wtf!)
		if (scancode == 0x451D)
			return 119;
		// go along, there's no break here.
	default:
			return 0;
	}
}

static void clear(uint8_t what, uint8_t test)
{
	while (inb(what) & test) {
		// busy waiting...
	}
}

static inline void send(uint8_t cmd)
{
	clear(KBC_CMD, STATUS_INB);
	outb(KBC_DATA, cmd);
}

static void send2(uint8_t cmd, uint8_t data)
{
	send(cmd);
	send(data);
}

int kbc_init(void)
{
	// clear any old data from the kbc
	clear(KBC_CMD, STATUS_OUTB);

	kbc_led(false, false, false);

	send2(SET_SPEED, 0);
	send(ACTIVATE);

	return OK;
}

void kbc_led(bool caps, bool num, bool scroll)
{
	uint8_t led = 0;
	if (caps)   bsetn(led, 2);
	if (num)    bsetn(led, 1);
	if (scroll) bsetn(led, 0);

	send2(SET_LED, led);
}

void kbc_reset_cpu(void)
{
	send(RESET_CPU);
}
