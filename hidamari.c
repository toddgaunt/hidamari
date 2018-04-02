/* See LICENSE file for copyright and license details */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "heap.h"
#include "hidamari.h"
#include "ralloc.h"
#include "region.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static f32 const drop_time = 1.0;
static u8 const slide_time = 15;

static void
r7system(HidamariShape bag[7]);
static void
field_init(HidamariPlayField *field);
static int
field_update(HidamariPlayField *field, Button act);


static size_t
field_hash(HidamariPlayField *a)
{
	return 0;
}

static int
field_cmp(HidamariPlayField *a, HidamariPlayField *b)
{
	return 0;
}

HEAP_INSTANTIATE(heap, HidamariPlayField *, ralloc, field_cmp)
HASHTABLE_INSTANTIATE(ht, HidamariPlayField *,  bool, ralloc, field_hash,
		field_cmp)

/* Gravity of the falling piece at certain levels */
static f32 gravity_level[15] = {
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
	'I',
	'J',
	'L',
	'O',
	'S',
	'T',
	'Z',
};

static void
draw_field(HidamariBuffer *buf, HidamariPlayField *field)
{
	int i;
	size_t x, y;

	for (x = 0; x < HIDAMARI_WIDTH; ++x) {
		for (y = 0; y < HIDAMARI_BUFFER_HEIGHT; ++y) {
			if (field->grid[y] & 1 << x) {
				buf->tile[x][y] = HIDAMARI_TILE_I;
			} else {
				buf->tile[x][y] = HIDAMARI_TILE_SPACE;
			}
		}
	}
	for (i = 0; i < 4; ++i) {
		x = hidamari_orientation[field->current.shape]
		                              [field->current.orientation]
		                              [i].x + field->current.pos.x;
		y = field->current.pos.y
			- hidamari_orientation[field->current.shape]
		                              [field->current.orientation]
		                              [i].y;
		if (y >= HIDAMARI_BUFFER_HEIGHT)
			continue;
		buf->tile[x][y] = HIDAMARI_TILE_I;
	}
}

/* Shift all lines above a certain y value down by one */
static void
shift_lines(HidamariPlayField *field, size_t y_start)
{
	size_t y;

	for (y = y_start; y < HIDAMARI_HEIGHT - 1; ++y) {
		field->grid[y] = field->grid[y + 1];
	}
}

static void
clear_lines(HidamariPlayField *field)
{
	size_t y = 1;

	while (y < HIDAMARI_HEIGHT) {
		if (2046 == (field->grid[y] & 2046)) {
			shift_lines(field, y);
		} else {
			y += 1;
		}
	}
}

static bool
is_game_over(HidamariPlayField *field)
{
	if (field->grid[HIDAMARI_HEIGHT - 1] & 2046)
		return true;
	return false;
}

/* Get a new hidamari into play, and update the next hidamari */
static void
get_next_hidamari(HidamariPlayField *field)
{
	field->current.shape = field->next;
	field->current.orientation = 0;
	field->current.pos.x = 10 / 2 - 1;
	field->current.pos.y = HIDAMARI_HEIGHT;

	if (field->bag_pos >= 7) {
		r7system(field->bag);
		field->bag_pos = 0;
	}
	field->next = field->bag[field->bag_pos];
	field->bag_pos += 1;
}

/* Check if the Hidamari would collide in the given grid, at the given x,y coordinates */
static bool
is_collision(Hidamari const *t, u12 const grid[HIDAMARI_HEIGHT])
{
	int i;
	u12 x, y;

	for (i = 0; i < 4; ++i) {
		x = 1 << (hidamari_orientation[t->shape]
				[t->orientation][i].x + t->pos.x);
		y = t->pos.y - hidamari_orientation[t->shape]
		                                   [t->orientation]
		                                   [i].y;
		if (x & grid[y])
			return true;
	}
	return false;
}

/* Lock the current piece in place by copying it to the static piece grid */
static void
lock_current(HidamariPlayField *field)
{
	int i;
	u12 x, y;

	for (i = 0; i < 4; ++i) {
		x = 1 << (hidamari_orientation[field->current.shape]
		                              [field->current.orientation]
		                              [i].x + field->current.pos.x);
		y = field->current.pos.y
			- hidamari_orientation[field->current.shape]
		                              [field->current.orientation]
		                              [i].y;
		field->grid[y] |= x;
	}
}

