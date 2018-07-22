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
r7system(HidamariShape bag[7]);

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

/* The different orientations of each hidamari. Note that for these
 * coordinates, y starts at the top of the matrix, not bottom. The vectors are
 * organized as (x, y) pairs */
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

static inline void
buf_set(HidamariBuffer *buf, size_t x, size_t y, HidamariTile tile, u8 const color[3])
{
	buf->tile[x][y] = tile;
	buf->color[x][y][0] = color[0];
	buf->color[x][y][1] = color[1];
	buf->color[x][y][2] = color[2];
}

static inline void
buf_write_cstr(HidamariBuffer *buf, size_t x, size_t y, char const *str)
{
	size_t i;

	for (i = 0; i < strlen(str); ++i) {
		buf->tile[x + i][y] = str[i] - 33;
	}
}

static void
draw_field(HidamariBuffer *buf, size_t x_offset, size_t y_offset, HidamariPlayField *field)
{
	size_t i;
	size_t x, y;
	HidamariTile score[HIDAMARI_WIDTH - 2 + 1];

	/* Draw the borders */
	for (y = 0; y < HIDAMARI_BUFFER_HEIGHT; ++y) {
		buf->tile[x_offset][y_offset + y] = HIDAMARI_TILE_WALL;
		buf->tile[x_offset + HIDAMARI_WIDTH - 1][y_offset + y] = HIDAMARI_TILE_WALL;
	}

	for (x = 1; x < HIDAMARI_BUFFER_WIDTH - 1; ++x) {
		buf->tile[x_offset + x][y_offset] = HIDAMARI_TILE_WALL;
	}

	/* Draw the next piece prievew area */
	for (x = 1; x < HIDAMARI_WIDTH - 1; ++x) {
		for (y = HIDAMARI_HEIGHT_VISIBLE + 3; y < HIDAMARI_HEIGHT_VISIBLE + 6; ++y) {
			if (HIDAMARI_HEIGHT_VISIBLE + 4 == y
			|| HIDAMARI_HEIGHT_VISIBLE + 3 == y) {
				buf->tile[x_offset + x][y_offset + y] = HIDAMARI_TILE_SPACE;
			} else {
				buf->tile[x_offset + x][y_offset + y] = HIDAMARI_TILE_WALL;
			}
		}
	}

	/* Draw the actual next piece */
	for (i = 0; i < 4; ++i) {
		x = x_offset + HIDAMARI_WIDTH_VISIBLE / 2 - 2
			+ hidamari_orientation[field->next]
		                              [0]
		                              [i].x;
		y = y_offset + (HIDAMARI_HEIGHT_VISIBLE + 4)
			- hidamari_orientation[field->next]
		                              [0]
		                              [i].y;
		/* SPECIAL CASE: O needs to be elevated by a single position */
		if (HIDAMARI_O == field->next)
			++y;
		/* if (y >= HIDAMARI_HEIGHT_VISIBLE + 4) */
		/* 	continue; */
		buf->tile[x][y] = HIDAMARI_TILE_SPACE + 1 + field->next;
	}

	/* Draw the scoreboard */
	snprintf((char *)score, sizeof(score) + 1, "%010d", field->score);
	for (x = 1; x < HIDAMARI_WIDTH_VISIBLE - 1; ++x) {
		for (y = HIDAMARI_HEIGHT_VISIBLE; y < HIDAMARI_HEIGHT_VISIBLE + 3; ++y) {
			if (HIDAMARI_HEIGHT_VISIBLE + 1 == y) {
				buf_set(buf, x_offset + x, y_offset + y, score[x - 1] - 48, (u8[]){0, 0 ,0});
			} else {
				buf->tile[x_offset + x][y_offset + y] = HIDAMARI_TILE_WALL;
			}
		}
	}
	/* Draw the playfield */
	for (x = 1; x < HIDAMARI_WIDTH_VISIBLE - 1; ++x) {
		for (y = y_offset + 1; y < y_offset + HIDAMARI_HEIGHT_VISIBLE; ++y) {
			if (field->grid[y] & 1 << x) {
				buf->tile[x_offset + x][y_offset +y] = HIDAMARI_TILE_FALLEN;
			} else {
				buf->tile[x_offset + x][y_offset + y] = HIDAMARI_TILE_SPACE;
			}
		}
	}
	/* Draw the current piece */
	for (i = 0; i < 4; ++i) {
		x = x_offset + hidamari_orientation[field->current.shape]
		                              [field->current.orientation]
		                              [i].x + field->current.pos.x;
		y = y_offset + field->current.pos.y
			- hidamari_orientation[field->current.shape]
		                              [field->current.orientation]
		                              [i].y;
		if (y >= HIDAMARI_HEIGHT_VISIBLE)
			continue;
		buf->tile[x][y] = HIDAMARI_TILE_SPACE + 1 + field->current.shape;
	}
}

