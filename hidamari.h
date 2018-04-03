/* See LICENSE file for copyright and license details */
#ifndef HIDAMARI_H
#define HIDAMARI_H

#include <stdint.h>

#include "type.h"

#define HIDAMARI_HEIGHT 23
#define HIDAMARI_WIDTH 12

#define HIDAMARI_BUFFER_HEIGHT (HIDAMARI_HEIGHT - 2)
#define HIDAMARI_BUFFER_WIDTH HIDAMARI_WIDTH

typedef enum {
	HIDAMARI_TILE_0,
	HIDAMARI_TILE_1,
	HIDAMARI_TILE_2,
	HIDAMARI_TILE_3,
	HIDAMARI_TILE_4,
	HIDAMARI_TILE_5,
	HIDAMARI_TILE_6,
	HIDAMARI_TILE_7,
	HIDAMARI_TILE_8,
	HIDAMARI_TILE_9,
	HIDAMARI_TILE_PLACEHOLDER1,
	HIDAMARI_TILE_PLACEHOLDER2,
	HIDAMARI_TILE_PLACEHOLDER3,
	HIDAMARI_TILE_PLACEHOLDER4,
	HIDAMARI_TILE_PLACEHOLDER5,
	HIDAMARI_TILE_PLACEHOLDER6,
	HIDAMARI_TILE_SPACE,
	HIDAMARI_TILE_I,
	HIDAMARI_TILE_J,
	HIDAMARI_TILE_L,
	HIDAMARI_TILE_O,
	HIDAMARI_TILE_S,
	HIDAMARI_TILE_T,
	HIDAMARI_TILE_Z,
	HIDAMARI_TILE_WALL,
	HIDAMARI_TILE_LAST, /* Not an actual tile, just used for enum length */
} HidamariTile;

typedef enum {
	BUTTON_NONE = 0,
	/* D-pad */
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_RIGHT,
	BUTTON_LEFT,
	/* Shoulder buttons */
	BUTTON_R,
	BUTTON_L,
	/* Face buttons */
	BUTTON_A,
	BUTTON_B,
	BUTTON_X,
	BUTTON_Y,
	BUTTON_LAST, /* Not an actual action, just used for enum length */
} Button;

typedef enum {
	HIDAMARI_I,        
	HIDAMARI_J,        
	HIDAMARI_L,        
	HIDAMARI_O,        
	HIDAMARI_S,        
	HIDAMARI_T,        
	HIDAMARI_Z,        
	HIDAMARI_LAST, /* Not an actual piece, just used for enum length */
} HidamariShape;

typedef struct {
	char tile[HIDAMARI_BUFFER_WIDTH][HIDAMARI_BUFFER_HEIGHT];
	u8 color[HIDAMARI_BUFFER_WIDTH][HIDAMARI_BUFFER_HEIGHT][3];
} HidamariBuffer;

typedef struct {
	Vec2 pos; /* Top-left position */
	HidamariShape shape : 4;
	u8 orientation : 3;
} Hidamari;

typedef struct {
	/* Scoring */
	u8 level;
	u16 score; 
	u16 lines; 
	/* Timing */
	f32 gravity_timer; 
	u8 slide_timer;
	/* Randomization */
	u4 bag_pos : 4; /* Current position in the bag */
	HidamariShape bag[7]; /* Random Bag, used for pseudo-random order */
	/* Hidamaries */
	HidamariShape next : 4; /* Lookahead piece for player */
	Hidamari current;
	u12 grid[HIDAMARI_HEIGHT]; /* Represents static Hidamaries */
} HidamariPlayField;

/* State that is saved for the renderer */
typedef struct {
	u8 level;
	u16 score;
	HidamariShape next : 4;
	Hidamari current;
} HidamariState;

typedef struct {
	HidamariState s1;
	HidamariState s2;
	HidamariBuffer buf;
	HidamariPlayField field;
} HidamariGame;

/* Note that for these coordinates, y starts at the top, not bottom.
 * The vectors are organized as (x, y) pairs */
