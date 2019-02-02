/* See LICENSE file for copyright and license details */
#ifndef HIDAMARI_H
#define HIDAMARI_H

#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#include "vec2.h"
#include "type.h"

#define FIELD_HEIGHT 23
#define FIELD_WIDTH 12

#define HIDAMARI_HEIGHT 23
#define HIDAMARI_WIDTH 12

#define HIDAMARI_HEIGHT_VISIBLE (HIDAMARI_HEIGHT - 3)
#define HIDAMARI_WIDTH_VISIBLE (HIDAMARI_WIDTH)

#define HIDAMARI_BUFFER_HEIGHT 24
#define HIDAMARI_BUFFER_WIDTH 32

/* Cursor index */
#define HIDAMARI_MAIN_CURSOR 0
#define HIDAMARI_OPTION_CURSOR 1

typedef u8 Button;

enum input {
	BTN_NONE = 0,
	/* D-pad */
	BTN_UP,
	BTN_DOWN,
	BTN_RIGHT,
	BTN_LEFT,
	/* Shoulder buttons */
	BTN_R,
	BTN_L,
	/* Face buttons */
	BTN_B,
	BTN_A,
	BTN_Y,
	BTN_X,
	BTN_LAST, /* Not an actual action, just used for enum length */
};

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

enum shape {
	SHAPE_I,        
	SHAPE_J,        
	SHAPE_L,        
	SHAPE_O,        
	SHAPE_S,        
	SHAPE_T,        
	SHAPE_Z,        
	SHAPE_LAST, /* Not an actual piece, just used for enum length. Also
			  can be used as a NULL value for hidamaris */
};

/* Current state of the game */
enum game_state {
	GAMESTATE_INIT = 0,
	GAMESTATE_PLAYING,
	GAMESTATE_OVER,
};

struct drawbuf  {
	enum tile tile[HIDAMARI_BUFFER_WIDTH][HIDAMARI_BUFFER_HEIGHT];
	u8 color[HIDAMARI_BUFFER_WIDTH][HIDAMARI_BUFFER_HEIGHT][3];
};

struct piece {
	i8 x;
	i8 y;
	i8 dir : 3;
	enum shape shape : 4;
};

struct field {
	/* Scoring */
	u8 level;
	u32 score; 
	u32 lines; 
	/* Timing */
	f32 gravity_timer; 
	u8 slide_timer;
	/* Randomization */
	u8 bag_pos; /* Current position in the bag */
	enum shape bag[7]; /* Random Bag, used for pseudo-random order */
	/* Hidamaries */
	enum shape next; /* Lookahead piece for player */
	struct piece current;
	u16 bitboard[FIELD_HEIGHT]; /* Represents static Hidamaries */
};

struct ai_state {
	bool active;
	void *region;
	Button const *planstr;
	u8 skill;
};

struct hidamari {
	enum game_state state;
	struct field field;
	struct ai_state ai;
};

/* Update the field by one timestep:
 *	Perform the player action;
 *	Move current piece downwards;
 *	Clear any rows;
 *	Update the score.
 *
 * This is the only function needed to run the game after initialization.
 */
void
hidamari_update(struct hidamari *game, Button act);

void
hidamari_render(struct drawbuf *buf, struct hidamari *game);

#endif
