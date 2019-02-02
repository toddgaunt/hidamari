/* See LICENSE file for copyright and license details */
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#include "ai.h"
#include "hidamari.h"
#include "region.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static f32 const drop_time = 1.0;
static u8 const slide_time = 15;

static void
r7system(enum shape bag[7]);

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

/* The different orientations of each hidamari. Note that for these
 * coordinates, y starts at the top of the matrix, not bottom. The vectors are
 * organized as (x, y) pairs */
static struct vec2 const shapes[SHAPE_LAST][4][4] = {
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

#if 0
	static inline void
	project_ghost()
	{
	}
#endif

static inline void
buf_set(struct drawbuf *buf, size_t x, size_t y, enum tile tile, u8 const color[3])
{
	buf->tile[x][y] = tile;
	buf->color[x][y][0] = color[0];
	buf->color[x][y][1] = color[1];
	buf->color[x][y][2] = color[2];
}

static inline void
buf_write_cstr(struct drawbuf *buf, size_t x, size_t y, char const *str)
{
	size_t i;

	for (i = 0; i < strlen(str); ++i) {
		buf->tile[x + i][y] = str[i] - 33;
	}
}

#if 0
	/* Draw the next piece prievew area */
	for (x = 1; x < HIDAMARI_WIDTH - 1; ++x) {
		for (y = HIDAMARI_HEIGHT_VISIBLE + 3; y < HIDAMARI_HEIGHT_VISIBLE + 6; ++y) {
			if (HIDAMARI_HEIGHT_VISIBLE + 4 == y
			|| HIDAMARI_HEIGHT_VISIBLE + 3 == y) {
				buf->tile[x_offset + x][y_offset + y] = TILE_SPACE;
			} else {
				buf->tile[x_offset + x][y_offset + y] = TILE_WALL;
			}
		}
	}

	/* Draw the actual next piece */
	for (i = 0; i < 4; ++i) {
		x = x_offset + HIDAMARI_WIDTH_VISIBLE / 2 - 2
			+ shapes[field->next]
		                              [0]
		                              [i].x;
		y = y_offset + (HIDAMARI_HEIGHT_VISIBLE + 4)
			- shapes[field->next]
		                              [0]
		                              [i].y;
		/* SPECIAL CASE: O needs to be elevated by a single position */
		if (SHAPE_O == field->next)
			++y;
		/* if (y >= HIDAMARI_HEIGHT_VISIBLE + 4) */
		/* 	continue; */
		buf->tile[x][y] = TILE_SPACE + 1 + field->next;
	}

