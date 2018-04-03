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
dump_field(HidamariPlayField *field)
{
	int i;
	size_t x, y;
	char buf[HIDAMARI_WIDTH][HIDAMARI_HEIGHT];

	for (x = 0; x < HIDAMARI_WIDTH; ++x) {
		for (y = 0; y < HIDAMARI_HEIGHT; ++y) {
			if (field->grid[y] & 1 << x) {
				buf[x][y] = '#';
			} else {
				buf[x][y] = '.';
			}
		}
	}
	printf("shape: %d\n", field->current.shape);
	printf("top-right: %d, %d\n",
			field->current.pos.x,
			field->current.pos.y);
	for (i = 0; i < 4; ++i) {
		x = hidamari_orientation[field->current.shape]
		                              [field->current.orientation]
		                              [i].x + field->current.pos.x;
		y = field->current.pos.y - hidamari_orientation[field->current.shape]
		                        [field->current.orientation]
		                        [i].y;
		printf("x, y: %zu, %zu\n", x, y);
		buf[x][y] = '$';
	}
	printf("--\n");
	for (x = 0; x < HIDAMARI_WIDTH; ++x) {
		for (y = 0; y < HIDAMARI_HEIGHT; ++y) {
			putc(buf[x][y], stdout);
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
		dump_field(&game.field);
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