/* Move the current piece in the given direction */
static bool
move_current(HidamariPlayField *field, Button dir)
{
	assert(BUTTON_DOWN == dir
	    || BUTTON_RIGHT == dir
	    || BUTTON_LEFT == dir);

	Hidamari tmp = field->current;

	switch (dir) {
	case BUTTON_DOWN: tmp.pos.y -= 1; break;
	case BUTTON_RIGHT: tmp.pos.x += 1; break;
	case BUTTON_LEFT: tmp.pos.x -= 1; break;
	default: break;
	}
	if (is_collision(&tmp, field->grid))
		return false;
	field->current = tmp;
	return true;
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
	
/* Rotate the current hidamari, and perform a wall kick if able */
static void
rotate_current(HidamariPlayField *field, Button dir)
{
	assert(BUTTON_R == dir || BUTTON_L == dir);

	Hidamari tmp = field->current;

	if (BUTTON_R == dir) {
		tmp.orientation = (tmp.orientation + 1) % 4;
	} else {
		tmp.orientation = MIN((u8)(tmp.orientation - 1), 3);
	}
	if (!is_collision(&tmp, field->grid))
		field->current = tmp;
}

static void
field_init(HidamariPlayField *field)
{
	size_t i;

	memset(field, 0, sizeof(*field));
	/* Initialize the random bag */
	r7system(field->bag);
	/* First hidamari must not be a S, Z, or O */
	do {
		field->next = rand() % 7;
	} while (HIDAMARI_O == field->next
	      || HIDAMARI_S == field->next
	      || HIDAMARI_Z == field->next);
	get_next_hidamari(field);
	/* Initialize the borders */
	field->grid[0] |= 4095;
	for (i = 1; i < HIDAMARI_HEIGHT; ++i) {
		field->grid[i] = 2049;
	}
}

static int
field_update(HidamariPlayField *field, Button act)
{
	switch (act) {
	case BUTTON_NONE:
		break;
	case BUTTON_DOWN:
	case BUTTON_RIGHT:
	case BUTTON_LEFT:
		move_current(field, act);
		break;
	case BUTTON_R:
	case BUTTON_L:
		rotate_current(field, act);
		break;
	case BUTTON_B:
		while (move_current(field, BUTTON_DOWN))
			;
		field->slide_timer = slide_time;
		field->gravity_timer = drop_time;
		break;
	default:
		/* Don't perform any action for an illegal action */
		break;
	}
	field->gravity_timer += gravity_level[field->level];
	/* If its not time for an update, then return early */
	if (field->gravity_timer >= drop_time) {
		if (move_current(field, BUTTON_DOWN)) {
			field->gravity_timer = MAX(0.0, field->gravity_timer
					- drop_time);
		} else {
			if (field->slide_timer < slide_time) {
				field->slide_timer += 1;
			} else {
				lock_current(field);
				clear_lines(field);
				get_next_hidamari(field);
				field->slide_timer = 0;
				if (is_game_over(field))
					return 1;
			}
		}
	}
	return 0;
}

void
save_state(HidamariState *ret, HidamariPlayField *field)
{
	ret->level = field->level;
	ret->score = field->score;
	ret->next = field->next;
	ret->current = field->current;
}

void
buffer_init(HidamariBuffer *buf)
{
	size_t x, y;

	for (x = 0; x < HIDAMARI_BUFFER_WIDTH; ++x) {
		for (y = 0; y < HIDAMARI_BUFFER_HEIGHT; ++y) {
			buf->tile[x][y] = HIDAMARI_TILE_SPACE;
		}
	}

	for (y = 0; y < HIDAMARI_BUFFER_HEIGHT; ++y) {
		buf->tile[0][y] = HIDAMARI_TILE_WALL;
		buf->tile[HIDAMARI_BUFFER_WIDTH - 1][y] = HIDAMARI_TILE_WALL;
	}

	for (x = 1; x < HIDAMARI_BUFFER_WIDTH - 1; ++x) {
		buf->tile[x][0] = HIDAMARI_TILE_WALL;
	}
}

void
buffer_update(HidamariBuffer *buf, HidamariState *s1, HidamariState *s2)
{
	
}

void
hidamari_init(HidamariGame *game)
{
	memset(game, 0, sizeof(*game));
	buffer_init(&game->buf);
	field_init(&game->field);
}

void
hidamari_update(HidamariGame *game, Button act)
{
	save_state(&game->s1, &game->field);
	field_update(&game->field, act);
	save_state(&game->s2, &game->field);
	buffer_update(&game->buf, &game->s1, &game->s2);
	draw_field(&game->buf, &game->field);
}