	enum tile score[FIELD_WIDTH_VIS - 2 + 1] = {0};
	/* Draw the scoreboard */
	snprintf((char *)score, sizeof(score) + 1, "%010d", field->score);
	for (x = 1; x < HIDAMARI_WIDTH_VISIBLE - 1; ++x) {
		for (y = HIDAMARI_HEIGHT_VISIBLE; y < HIDAMARI_HEIGHT_VISIBLE + 3; ++y) {
			if (HIDAMARI_HEIGHT_VISIBLE + 1 == y) {
				buf_set(buf, x_offset + x, y_offset + y, score[x - 1], (u8[]){0, 0 ,0});
			} else {
				buf->tile[x_offset + x][y_offset + y] = TILE_WALL;
			}
		}
	}

#endif

static void
draw_field(struct drawbuf *buf, size_t x_offset, size_t y_offset, struct field *field)
{
	size_t i;
	size_t x, y;

	/* Draw the borders */
	for (y = 0; y < FIELD_HEIGHT_VIS; ++y) {
		buf->tile[x_offset][y_offset + y] = TILE_WALL;
		buf->tile[x_offset + FIELD_WIDTH_VIS - 1][y_offset + y] = TILE_WALL;
	}

	for (x = 1; x < FIELD_WIDTH_VIS - 1; ++x) {
		buf->tile[x_offset + x][y_offset] = TILE_WALL;
	}

	/* Draw the field */
	for (x = 1; x < FIELD_WIDTH_VIS - 1; ++x) {
		for (y = 1; y < FIELD_HEIGHT_VIS; ++y) {
			if (field->grid[y] & 1 << x) {
				buf->tile[x_offset + x][y_offset + y] = TILE_FALLEN;
			} else {
				buf->tile[x_offset + x][y_offset + y] = TILE_SPACE;
			}
		}
	}
	/* Draw the current piece */
	for (i = 0; i < 4; ++i) {
		x = x_offset + shapes[field->current.shape]
		                     [field->current.dir]
		                     [i].x + field->current.x;
		y = y_offset + field->current.y
			- shapes[field->current.shape]
		                              [field->current.dir]
		                              [i].y;
		if (y >= FIELD_HEIGHT_VIS)
			continue;
		buf->tile[x][y] = TILE_SPACE + 1 + field->current.shape;
	}
}

/* Shift all lines above a certain y value down by one */
static void
shift_lines(struct field *field, size_t y_start)
{
	size_t y;

	for (y = y_start; y < HIDAMARI_HEIGHT - 1; ++y) {
		field->grid[y] = field->grid[y + 1];
	}
}

static void
clear_lines(struct field *field)
{
	size_t y = 1;
	size_t combo = 0;
	size_t score;

	while (y < HIDAMARI_HEIGHT) {
		if (2046 == (field->grid[y] & 2046)) {
			shift_lines(field, y);
			combo += 1;
		} else {
			y += 1;
		}
	}

	field->lines += combo;
	
	switch (combo) {
	case 0:
		score = 0;
		break;
	case 1:
		score = 1;
		break;
	case 2:
		score = 3;
		break;
	case 3:
		score = 5;
		break;
	case 4:
		score = 8;
		break;
	default:
		score = 8;
		break;
	}

	if (field->score > UINT32_MAX - score) {
		field->score = UINT32_MAX;
	} else {
		field->score += score;
	}

	/* Temporary solution to leveling */
	if (field->score <= 5) {
		field->level = 0;
	} else if (field->score <= 15) {
		field->level = 1;
	} else if (field->score <= 30) {
		field->level = 2;
	} else if (field->score <= 50) {
		field->level = 3;
	} else if (field->score <= 75) {
		field->level = 4;
	} else if (field->score <= 105) {
		field->level = 5;
	} else if (field->score <= 140) {
		field->level = 6;
	} else if (field->score <= 180) {
		field->level = 7;
	} else {
		field->level = 8;
	}
}

static bool
is_game_over(struct field *field)
{
	if (field->grid[HIDAMARI_HEIGHT - 1] & 0x7FE)
		return true;
	return false;
}

/* Get a new hidamari into play, and update the next hidamari */
static void
get_next_hidamari(struct field *field)
{
	field->current.shape = field->next;
	field->current.dir = 0;
	field->current.x = 10 / 2 - 1;
	field->current.y = HIDAMARI_HEIGHT - 1;

	if (field->bag_pos >= 7) {
		r7system(field->bag);
		field->bag_pos = 0;
	}
	field->next = field->bag[field->bag_pos];
	field->bag_pos += 1;
}

/* Check if the Hidamari would collide in the given grid, at the given x,y coordinates */
static bool
is_collision(struct piece const *t, u12 const grid[HIDAMARI_HEIGHT])
{
	int i;
	int x, y;

	for (i = 0; i < 4; ++i) {
		x = shapes[t->shape][t->dir][i].x + t->x;
		y = t->y - shapes[t->shape][t->dir][i].y;
		if (y < 0 || y > HIDAMARI_HEIGHT - 1
		|| x < 0 || x > HIDAMARI_WIDTH - 1
		|| (1 << x) & grid[y])
			return true;
	}
	return false;
}

/* Lock the current piece in place by copying it to the static piece grid */
static void
lock_piece(u12 grid[HIDAMARI_HEIGHT], struct piece const *t)
{
	int i;
	u12 x, y;

	for (i = 0; i < 4; ++i) {
		x = 1 << (shapes[t->shape][t->dir][i].x + t->x);
		y = t->y - shapes[t->shape][t->dir][i].y;

		printf("%d - %d = %d\n", t->y, shapes[t->shape][t->dir][i].y, y);

		grid[y] |= x;
	}
}

/* Move the current piece in the given direction */
static bool
move_current(struct field *field, int x, int y)
{
	struct piece tmp = field->current;

	tmp.x += x;
	tmp.y += y;
	if (is_collision(&tmp, field->grid))
		return false;
	field->current = tmp;
	return true;
}

/* Standard Tetris Random Hidamari generator. */
static void
r7system(enum shape bag[7])
{
	int i;
	int r1;
	int r2;
	enum shape tmp;

	bag[0] = SHAPE_I;
	bag[1] = SHAPE_J;
	bag[2] = SHAPE_L;
	bag[3] = SHAPE_O;
	bag[4] = SHAPE_S;
	bag[5] = SHAPE_T;
	bag[6] = SHAPE_Z;
	for (i = 0; i < 7; ++i) {
		r1 = rand() % 7;
		r2 = rand() % 7;
		tmp = bag[r1];
		bag[r1] = bag[r2];
		bag[r2] = tmp;
	}
}
	
/* Rotate the current hidamari */
static void
rotate_current(struct field *field, int delta)
{
	struct piece tmp = field->current;

	tmp.dir = MIN((u8)(tmp.dir + delta) % 4, 3);
	if (!is_collision(&tmp, field->grid))
		field->current = tmp;
}

void
field_init(struct field *field)
{
	size_t i;

	memset(field, 0, sizeof(*field));
	/* Initialize the random bag */
	r7system(field->bag);
	/* First hidamari must not be a S, Z, or O */
	do {
		field->next = rand() % 7;
	} while (SHAPE_O == field->next
	      || SHAPE_S == field->next
	      || SHAPE_Z == field->next);
	get_next_hidamari(field);
	/* Initialize the borders */
	field->grid[0] |= 4095;
	for (i = 1; i < HIDAMARI_HEIGHT; ++i) {
		field->grid[i] = 2049;
	}
}

int
field_update(struct field *field, Button act)
{
	struct piece tmp;

	switch (act) {
	case BTN_NONE:
		break;
	case BTN_DOWN:
		move_current(field, +0, -1);
		break;
	case BTN_RIGHT:
		move_current(field, +1, +0);
		break;
	case BTN_LEFT:
		move_current(field, -1, +0);
		break;
	case BTN_R:
		rotate_current(field, +1);
		break;
	case BTN_L:
		rotate_current(field, -1);
		break;
	case BTN_B:
		while (move_current(field, +0, -1))
			;
		field->slide_timer = slide_time;
		break;
	default:
		/* Don't perform any action for an illegal action */
		break;
	}

	field->gravity_timer += gravity_level[field->level];
	
	if (field->gravity_timer >= drop_time) {
		move_current(field, +0, -1);
		field->gravity_timer = 0.0;
	}
	tmp = field->current;
	tmp.y -= 1;
	if (is_collision(&tmp, field->grid)) {
		if (field->slide_timer < slide_time) {
			field->slide_timer += 1;
		} else {
			lock_piece(field->grid, &field->current);
			clear_lines(field);
			get_next_hidamari(field);
			field->slide_timer = 0;
			if (is_game_over(field))
				return GAMESTATE_OVER;
		}
	}
	return GAMESTATE_PLAYING;
}

/*
 * Public API
 */

void
hidamari_update(struct hidamari *game, Button in)
{
	static Button const dbv = BTN_NONE;
	double weight[3] = {0.848058, 2.304684, 1.405450};

	switch(game->state) {
	case GAMESTATE_INIT:
		memset(game, 0, sizeof(*game));
		game->state = GAMESTATE_PLAYING;
		game->ai.region = region_create(ai_size_requirement());
		game->ai.planstr = &dbv;
		game->ai.active = false;
		field_init(&game->field);
		break;
	case GAMESTATE_PLAYING:
		game->state = field_update(&game->field, in);
		break;
	case GAMESTATE_OVER:
		game->state = GAMESTATE_PLAYING;
		field_init(&game->field);
		break;
	}
}

void
hidamari_render(struct drawbuf *buf, struct hidamari *game)
{
	draw_field(buf, 0, 0, &game->field);
	draw_field(buf, FIELD_WIDTH_VIS + 4, 0, &game->field);
}