static void
draw_main_menu(HidamariBuffer *buf, HidamariGame *game)
{
	static u8 const color[] = {130, 130, 130};
	size_t x, y;

	/* Draw the backdrop */
	for (x = 0; x < HIDAMARI_BUFFER_WIDTH; ++x) {
		for (y = 0; y < HIDAMARI_BUFFER_HEIGHT; ++y) {
			buf->tile[x][y] = HIDAMARI_TILE_WALL;
		}
	}
	for (x = 0; x < HIDAMARI_BUFFER_WIDTH; ++x) {
		for (y = HIDAMARI_BUFFER_HEIGHT - 4; y < HIDAMARI_BUFFER_HEIGHT - 1; ++y) {
			buf_set(buf, x, y, HIDAMARI_TILE_SPACE, color);
		}
	}
	buf_write_cstr(buf, 8, HIDAMARI_BUFFER_HEIGHT - 3, "HIDAMARI");
	buf_write_cstr(buf, 10, HIDAMARI_BUFFER_HEIGHT - 7, "PLAY");
	buf_write_cstr(buf, 9, HIDAMARI_BUFFER_HEIGHT - 9, "OPTION");
	buf_write_cstr(buf, 10, HIDAMARI_BUFFER_HEIGHT - 11, "QUIT");
}

static void
draw_option_menu(HidamariBuffer *buf, HidamariGame *game)
{
	static u8 const color[] = {130, 130, 130};
	size_t x, y;

	/* Draw the backdrop */
	for (x = 0; x < HIDAMARI_BUFFER_WIDTH; ++x) {
		for (y = 0; y < HIDAMARI_BUFFER_HEIGHT; ++y) {
			buf->tile[x][y] = HIDAMARI_TILE_WALL;
		}
	}
	for (x = 0; x < HIDAMARI_BUFFER_WIDTH; ++x) {
		for (y = HIDAMARI_BUFFER_HEIGHT - 4; y < HIDAMARI_BUFFER_HEIGHT - 1; ++y) {
			buf->tile[x][y] = HIDAMARI_TILE_SPACE;
		}
	}
	buf_write_cstr(buf, 3, HIDAMARI_BUFFER_HEIGHT - 3, "OPTION");
	buf_write_cstr(buf, 3, HIDAMARI_BUFFER_HEIGHT - 7, "AI ");
	if (game->ai.active) {
		buf_write_cstr(buf, 6, HIDAMARI_BUFFER_HEIGHT - 7, "ON");
	} else {
		buf_write_cstr(buf, 6, HIDAMARI_BUFFER_HEIGHT - 7, "OFF");
	}
	switch (game->ai.skill) {
		case HIDAMARI_AI_POOR:
			buf_write_cstr(buf, 4, HIDAMARI_BUFFER_HEIGHT - 9, "POOR");
			break;
		case HIDAMARI_AI_NORMAL:
			buf_write_cstr(buf, 3, HIDAMARI_BUFFER_HEIGHT - 9, "NORMAL");
			break;
		case HIDAMARI_AI_SKILLED:
			buf_write_cstr(buf, 2, HIDAMARI_BUFFER_HEIGHT - 9, "SKILLED");
			break;
		case HIDAMARI_AI_GODLIKE:
			buf_write_cstr(buf, 2, HIDAMARI_BUFFER_HEIGHT - 9, "GODLIKE");
			break;
	}
	buf_write_cstr(buf, 4, HIDAMARI_BUFFER_HEIGHT - 11, "BACK");
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
	field->current.pos.y = HIDAMARI_HEIGHT - 1;

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
	int x, y;

	for (i = 0; i < 4; ++i) {
		x = hidamari_orientation[t->shape]
				[t->orientation][i].x + t->pos.x;
		y = t->pos.y - hidamari_orientation[t->shape]
		                                   [t->orientation]
		                                   [i].y;
		if (y < 0 || y > HIDAMARI_HEIGHT - 1
		|| x < 0 || x > HIDAMARI_WIDTH - 1
		|| (1 << x) & grid[y])
			return true;
	}
	return false;
}

