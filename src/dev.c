/* See LICENSE file for copyright and license details */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hidamari.h"
#include "vga.h"

#define SCRN_W 256
#define SCRN_H 192
#define TILE_S 8 

int
main()
{
	Button button = BTN_NONE;
	struct hidamari game;

	uint32_t *px = malloc(sizeof(*px) * SCRN_W * SCRN_H);
	struct vga vga = vga_init(px, SCRN_W, SCRN_H);

	srand(time(NULL));
	hidamari_init(&game);
	for (;;) {
		hidamari_update(&game, BTN_NONE);
		hidamari_render2(&vga, &game);
	}
	return 0;
}
