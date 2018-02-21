/* See LICENSE file for copyright and license details */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "hidamari.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static double const drop_time = 1.0;
static uint8_t const slide_time = 15;

static void
r7system(HidamariShape bag[7]);

/* Gravity of the falling piece at certain levels */
static float gravity_level[15] = {
	0.01667,
	0.021017,
	0.026977,
	0.035256,
	0.04693,
	0.06361,
	0.0879,
	0.1236,
	0.1775,
	0.2598,
	0.388,
	0.59,
	0.92,
	1.46,
	2.36
};

/* Character representations of each hidamari */
static char const hidamari_shape_char[HIDAMARI_LAST] =
{
	' ',
	'I',
	'J',
	'L',
	'O',
	'S',
	'T',
	'Z',
	'#',
};

/* Matrix size of each hidamari */
static uint8_t const hidamari_shape_mlen[HIDAMARI_LAST] =
{
	0,
	4,
	3,
	3,
	4,
	3,
	3,
	3,
	0, /* Walls should never be falling */
};


/* Initial orientation and structure of hidamaries */
static uint8_t const hidamari_shape_init[HIDAMARI_LAST][4][4] = {
	{
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE},
	},
	{
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_I, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_I, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_I, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_I, HIDAMARI_NONE},
	},
	{
		{HIDAMARI_NONE, HIDAMARI_J, HIDAMARI_J, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_J, HIDAMARI_NONE, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_J, HIDAMARI_NONE, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE},
	},
	{
		{HIDAMARI_NONE, HIDAMARI_L, HIDAMARI_NONE, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_L, HIDAMARI_NONE, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_L, HIDAMARI_L, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE},
	},
	{
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_O, HIDAMARI_O, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_O, HIDAMARI_O, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE},
	},
	{
		{HIDAMARI_NONE, HIDAMARI_S, HIDAMARI_NONE, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_S, HIDAMARI_S, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_S, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE},
	},
	{
		{HIDAMARI_NONE, HIDAMARI_T, HIDAMARI_NONE, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_T, HIDAMARI_T, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_T, HIDAMARI_NONE, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE},
	},
	{
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_Z, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_Z, HIDAMARI_Z, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_Z, HIDAMARI_NONE, HIDAMARI_NONE},
		{HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE, HIDAMARI_NONE},
	},
};

/* Shift all lines above a certain y value down by one */
static void
shift_lines(Playfield *field, int y_start)
{
	int x, y;

	for (y = y_start; y < HIDAMARI_HEIGHT - 1; ++y) {
		for (x = 1; x < HIDAMARI_WIDTH - 1; ++x) {
			field->grid[x][y] = field->grid[x][y + 1];
		}
	}
}

/* Clear any lines, and shift down hidamaries on the grid */
static void
clear_lines(Playfield *field)
{
	int lines = 0;
	int x, y;

	for (y = 1; y < HIDAMARI_HEIGHT - 2; ++y) {
		for (x = 1; x < HIDAMARI_WIDTH - 1; ++x) {
			if (HIDAMARI_NONE == field->grid[x][y])
				break;
		}
		if (x == HIDAMARI_WIDTH - 1) {
			for (x = 1; x < HIDAMARI_WIDTH - 1; ++x) {
				field->grid[x][y] = HIDAMARI_NONE;
			}
			shift_lines(field, y);
			lines += 1;
			--y;
		}
	}
	field->lines += lines;
	switch (lines) {
	case 0:
		break;
	case 1:
		field->score += 100;
		break;
	case 2:
		field->score += 300;
		break;
	case 3:
		field->score += 500;
		break;
	case 4:
		field->score += 800;
		break;
	default:
		field->score += 800;
		break;
	}
}

/* Get a new hidamari into play, and update the next hidamari */
static void
get_next_hidamari(Playfield *field)
{
	int i, j;
	HidamariShape shape = field->next[0];

	if (field->bag_pos >= 7) {
		r7system(field->bag);
		field->bag_pos = 0;
	}
	field->next[0] = field->bag[field->bag_pos];
	field->bag_pos += 1;
	field->current.mlen = hidamari_shape_mlen[shape];
	for (i = 0; i < field->current.mlen; ++i) {
		for (j = 0; j < field->current.mlen; ++j) {
			field->current.matrix[i][j] = hidamari_shape_init[shape][i][j];
		}
	}
	field->current.x = HIDAMARI_WIDTH / 2 - field->current.mlen % 2;
	field->current.y = HIDAMARI_HEIGHT - field->current.mlen / 2 - field->current.mlen % 2 + 1;
}

