/* See LICENSE file for copyright and license details */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unistd.h>

#include "hidamari.h"

int
main()
{
	HidamariBuffer buf;
	HidamariGame game;

	srand(time(NULL));
	hidamari_init(&buf, &game);
	game.field.grid[0] |= 7 << 1;
	while(1) {
		usleep(100000);
		hidamari_update(&buf, &game, BUTTON_NONE);
	}
}
