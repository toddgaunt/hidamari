/* See LICENSE file for copyright and license details */
#include <stdlib.h>
#include <string.h>
#include "vga.h"
	
struct vga
vga_init(uint32_t *px, int w, int h)
{
	struct vga rv;

	rv.w = w;
	rv.h = h;
	rv.px = px;

	return rv;
}

void
vga_fill(struct vga *vp, uint32_t color)
{
	memset(vp->px, color, sizeof(*vp->px) * vp->w * vp->h);
}

void
vga_fill_rect(struct vga *vp, struct vga_rect *dst, uint32_t color)
{
	int i, j;

	for (i = 0; i < dst->h; ++i) {
		for (j = 0; j < dst->w; ++j) {
			vp->px[((dst->y + i) * vp->w) + (dst->x + j)] = color;
		}
	}
}