/* Check if the Hidamari would collide in the given grid, at the given x,y coordinates */
static bool
is_collision(int x, int y, Hidamari const *t, uint8_t grid[HIDAMARI_WIDTH][HIDAMARI_HEIGHT])
{
	int i, j;

	for (i = 0; i < t->mlen; ++i) {
		for (j = 0; j < t->mlen; ++j) {
			if (HIDAMARI_NONE != t->matrix[i][j]
			&& HIDAMARI_NONE != grid[x + i - t->mlen / 2][y + j - t->mlen / 2])
				return true;
		}
	}
	return false;
}

/* Lock the current piece in place by copying it to the static piece grid */
static void
lock_current(Playfield *field)
{
	int i, j;
	int const mlen = field->current.mlen;

	for (i = 0; i < mlen; ++i) {
		for (j = 0; j < mlen; ++j) {
			if (HIDAMARI_NONE != field->current.matrix[i][j])
				field->grid[field->current.x + i - mlen / 2]
					[field->current.y + j - mlen / 2] = field->current.matrix[i][j];
		}
	}
}

/* Move the current piece in the given direction */
static bool
move_current(Playfield *field, char dir)
{
	assert('d' == dir || 'r' == dir || 'l' == dir);

	int x, y;

	x = field->current.x;
	y = field->current.y;
	switch (dir) {
	case 'd': y -= 1; break;
	case 'r': x += 1; break;
	case 'l': x -= 1; break;
	}
	if (!is_collision(x, y, &field->current, field->grid)) {
		field->current.x = x;
		field->current.y = y;
		return true;
	} else {
		return false;
	}
}

/* Standard Tetris Random Hidamari generator. */
static void
r7system(HidamariShape bag[7])
{
	int i;
	int r1;
	int r2;
	HidamariShape tmp;

	bag[0] = HIDAMARI_I;
	bag[1] = HIDAMARI_J;
	bag[2] = HIDAMARI_L;
	bag[3] = HIDAMARI_O;
	bag[4] = HIDAMARI_S;
	bag[5] = HIDAMARI_T;
	bag[6] = HIDAMARI_Z;
	for (i = 0; i < 7; ++i) {
		r1 = rand() % 7;
		r2 = rand() % 7;
		tmp = bag[r1];
		bag[r1] = bag[r2];
		bag[r2] = tmp;
	}
}

/* Rotate a hidamari, saving its new orientation in 'dest' */
static void
rotate(Hidamari *dest, Hidamari *src, char dir)
{
	assert('l' == dir || 'r' == dir);

	int i, j;

	dest->mlen = src->mlen;
	switch (dir) {
	case 'l':
		for (i = 0; i < dest->mlen; ++i) {
			for (j = 0; j < dest->mlen; ++j) {
				dest->matrix[i][j] = src->matrix[dest->mlen - j - 1][i];
			}
		}
		break;
	case 'r':
		for (i = 0; i < dest->mlen; ++i) {
			for (j = 0; j < dest->mlen; ++j) {
				dest->matrix[i][j] = src->matrix[j][dest->mlen - i - 1];
			}
		}
		break;
	}
}
	
/* Rotate the current hidamari, and perform a wall kick if able */
static bool
rotate_current(Playfield *field, char dir)
{
	assert('l' == dir || 'r' == dir);

	int i, j;
	Hidamari tmp;

	rotate(&tmp, &field->current, dir);
	if (!is_collision(field->current.x, field->current.y, &tmp, field->grid)) {
		for (i = 0; i < tmp.mlen; ++i) {
			for (j = 0; j < tmp.mlen; ++j) {
				field->current.matrix[i][j] = tmp.matrix[i][j];
			}
		}
		return true;
	} else {
		return false;
	}
}


static void
init_buffer(HidamariBuffer buf) {
	size_t i, j;

	for (i = 0; i < HIDAMARI_BUFFER_WIDTH; ++i) {
		for (j = 0; j < HIDAMARI_BUFFER_HEIGHT; ++j) {
			buf[i][j] = HIDAMARI_NONE;
		}
	}

	for (i = 0; i < HIDAMARI_WIDTH; ++i) {
		buf[i][HIDAMARI_HEIGHT_VISIBLE] = HIDAMARI_WALL;
		buf[i][HIDAMARI_HEIGHT_VISIBLE + 5] = HIDAMARI_WALL;
		buf[i][HIDAMARI_HEIGHT_VISIBLE + 7] = HIDAMARI_WALL;
	}
	
	for (i = 0; i < 8; ++i) {
		buf[0][HIDAMARI_HEIGHT_VISIBLE + i] = HIDAMARI_WALL;
		buf[HIDAMARI_WIDTH - 1][HIDAMARI_HEIGHT_VISIBLE + i] = HIDAMARI_WALL;
	}
}

