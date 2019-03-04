/* See LICENSE file for copyright and license details */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ai.h"
#include "hidamari.h"

/*
 * Public API
 */

void
hidamari_init(struct hidamari *game)
{
	static button const dbv = BTN_NONE;

	memset(game, 0, sizeof(*game));
	game->state = GAMESTATE_PLAYING;

	game->ai.planstr = &dbv;
	game->ai.active = true;
    game->ai.region = malloc(ai_size() * 2);

	field_init(&game->field);
}

void
hidamari_update(struct hidamari *game, button in)
{
	double weight[3] = {0.848058, 2.304684, 1.405450};

	switch(game->state) {
	case GAMESTATE_INIT:
		hidamari_init(game);
		break;
	case GAMESTATE_PLAYING:
        if (!*game->ai.planstr)
            game->ai.planstr = ai_plan(game->ai.region, weight, &game->field);
        if (false == field_update(&game->field, game->ai.planstr[0]))
            game->state = GAMESTATE_OVER;
        ++game->ai.planstr;
		break;
	case GAMESTATE_OVER:
		field_init(&game->field);
		game->state = GAMESTATE_PLAYING;
		break;
	}
}

void
hidamari_print(struct hidamari *game)
{
	/* Score */
	printf("%010d\n", game->field.score);

	/* Field */
	field_print(&game->field);
}

void
hidamari_render(struct vga *vga, struct hidamari *game)
{
    field_draw(vga, NULL, &game->field, 1, 0, 0);
}
