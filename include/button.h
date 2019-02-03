/* See LICENSE file for copyright and license details */
#ifndef BUTTON_H
#define BUTTON_H

enum button {
	BTN_NONE = 0,
	/* D-pad */
	BTN_UP,
	BTN_DOWN,
	BTN_RIGHT,
	BTN_LEFT,
	/* Shoulder buttons */
	BTN_R,
	BTN_L,
	/* Face buttons */
	BTN_B,
	BTN_A,
	BTN_Y,
	BTN_X,
	BTN_LAST, /* Not an actual action, just used for enum length */
};

#endif