/* Update the game buffer */
static void
update_buffer(HidamariBuffer buf, Playfield *field)
{
	int i, j;
	int x, y;
	int mlen;

	/* Copy the static hidamaries */
	for (i = 0; i < HIDAMARI_WIDTH; ++i) {
		for (j = 0; j < HIDAMARI_HEIGHT_VISIBLE; ++j) {
			buf[i][j] = field->grid[i][j];
		}
	}

	mlen = hidamari_shape_mlen[field->next[0]];
	/* Copy over the next hidamari preview */
	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 4; ++j) {
			if (i < mlen ||  j < mlen) {
				buf[i + HIDAMARI_WIDTH / 2 - mlen / 2 - mlen % 2]
					[j + HIDAMARI_HEIGHT_VISIBLE + 1] = hidamari_shape_init[field->next[0]][i][j];
			} else {
				buf[i + HIDAMARI_WIDTH / 2 - mlen / 2 - mlen % 2]
					[j + HIDAMARI_HEIGHT_VISIBLE + 1] = HIDAMARI_NONE;
			}
		}
	}

	/* Insert the ghost piece */
	x = field->current.x;
	y = field->current.y;
	while (!is_collision(x, y - 1, &field->current, field->grid)) {
		--y;
	}
	mlen = field->current.mlen;
	for (i = 0; i < mlen; ++i) {
		for (j = 0; j < mlen; ++j) {
			if (HIDAMARI_NONE != field->current.matrix[i][j]
			&& y + j - mlen / 2 < HIDAMARI_HEIGHT_VISIBLE) {
				buf[x + i - mlen / 2][y + j - mlen / 2] = field->current.matrix[i][j] | HIDAMARI_GHOST;
			}
		}
	}

	/* Now copy over the current hidamari */
	x = field->current.x;
	y = field->current.y;
	for (i = 0; i < mlen; ++i) {
		for (j = 0; j < mlen; ++j) {
			if (HIDAMARI_NONE != field->current.matrix[i][j]
			&& y + j - mlen / 2 < HIDAMARI_HEIGHT_VISIBLE) {
				buf[x + i - mlen / 2][y + j - mlen / 2] = field->current.matrix[i][j];
			}
		}
	}

}

void
hidamari_init(HidamariBuffer buf, Playfield *field)
{
	int i;

	memset(field, 0, sizeof(*field));
	/* Set the borders */
	for (i = 0; i < HIDAMARI_HEIGHT; ++i) {
		field->grid[0][i] = HIDAMARI_WALL;
		field->grid[HIDAMARI_WIDTH - 1][i] = HIDAMARI_WALL;
	}
	for (i = 1; i < HIDAMARI_WIDTH - 1; ++i) {
		field->grid[i][0] = HIDAMARI_WALL;
	}
	r7system(field->bag);
	/* First hidamari must not be a s or z */
	do {
		field->next[0] = 1 + rand() % 7;
	} while (HIDAMARI_O == field->next[0]
	      || HIDAMARI_S == field->next[0]
	      || HIDAMARI_Z == field->next[0]);
	get_next_hidamari(field);
	/* Set the initial game speed */
	init_buffer(buf);
}

void
hidamari_update(HidamariBuffer buf, Playfield *field, Action act)
{
	size_t i;

	switch (act) {
	case ACTION_NONE:
		break;
	case ACTION_MV_D:
		move_current(field, 'd');
		break;
	case ACTION_MV_R:
		move_current(field, 'r');
		break;
	case ACTION_MV_L:
		move_current(field, 'l');
		break;
	case ACTION_ROT_R:
		rotate_current(field, 'r');
		break;
	case ACTION_ROT_L:
		rotate_current(field, 'l');
		break;
	case ACTION_HARD_DROP:
		while (move_current(field, 'd'))
			;
		field->slide_timer = slide_time;
		field->gravity_timer = drop_time;
		break;
	}
	field->gravity_timer += gravity_level[field->level / 10];
	/* If its not time for an update, then return early */
	if (field->gravity_timer >= drop_time) {
		if (is_collision(field->current.x,
					field->current.y - 1,
					&field->current,
					field->grid)) {
			if (field->slide_timer >= slide_time) {
				lock_current(field);
				clear_lines(field);
				get_next_hidamari(field);
				field->slide_timer = 0;
			} else {
				field->slide_timer += 1;
			}
		} else {
			move_current(field, 'd');
			field->gravity_timer = MAX(0.0, field->gravity_timer - drop_time);
		}
		/* Check for game over */
		for (i = 1; i < HIDAMARI_WIDTH - 1; ++i) {
			if (HIDAMARI_NONE != field->grid[i][HIDAMARI_HEIGHT_VISIBLE]) {
				printf("lines: %zu\n", field->lines);
				printf("score: %zu\n", field->score);
				hidamari_init(buf, field);
				break;
			}
		}
	}
	/* Update the buffer for rendering */
	update_buffer(buf, field);
}
