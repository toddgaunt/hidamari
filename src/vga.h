struct vga {
	uint32_t *px;
	size_t w;
	size_t h;
};

struct vga
vga_init(uint32_t *px, size_t w, size_t h);
