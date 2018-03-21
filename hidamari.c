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

static void r7system(HidamariShape bag[7]);
static void field_init(HidamariBuffer *buf, HidamariPlayField *field);
static void field_update(HidamariBuffer *buf, HidamariPlayField *field, Button act);

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
	'I',
	'J',
	'L',
	'O',
	'S',
	'T',
	'Z',
};

static void
dump_field(HidamariPlayField *field)
{
	int i;
	size_t x, y;
	char buf[HIDAMARI_WIDTH][HIDAMARI_HEIGHT];

	for (x = 0; x < HIDAMARI_WIDTH; ++x) {
		for (y = 0; y < HIDAMARI_HEIGHT; ++y) {
			if (field->grid[y] & 1 << x) {
				buf[x][y] = '#';
			} else {
				buf[x][y] = '.';
			}
		}
	}
	printf("shape: %d\n", field->current.shape);
	for (i = 0; i < 4; ++i) {
		x = hidamari_orientation[field->current.shape]
		                              [field->current.orientation]
		                              [i].x + field->current.pos.x;
		y = hidamari_orientation[field->current.shape]
		                        [field->current.orientation]
		                        [i].y + field->current.pos.y;
		printf("x, y: %zu, %zu\n", x, y);
		buf[x][y] = '$';
	}
	printf("--\n");
	for (x = 0; x < HIDAMARI_WIDTH; ++x) {
		for (y = 0; y < HIDAMARI_HEIGHT; ++y) {
			putc(buf[x][y], stdout);
			putc(' ', stdout);
		}
		putc('\n', stdout);
	}
	printf("--\n");
}

/* Get a new hidamari into play, and update the next hidamari */
static void
get_next_hidamari(HidamariPlayField *field)
{
	field->current.shape = field->next;
	field->current.orientation = 0;
	field->current.pos.x = 10 / 2 - 2;
	field->current.pos.y = HIDAMARI_HEIGHT - 4;

	if (field->bag_pos >= 7) {
		r7system(field->bag);
		field->bag_pos = 0;
	}
	field->next = field->bag[field->bag_pos];
	field->bag_pos += 1;
}

/* Check if the Hidamari would collide in the given grid, at the given x,y coordinates */
static bool
is_collision(Hidamari const *t, u10 grid[HIDAMARI_HEIGHT])
{
	int i;
	u10 x, y;

	for (i = 0; i < 4; ++i) {
		x = 1 << (hidamari_orientation[t->shape][t->orientation][i].x + t->pos.x);
		y = hidamari_orientation[t->shape][t->orientation][i].y + t->pos.y;
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
	u10 x, y;

	for (i = 0; i < 4; ++i) {
		x = 1 << (hidamari_orientation[field->current.shape]
		                              [field->current.orientation]
		                              [i].x + field->current.pos.x);
		y = hidamari_orientation[field->current.shape]
		                        [field->current.orientation]
		                        [i].y + field->current.pos.y;
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

	tmp.orientation = (tmp.orientation + 1) % 4;
	if (!is_collision(&tmp, field->grid))
		field->current = tmp;
}

static void
field_init(HidamariBuffer *buf, HidamariPlayField *field)
{
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
}

static void
field_update(HidamariBuffer *buf, HidamariPlayField *field, Button act)
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
	move_current(field, BUTTON_DOWN);
	rotate_current(field, BUTTON_R);
	dump_field(field);
}

void
hidamari_init(HidamariBuffer *buf, HidamariGame *game)
{
	field_init(buf, &game->field);
}

void
hidamari_update(HidamariBuffer *buf, HidamariGame *game, Button act)
{
	field_update(buf, &game->field, act);
}