/* Lock the current piece in place by copying it to the static piece grid */
static void
lock_hidamari(u12 recv[HIDAMARI_HEIGHT], Hidamari const *hidamari)
{
	int i;
	u12 x, y;

	for (i = 0; i < 4; ++i) {
		x = 1 << (hidamari_orientation[hidamari->shape]
		                              [hidamari->orientation]
		                              [i].x + hidamari->pos.x);
		y = hidamari->pos.y
			- hidamari_orientation[hidamari->shape]
		                              [hidamari->orientation]
		                              [i].y;
		recv[y] |= x;
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

void
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

int
field_update(HidamariPlayField *field, Button act)
{
	Hidamari tmp;

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
		break;
	default:
		/* Don't perform any action for an illegal action */
		break;
	}

	field->gravity_timer += gravity_level[field->level];
	
	if (field->gravity_timer >= drop_time) {
		move_current(field, BUTTON_DOWN);
		field->gravity_timer = 0.0;
	}
	tmp = field->current;
	tmp.pos.y -= 1;
	if (is_collision(&tmp, field->grid)) {
		if (field->slide_timer < slide_time) {
			field->slide_timer += 1;
		} else {
			lock_hidamari(field->grid, &field->current);
			clear_lines(field);
			get_next_hidamari(field);
			field->slide_timer = 0;
			if (is_game_over(field))
				return HIDAMARI_GS_GAME_OVER;
		}
	}
	return HIDAMARI_GS_GAME_PLAYING;
}

int
main_menu(HidamariGame *game, Button act)
{
	uint8_t *cursor = &game->cursor[HIDAMARI_MAIN_CURSOR];

	switch (act) {
		case BUTTON_UP:
			if (0 == *cursor) {
				*cursor = 2;
			} else {
				*cursor = *cursor - 1;
			}
			break;
		case BUTTON_DOWN:
			*cursor = (*cursor + 1) % 3;
			break;
		case BUTTON_B:
			switch (*cursor) {
			case 0:
				field_init(&game->field);
				return HIDAMARI_GS_GAME_PLAYING;
			case 1:
				return HIDAMARI_GS_OPTION_MENU;
			case 2:
				exit(0);
			}
	}
	return HIDAMARI_GS_MAIN_MENU;
}

int
option_menu(HidamariGame *game, Button act)
{
	uint8_t *cursor = &game->cursor[HIDAMARI_OPTION_CURSOR];

	switch (act) {
		case BUTTON_UP:
			if (0 == *cursor) {
				*cursor = 2;
			} else {
				*cursor = *cursor - 1;
			}
			break;
		case BUTTON_DOWN:
			*cursor = (*cursor + 1) % 3;
			break;
		case BUTTON_B:
			switch (*cursor) {
			case 0:
				game->ai.active = !game->ai.active;
				return HIDAMARI_GS_OPTION_MENU;
			case 1:
				return HIDAMARI_GS_OPTION_MENU;
			case 2:
				return HIDAMARI_GS_MAIN_MENU;
			}
	}
	return HIDAMARI_GS_OPTION_MENU;
}

/*
 * Public API
 */

void
hidamari_init(HidamariGame *game)
{
	static Button const dbv = BUTTON_NONE;

	memset(game, 0, sizeof(*game));
	game->state = HIDAMARI_GS_MAIN_MENU;
	game->ai.region = region_create(ai_size_requirement());
	game->ai.planstr = &dbv;
	game->ai.active = false;
}

void
hidamari_update(HidamariGame *game, Button act)
{
	//double weight[3] = {28.586806, 77.649833, 61.638933};
	//double weight[3] = {1.487517, 4.727170, 3.072305};
	//double weight[3] = {0.3146738, 1, 0.649924};
	double weight[3] = {0.848058, 2.304684, 1.405450};

	switch(game->state) {
	case HIDAMARI_GS_MAIN_MENU:
		game->state = main_menu(game, act);
		draw_main_menu(&game->buf, game);
		break;
	case HIDAMARI_GS_OPTION_MENU:
		game->state = option_menu(game, act);
		draw_option_menu(&game->buf, game);
		break;
	case HIDAMARI_GS_GAME_PLAYING:
		if (game->ai.active) {
			if (game->ai.planstr[0] == BUTTON_NONE) {
				region_clear(game->ai.region);
				game->ai.planstr = ai_plan(game->ai.region, weight, &game->field);
			}
			game->state = field_update(&game->field, game->ai.planstr[0]);
			if (HIDAMARI_GS_GAME_OVER == game->state)
				region_destroy(game->ai.region);
			++game->ai.planstr;
		} else {
			game->state = field_update(&game->field, act);
		}
		draw_field(&game->buf, 6, 0, &game->field);
		break;
	case HIDAMARI_GS_GAME_OVER:
		game->state = HIDAMARI_GS_MAIN_MENU;
		break;
	}
}

void
hidamari_pso_update(HidamariGame *game, double weight[3])
{
	if (game->ai.planstr[0] == BUTTON_NONE) {
		region_clear(game->ai.region);
		game->ai.planstr = ai_plan(game->ai.region, weight, &game->field);
	}
	if (0 != field_update(&game->field, game->ai.planstr[0])) {
		game->state = HIDAMARI_GS_GAME_OVER;
		region_destroy(game->ai.region);
	}
	++game->ai.planstr;
}
