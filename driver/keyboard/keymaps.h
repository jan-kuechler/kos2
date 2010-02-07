#ifndef KEYMAPS_H
#define KEYMAPS_H

#define NUM_KEYS 128
#define EOT      0xFF

struct km_entry
{
	char normal;
	char shift;
	char altgr;
	char ctrl;
};

typedef struct km_entry *keymap_t;

extern struct km_entry keymap_de[];

#endif /*KEYMAPS_H*/
