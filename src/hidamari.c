/* See LICENSE file for copyright and license details */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hidamari.h"
#include "vga.h"

/*
 * Public API
 */

void
hidamari_init(struct hidamari *game)
{
	static enum button const dbv = BTN_NONE;

	memset(game, 0, sizeof(*game));
	game->state = GAMESTATE_PLAYING;
	//game->ai.region = region_create(ai_size_requirement());
	//game->ai.planstr = &dbv;
	game->ai.active = false;
	field_init(&game->field);
}

void
hidamari_update(struct hidamari *game, enum button in)
{
	double weight[3] = {0.848058, 2.304684, 1.405450};

	switch(game->state) {
	case GAMESTATE_INIT:
		hidamari_init(game);
		break;
	case GAMESTATE_PLAYING:
		game->state = field_update(&game->field, in);
		break;
	case GAMESTATE_OVER:
		game->state = GAMESTATE_PLAYING;
		field_init(&game->field);
		break;
	}
}

void
hidamari_render(struct vga *vp, struct hidamari *game)
{
	vga_fill(vp, 0x000000);
	field_draw(vp, &game->field, 0, 0);
}
