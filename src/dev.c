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
	enum button button = BTN_NONE;
	struct hidamari game;

	srand(time(NULL));
	hidamari_init(&game);
	for (;;) {
		hidamari_update(&game, BTN_NONE);
		hidamari_print(&game);
        usleep(100);
	}
	return 0;
}
