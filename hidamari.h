/* See LICENSE file for copyright and license details */
#define HIDAMARI_WIDTH 12
#define HIDAMARI_HEIGHT 22 
#define HIDAMARI_HEIGHT_VISIBLE (HIDAMARI_HEIGHT - 2)
#define HIDAMARI_HEIGHT_INVISIBLE (HIDAMARI_HEIGHT - HIDAMARI_HEIGHT_VISIBLE)

#define HIDAMARI_BUFFER_WIDTH HIDAMARI_WIDTH
#define HIDAMARI_BUFFER_HEIGHT (HIDAMARI_HEIGHT_VISIBLE + 8)
typedef uint8_t HidamariBuffer[HIDAMARI_BUFFER_WIDTH][HIDAMARI_BUFFER_HEIGHT];

#define HIDAMARI_FLAG_MASK 240  /* 1111 0000 */
typedef enum {
	HIDAMARI_GHOST = 1 << 4, /* 0001 0000 */
} HidamariFlag;

#define HIDAMARI_SHAPE_MASK 15  /* 0000 1111 */
typedef enum {
	HIDAMARI_NONE = 0, /* 0000 0000 */
	HIDAMARI_I,        /* 0000 0001 */
	HIDAMARI_J,        /* 0000 0010 */
	HIDAMARI_L,        /* 0000 0011 */
	HIDAMARI_O,        /* 0000 0100 */
	HIDAMARI_S,        /* 0000 0101 */
	HIDAMARI_T,        /* 0000 0110 */
	HIDAMARI_Z,        /* 0000 0111 */
	HIDAMARI_WALL,     /* 0000 1000 */
	HIDAMARI_LAST, /* Not an actual piece, just used for enum length */
} HidamariShape;

typedef enum {
	ACTION_NONE = 0,
	ACTION_MV_D,
	ACTION_MV_R,
	ACTION_MV_L,
	ACTION_ROT_R,
	ACTION_ROT_L,
	ACTION_HARD_DROP,
} Action;

typedef enum {
	ROTATION_U = 0,
	ROTATION_R,
	ROTATION_D,
	ROTATION_L,
} Rotation;

typedef struct {
	int x, y; /* Position of the matrix in space */
	uint8_t mlen; /* Length of each side of the matrix */
	uint8_t matrix[4][4]; /* Matrix of current hidamari */
} Hidamari;

typedef struct {
	/* Scoring */
	size_t level;
	size_t score;
	size_t lines;
	/* Timing */
	float gravity_timer;
	uint8_t slide_timer; /* Counts ticks for hidamari sliding */
	uint8_t bag_pos; /* Current position in the bag */
	HidamariShape bag[7]; /* Current random bag, used for generating next pieces */
	HidamariShape next[1]; /* Lookahead piece for player */
	Hidamari current; /* Current piece information */
	/* The grid uses a mailbox representation, with the outer edges being
	 * permanently frozen wall pieces */
	uint8_t grid[HIDAMARI_WIDTH][HIDAMARI_HEIGHT];
} Playfield;

/* Initialize the playfield. Can be called as many times as you want */
void
hidamari_init(HidamariBuffer buf, Playfield *field);

/* Update the playfield by one timestep, perform the player's action,
 * move the current piece downwards, clear any rows, and
 * add score to the counter.
 * This is the main public function for running the game.
 */
void
hidamari_update(HidamariBuffer buf, Playfield *field, Action act);
