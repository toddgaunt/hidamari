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

#define HIDAMARI_HEIGHT 23
#define HIDAMARI_WIDTH 12

#define HIDAMARI_HEIGHT_VISIBLE (HIDAMARI_HEIGHT - 3)
#define HIDAMARI_WIDTH_VISIBLE (HIDAMARI_WIDTH)

#define HIDAMARI_BUFFER_HEIGHT (HIDAMARI_HEIGHT_VISIBLE + 3 + 3)
#define HIDAMARI_BUFFER_WIDTH HIDAMARI_WIDTH * 2

/* Cursor index */
#define HIDAMARI_MAIN_CURSOR 0
#define HIDAMARI_OPTION_CURSOR 1

typedef uint8_t Button;
typedef struct HidamariMenu HidamariMenu;

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
	TILE_0,
	TILE_1,
	TILE_2,
	TILE_3,
	TILE_4,
	TILE_5,
	TILE_6,
	TILE_7,
	TILE_8,
	TILE_9,
	TILE_PLACEHOLDER1,
	TILE_PLACEHOLDER2,
	TILE_PLACEHOLDER3,
	TILE_PLACEHOLDER4,
	TILE_PLACEHOLDER5,
	TILE_PLACEHOLDER6,
	TILE_SPACE,
	TILE_I,
	TILE_J,
	TILE_L,
	TILE_O,
	TILE_S,
	TILE_T,
	TILE_Z,
	TILE_FALLEN,
	TILE_WALL,
	TILE_PLAIN,
	TILE_PLACEHOLDER7,
	TILE_PLACEHOLDER8,
	TILE_PLACEHOLDER9,
	TILE_PLACEHOLDER10,
	TILE_PLACEHOLDER11,
	TILE_CHAR_A,
	TILE_CHAR_B,
	TILE_CHAR_C,
	TILE_CHAR_D,
	TILE_CHAR_E,
	TILE_CHAR_F,
	TILE_CHAR_G,
	TILE_CHAR_H,
	TILE_CHAR_I,
	TILE_CHAR_J,
	TILE_CHAR_K,
	TILE_CHAR_L,
	TILE_CHAR_M,
	TILE_CHAR_N,
	TILE_CHAR_O,
	TILE_CHAR_P,
	TILE_CHAR_Q,
	TILE_CHAR_R,
	TILE_CHAR_S,
	TILE_CHAR_T,
	TILE_CHAR_U,
	TILE_CHAR_V,
	TILE_CHAR_W,
	TILE_CHAR_X,
	TILE_CHAR_Y,
	TILE_CHAR_Z,
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
	int x;
	int y;
	enum shape shape : 4;
	u8 orientation : 3;
};

struct playfield {
	/* Scoring */
	u8 level;
	u32 score; 
	u32 lines; 
	/* Timing */
	f32 gravity_timer; 
	u8 slide_timer : 4;
	/* Randomization */
	u4 bag_pos : 4; /* Current position in the bag */
	enum shape bag[7]; /* Random Bag, used for pseudo-random order */
	/* Hidamaries */
	enum shape next : 4; /* Lookahead piece for player */
	struct piece current;
	u12 grid[HIDAMARI_HEIGHT]; /* Represents static Hidamaries */
};

struct ai_state {
	bool active;
	void *region;
	Button const *planstr;
	uint8_t skill;
};

struct hidamari {
	enum game_state state;
	uint8_t cursor[2];
	struct playfield field;
	struct ai_state ai;
};

/* Update the playfield by one timestep:
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
