/* See LICENSE file for copyright and license details */
#ifndef HIDAMARI_H
#define HIDAMARI_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "type.h"
#include "field.h"

enum tile {
	TILE_SPACE = 0,
	TILE_PIECE_I,
	TILE_PIECE_J,
	TILE_PIECE_L,
	TILE_PIECE_O,
	TILE_PIECE_S,
	TILE_PIECE_T,
	TILE_PIECE_Z,
	TILE_FALLEN,
	TILE_WALL,
	TILE_PLAIN,
	TILE_0 = 48,
	TILE_1,
	TILE_2,
	TILE_3,
	TILE_4,
	TILE_5,
	TILE_6,
	TILE_7,
	TILE_8,
	TILE_9,
	TILE_A = 65,
	TILE_B,
	TILE_C,
	TILE_D,
	TILE_E,
	TILE_F,
	TILE_G,
	TILE_H,
	TILE_I,
	TILE_J,
	TILE_K,
	TILE_L,
	TILE_M,
	TILE_N,
	TILE_O,
	TILE_P,
	TILE_Q,
	TILE_R,
	TILE_S,
	TILE_T,
	TILE_U,
	TILE_V,
	TILE_W,
	TILE_X,
	TILE_Y,
	TILE_Z,
	TILE_a = 97,
	TILE_b,
	TILE_c,
	TILE_d,
	TILE_e,
	TILE_f,
	TILE_g,
	TILE_h,
	TILE_i,
	TILE_j,
	TILE_k,
	TILE_l,
	TILE_m,
	TILE_n,
	TILE_o,
	TILE_p,
	TILE_q,
	TILE_r,
	TILE_s,
	TILE_t,
	TILE_u,
	TILE_v,
	TILE_w,
	TILE_x,
	TILE_y,
	TILE_z,
	TILE_LAST, /* Not an actual tile, just used for enum length */
};

struct ai_state {
	bool active;
	void *region;
	enum button const *planstr;
	u8 skill;
};

struct hidamari {
	enum gamestate state;
	struct field field;
	struct ai_state ai;
};

/* Initialize the game state */
void
hidamari_init(struct hidamari *game);

/* Update the field by one timestep:
 *	Perform the player action;
 *	Move current piece downwards;
 *	Clear any rows;
 *	Update the score.
 *
 * This is the only function needed to run the game after initialization.
 */
void
hidamari_update(struct hidamari *game, enum button act);

void
hidamari_print(struct hidamari *game);

#endif
