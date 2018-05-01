/* See LICENSE file for copyright and license details */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hidamari.h"
#include "region.h"
#include "ai.h"

void
hidamari_pso_update(HidamariGame *game, double weight[3]);

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

size_t
fitness(HidamariGame *game)
{
	double weight[3] = {3, 2, 10};
	hidamari_init(game);
	do {
		hidamari_pso_update(game, weight);
		dump_field(&game->buf);
	} while (game->state == GS_GAME_PLAYING);
	return game->field.score;
}

int
main()
{
	HidamariGame game;

	srand(time(NULL));
	fitness(&game);
	return 0;
}
