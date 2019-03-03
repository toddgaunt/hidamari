/* See LICENSE file for copyright and license details */
#ifndef VGA_H
#define VGA_H

#include "stdint.h"

#define VGA_FLIP_VERTICAL   (1 << 0)
#define VGA_FLIP_HORIZONTAL (1 << 1)

struct vga {
	int w;
	int h;
	uint32_t *px;
};

struct vga_surface {
	int w;
	int h;
	uint32_t *px;
};

struct vga_rect {
	int w;
	int h;
	int x;
	int y;
};

struct vga
vga_init(uint32_t *px, int w, int h);

uint32_t
vga_map_rgb(uint8_t r, uint8_t g, uint8_t b);

void
vga_load_image(struct vga_surface *sp);

void
vga_fill(struct vga *vp, uint32_t color);

void
vga_fill_rect(struct vga *vp, struct vga_rect *dst, uint32_t color);

void
vga_copy_rect(struct vga *vga,
              struct vga_surface *surface,
              struct vga_rect *src,
              struct vga_rect *dst);

#endif
