/* See LICENSE file for copyright and license details */
#ifndef FIELD_H__
#define FIELD_H__

#include <stdlib.h>
#include <stdbool.h>

#include "button.h"
#include "gamestate.h"
#include "type.h"
#include "vga.h"

#define FIELD_WIDTH 12
#define FIELD_HEIGHT 23

#define FIELD_WIDTH_VISIBLE (FIELD_WIDTH)
#define FIELD_HEIGHT_VISIBLE ((FIELD_HEIGHT) - 3)

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

struct piece {
	i8 x;
	i8 y;
	i8 dir : 3;
	enum shape shape : 4;
};

struct field {
	/* Scoring */
	u8 level;
	u32 score; 
	u32 lines; 
	/* Timing */
	f32 gravity_timer; 
	u8 slide_timer;
	/* Randomization */
	u8 bag_pos; /* Current position in the bag */
	enum shape bag[7]; /* Random Bag, used for pseudo-random order */
	/* Hidamaries */
	enum shape next; /* Lookahead piece for player */
	struct piece current;
    /* Bitboard */
	u16 bitboard[FIELD_HEIGHT]; /* Represents static Hidamaries */
};

void
field_init(struct field *field);

bool
field_update(struct field *field, button in);

void
field_draw(struct vga *vp,
           struct vga_surface *sp,
           struct field *field,
           int scale, int x_offset, int y_offset);

void
field_print(struct field *field);

#endif
