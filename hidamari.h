/* See LICENSE file for copyright and license details */
#ifndef HIDAMARI_H
#define HIDAMARI_H

#include <stdint.h>

#include "type.h"

#define HIDAMARI_HEIGHT 20
#define HIDAMARI_WIDTH 10

#define HIDAMARI_BUFFER_HEIGHT HIDAMARI_HEIGHT + 10
#define HIDAMARI_BUFFER_WIDTH HIDAMARI_WIDTH + 2

typedef struct HidamariPlayField HidamariPlayField;

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
	uint8_t color[HIDAMARI_BUFFER_WIDTH][HIDAMARI_BUFFER_HEIGHT][3];
} HidamariBuffer;

typedef struct {
	Vec2 pos; /* Bottom-left position */
	HidamariShape shape : 4;
	u8 orientation : 3;
} Hidamari;

struct HidamariPlayField {
	/* Scoring */
	u8 level;
	u32 score; 
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
	u10 grid[HIDAMARI_HEIGHT + 2]; /* Represents static Hidamaries */
};

typedef struct {
	HidamariPlayField field;
} HidamariGame;

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
		{VEC2(0, 1), VEC2(0, 2), VEC2(1, 1), VEC2(2, 1)},
		/* - J J 
		   - J -
		   - J -*/
		{VEC2(1, 0), VEC2(1, 1), VEC2(1, 2), VEC2(2, 2)},
		/* - - - 
		   J J J 
		   - - J */
		{VEC2(0, 1), VEC2(1, 1), VEC2(2, 0), VEC2(2, 1)},
		/* - J - 
		   - J - 
		   J J - */
		{VEC2(0, 0), VEC2(1, 0), VEC2(1, 1), VEC2(1, 2)},
	},
	{ /* 'L' */
		/* - - L 
		   L L L 
		   - - - */
		{VEC2(0, 1), VEC2(1, 1), VEC2(2, 1), VEC2(2, 2)},
		/* - L - 
		   - L - 
		   - L L*/
		{VEC2(1, 2), VEC2(2, 0), VEC2(2, 1), VEC2(2, 2)},
		/* - - -
		   L L L
		   L - - */
		{VEC2(0, 1), VEC2(0, 2), VEC2(1, 2), VEC2(2, 2)},
		/* L L - 
		   - L - 
		   - L - */
		{VEC2(0, 2), VEC2(1, 0), VEC2(1, 1), VEC2(1, 2)},
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
		{VEC2(0, 1), VEC2(1, 1), VEC2(1, 2), VEC2(2, 2)},
		/* - S - 
		   - S S 
		   - - S*/
		{VEC2(1, 1), VEC2(1, 2), VEC2(2, 0), VEC2(2, 1)},
		/* - - -
		   - S S 
		   S S - */
		{VEC2(0, 0), VEC2(1, 0), VEC2(1, 1), VEC2(2, 1)},
		/* S - - 
		   S S - 
		   - S - */
		{VEC2(0, 1), VEC2(0, 2), VEC2(1, 0), VEC2(1, 1)},
	},
	{ /* 'T' */
		/* - T - 
		   T T T 
		   - - - */
		{VEC2(0, 1), VEC2(1, 1), VEC2(1, 2), VEC2(2, 1)},
		/* - T - 
		   - T T 
		   - T -*/
		{VEC2(1, 0), VEC2(1, 1), VEC2(1, 2), VEC2(2, 1)},
		/* - - -
		   T T T 
		   - T - */
		{VEC2(0, 1), VEC2(1, 0), VEC2(1, 1), VEC2(2, 1)},
		/* - T - 
		   T T - 
		   - T - */
		{VEC2(0, 1), VEC2(1, 0), VEC2(1, 1), VEC2(1, 2)},
	},
	{ /* 'Z' */
		/* Z Z - 
		   - Z Z 
		   - - - */
		{VEC2(0, 2), VEC2(1, 1), VEC2(1, 2), VEC2(2, 1)},
		/* - - Z 
		   - Z Z 
		   - Z -*/
		{VEC2(1, 0), VEC2(1, 1), VEC2(2, 1), VEC2(2, 2)},
		/* - - -
		   Z Z - 
		   - Z Z*/
		{VEC2(0, 1), VEC2(1, 0), VEC2(1, 1), VEC2(2, 0)},
		/* - Z - 
		   Z Z - 
		   Z - - */
		{VEC2(0, 0), VEC2(0, 1), VEC2(1, 1), VEC2(1, 2)},
	},
};

/* Initialize the playfield. Is pseudo-idempotent, the randomized portions
 * are always different. */
void
hidamari_init(HidamariBuffer *buf, HidamariGame *field);

/* Update the playfield by one timestep:
 *	Perform the player action;
 *	Move current piece downwards;
 *	Clear any rows;
 *	Update the score.
 *
 * This is the only function needed to run the game after initialization.
 */
void
hidamari_update(HidamariBuffer *buf, HidamariGame *field, Button act);

#endif
