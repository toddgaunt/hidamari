/* See LICENSE file for copyright and license details */
#ifndef HIDAMARI_H
#define HIDAMARI_H

#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

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

#define ASCII_OFFSET (HIDAMARI_TILE_CHAR_A+1)

typedef uint8_t Button;
typedef uint8_t HidamariTile;
typedef uint8_t HidamariShape;
typedef uint8_t HidamariGameState;
typedef struct Hidamari Hidamari;
typedef struct HidamariGame HidamariGame;
typedef struct HidamariPlayField HidamariPlayField;
typedef struct HidamariAIState HidamariAIState;
typedef struct HidamariBuffer HidamariBuffer;
typedef struct HidamariMenu HidamariMenu;

enum {
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
	BUTTON_B,
	BUTTON_A,
	BUTTON_Y,
	BUTTON_X,
	BUTTON_LAST, /* Not an actual action, just used for enum length */
};

enum {
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
	HIDAMARI_TILE_FALLEN,
	HIDAMARI_TILE_WALL,
	HIDAMARI_TILE_PLAIN,
	HIDAMARI_TILE_PLACEHOLDER7,
	HIDAMARI_TILE_PLACEHOLDER8,
	HIDAMARI_TILE_PLACEHOLDER9,
	HIDAMARI_TILE_PLACEHOLDER10,
	HIDAMARI_TILE_PLACEHOLDER11,
	HIDAMARI_TILE_CHAR_A,
	HIDAMARI_TILE_CHAR_B,
	HIDAMARI_TILE_CHAR_C,
	HIDAMARI_TILE_CHAR_D,
	HIDAMARI_TILE_CHAR_E,
	HIDAMARI_TILE_CHAR_F,
	HIDAMARI_TILE_CHAR_G,
	HIDAMARI_TILE_CHAR_H,
	HIDAMARI_TILE_CHAR_I,
	HIDAMARI_TILE_CHAR_J,
	HIDAMARI_TILE_CHAR_K,
	HIDAMARI_TILE_CHAR_L,
	HIDAMARI_TILE_CHAR_M,
	HIDAMARI_TILE_CHAR_N,
	HIDAMARI_TILE_CHAR_O,
	HIDAMARI_TILE_CHAR_P,
	HIDAMARI_TILE_CHAR_Q,
	HIDAMARI_TILE_CHAR_R,
	HIDAMARI_TILE_CHAR_S,
	HIDAMARI_TILE_CHAR_T,
	HIDAMARI_TILE_CHAR_U,
	HIDAMARI_TILE_CHAR_V,
	HIDAMARI_TILE_CHAR_W,
	HIDAMARI_TILE_CHAR_X,
	HIDAMARI_TILE_CHAR_Y,
	HIDAMARI_TILE_CHAR_Z,
	HIDAMARI_TILE_LAST, /* Not an actual tile, just used for enum length */
};

enum {
	HIDAMARI_I,
	HIDAMARI_J,
	HIDAMARI_L,
	HIDAMARI_O,
	HIDAMARI_S,
	HIDAMARI_T,
	HIDAMARI_Z,
	HIDAMARI_LAST, /* Not an actual piece, just used for enum length. Also
			  can be used as a NULL value for hidamaris */
};

/* Current state of the game */
enum {
	HIDAMARI_GS_MAIN_MENU,
	HIDAMARI_GS_OPTION_MENU,
	HIDAMARI_GS_GAME_PLAYING,
	HIDAMARI_GS_GAME_OVER,
};

enum {
	HIDAMARI_AI_POOR,
	HIDAMARI_AI_NORMAL,
	HIDAMARI_AI_SKILLED,
	HIDAMARI_AI_GODLIKE,
	HIDAMARI_AI_LAST,
};

struct HidamariBuffer {
	HidamariTile tile[HIDAMARI_BUFFER_WIDTH][HIDAMARI_BUFFER_HEIGHT];
	u8 color[HIDAMARI_BUFFER_WIDTH][HIDAMARI_BUFFER_HEIGHT][3];
	bool highlight[HIDAMARI_BUFFER_WIDTH][HIDAMARI_BUFFER_HEIGHT];
};

struct Hidamari {
	Vec2 pos; /* Top-left position */
	HidamariShape shape : 4;
	u8 orientation : 3;
};

struct HidamariPlayField {
	/* Scoring */
	u8 level;
	u32 score;
	u32 lines;
	/* Timing */
	f32 gravity_timer;
	u8 slide_timer : 4;
	/* Randomization */
	u4 bag_pos : 4; /* Current position in the bag */
	HidamariShape bag[7]; /* Random Bag, used for pseudo-random order */
	/* Hidamaries */
	HidamariShape next : 4; /* Lookahead piece for player */
	Hidamari current;
	u12 grid[HIDAMARI_HEIGHT]; /* Represents static Hidamaries */
};

struct HidamariAIState {
	bool active;
	void *region;
	Button const *planstr;
	uint8_t skill;
};

struct HidamariGame {
	HidamariGameState state;
	HidamariBuffer buf;
	uint8_t cursor[2];
	HidamariPlayField field;
	HidamariAIState ai;
};

/* Initialize the playfield, and allocate the global region used by all
 * games. Also start the AI-thread. */
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

/* An alternative update function with no visuals for particle-swarm
 * optimization, or simulation without the overhead of visualization.
 * The weights of the AI heuristics can be provided. */
void
hidamari_pso_update(HidamariGame *game, double weight[3]);

#endif
