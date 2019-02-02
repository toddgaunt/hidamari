struct vga
vga_init(uint32_t *px, size_t w, size_t h)
{
	struct vga rv;

	rv.px = px;
	rv.w = w;
	rv.h = h;

	return rv;
}
