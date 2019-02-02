/* See LICENSE file for copyright and license details */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hidamari.h"
#include "region.h"
#include "ai.h"

static void
dump_field(HidamariBuffer const *buf)
{
	size_t y, x;

	for (x = 0; x < HIDAMARI_BUFFER_WIDTH; ++x) {
		for (y = 0; y < HIDAMARI_BUFFER_HEIGHT; ++y) {
			if (HIDAMARI_TILE_SPACE == buf->tile[x][y]) {
				putc('.', stdout);
			} else {
				putc('#', stdout);
			}
			putc(' ', stdout);
		}
		putc('\n', stdout);
	}
	printf("--\n");
}

int
main()
{
	HidamariGame game = {0};

	srand(time(NULL));
	game.ai.active = false;
	game.state = HIDAMARI_GS_GAME_PLAYING;
	for (;;) {
		printf("top-right: %d, %d\n",
				game.field.current.pos.x,
				game.field.current.pos.y);
		dump_field(&game.buf);
		hidamari_update(&game, BUTTON_NONE);
	}
	return 0;
}