static Vec2 const hidamari_orientation[HIDAMARI_LAST][4][4] = {
	{ /* 'I' */
		/* - - - -
		   I I I I 
		   - - - - 
		   - - - - */
		{VEC2(0, 1), VEC2(1, 1), VEC2(2, 1), VEC2(3, 1)},
		/* - - I -
		   - - I - 
		   - - I - 
		   - - I - */
		{VEC2(2, 0), VEC2(2, 1), VEC2(2, 2), VEC2(2, 3)},
		/* - - - -
		   - - - - 
		   I I I I 
		   - - - - */
		{VEC2(0, 2), VEC2(1, 2), VEC2(2, 2), VEC2(3, 2)},
		/* - I - -
		   - I - - 
		   - I - - 
		   - I - - */
		{VEC2(1, 0), VEC2(1, 1), VEC2(1, 2), VEC2(1, 3)},
	}, 
	{ /* 'J' */
		/* J - - 
		   J J J 
		   - - -*/
		{VEC2(0, 0), VEC2(0, 1), VEC2(1, 1), VEC2(2, 1)},
		/* - J J 
		   - J -
		   - J -*/
		{VEC2(1, 0), VEC2(1, 1), VEC2(1, 2), VEC2(2, 0)},
		/* - - - 
		   J J J 
		   - - J */
		{VEC2(0, 1), VEC2(1, 1), VEC2(2, 1), VEC2(2, 2)},
		/* - J - 
		   - J - 
		   J J - */
		{VEC2(0, 2), VEC2(1, 0), VEC2(1, 1), VEC2(1, 2)},
	},
	{ /* 'L' */
		/* - - L 
		   L L L 
		   - - - */
		{VEC2(0, 1), VEC2(1, 1), VEC2(2, 0), VEC2(2, 1)},
		/* - L - 
		   - L - 
		   - L L*/
		{VEC2(1, 0), VEC2(1, 1), VEC2(1, 2), VEC2(2, 2)},
		/* - - -
		   L L L
		   L - - */
		{VEC2(0, 1), VEC2(0, 2), VEC2(1, 1), VEC2(2, 1)},
		/* L L - 
		   - L - 
		   - L - */
		{VEC2(0, 0), VEC2(1, 0), VEC2(1, 1), VEC2(1, 2)},
	},
	{ /* 'O' */
		/* - - - -
		   - O O - 
		   - O O - 
		   - - - - */
		{VEC2(1, 1), VEC2(1, 2), VEC2(2, 1), VEC2(2, 2)},
		{VEC2(1, 1), VEC2(1, 2), VEC2(2, 1), VEC2(2, 2)},
		{VEC2(1, 1), VEC2(1, 2), VEC2(2, 1), VEC2(2, 2)},
		{VEC2(1, 1), VEC2(1, 2), VEC2(2, 1), VEC2(2, 2)},
	}, 
	{ /* 'S' */
		/* - S S 
		   S S - 
		   - - - */
		{VEC2(0, 1), VEC2(1, 0), VEC2(1, 1), VEC2(2, 0)},
		/* - S - 
		   - S S 
		   - - S*/
		{VEC2(1, 0), VEC2(1, 1), VEC2(2, 1), VEC2(2, 2)},
		/* - - -
		   - S S 
		   S S - */
		{VEC2(0, 2), VEC2(1, 1), VEC2(1, 2), VEC2(2, 1)},
		/* S - - 
		   S S - 
		   - S - */
		{VEC2(0, 0), VEC2(0, 1), VEC2(1, 1), VEC2(1, 2)},
	},
	{ /* 'T' */
		/* - T - 
		   T T T 
		   - - - */
		{VEC2(0, 1), VEC2(1, 0), VEC2(1, 1), VEC2(2, 1)},
		/* - T - 
		   - T T 
		   - T -*/
		{VEC2(1, 0), VEC2(1, 1), VEC2(1, 2), VEC2(2, 1)},
		/* - - -
		   T T T 
		   - T - */
		{VEC2(0, 1), VEC2(1, 1), VEC2(1, 2), VEC2(2, 1)},
		/* - T - 
		   T T - 
		   - T - */
		{VEC2(0, 1), VEC2(1, 0), VEC2(1, 1), VEC2(1, 2)},
	},
	{ /* 'Z' */
		/* Z Z - 
		   - Z Z 
		   - - - */
		{VEC2(0, 0), VEC2(1, 0), VEC2(1, 1), VEC2(2, 1)},
		/* - - Z 
		   - Z Z 
		   - Z -*/
		{VEC2(1, 1), VEC2(1, 2), VEC2(2, 0), VEC2(2, 1)},
		/* - - -
		   Z Z - 
		   - Z Z*/
		{VEC2(0, 1), VEC2(1, 1), VEC2(1, 2), VEC2(2, 2)},
		/* - Z - 
		   Z Z - 
		   Z - - */
		{VEC2(0, 1), VEC2(0, 2), VEC2(1, 0), VEC2(1, 1)},
	},
};

/* Initialize the playfield, and allocate the global region used by all
 * games. Pseudo-idempotent. */
void
hidamari_init(HidamariGame *game);

/* Update the playfield by one timestep:
 *	Perform the player action;
 *	Move current piece downwards;
 *	Clear any rows;
 *	Update the score.
 *
 * This is the only function needed to run the game after initialization.
 */
void
hidamari_update(HidamariGame *game, Button act);

void
hidamari_state_save(HidamariGame *game, u1 slot);

#endif
