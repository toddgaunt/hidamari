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
	for (i = 0; i < 4; ++i) {
		x = hidamari_orientation[field->current.shape]
		                              [field->current.orientation]
		                              [i].x + field->current.pos.x;
		y = hidamari_orientation[field->current.shape]
		                        [field->current.orientation]
		                        [i].y + field->current.pos.y;
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

	srand(time(NULL));
	ralloc_aquire(1024 << 10);
	hidamari_init(&game);
	game.field.grid[0] |= 7 << 1;
	size_t i = 0;
	while(i < 100000) {
		//usleep(100000);
		hidamari_update(&game, BUTTON_NONE);
		i++;
	}
	hidamari_buffer_draw(&game);
	dump_field(&game.field);
}
