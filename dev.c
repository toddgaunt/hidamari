/* See LICENSE file for copyright and license details */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unistd.h>

#include "hidamari.h"
#include "ralloc.h"

static void
dump_field(HidamariBuffer *buf)
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
	HidamariGame game;

	Button button;
	srand(time(NULL));
	ralloc_aquire(1024 << 10);
	hidamari_init(&game);
	game.field.grid[0] |= 7 << 1;
	for (;;) {
		printf("top-right: %d, %d\n",
				game.field.current.pos.x,
				game.field.current.pos.y);
		dump_field(&game.buf);
		printf("Enter command\n");
		switch(fgetc(stdin)) {
		case 'a': button = BUTTON_LEFT; break;
		case 's': button = BUTTON_DOWN; break;
		case 'd': button = BUTTON_RIGHT; break;
		case 'w': button = BUTTON_UP; break;
		case 'q': button = BUTTON_L; break;
		case 'e': button = BUTTON_R; break;
		case EOF: goto endgameloop;
		default: button = BUTTON_NONE; break;
		}
		fgetc(stdin);
		hidamari_update(&game, button);
	}
endgameloop:
	return 0;
}
