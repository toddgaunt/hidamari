/* See LICENSE file for copyright and license details */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unistd.h>

#include "hidamari.h"
#include "vga.h"

#define SCRN_W 256
#define SCRN_H 192
#define TILE_S 8 

void
draw(SDL_Renderer *renderer, SDL_Texture *texture)
{
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

int
main()
{
	uint32_t acc;
	uint32_t dt;
	uint32_t last;
	uint32_t now;
	uint32_t frame_time;

	SDL_Window *screen;
	SDL_Event event;

	enum button in = BTN_NONE;
	struct hidamari game;

	uint32_t *px = malloc(sizeof(*px) * SCRN_W * SCRN_H);
	struct vga vga = vga_init(px, SCRN_W, SCRN_H);

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return EXIT_FAILURE;

	screen = SDL_CreateWindow("Hidamari - SDL2",
	                                      SDL_WINDOWPOS_UNDEFINED,
	                                      SDL_WINDOWPOS_UNDEFINED,
	                                      SCRN_W, SCRN_H,
	                                      SDL_WINDOW_RESIZABLE |
	                                      SDL_WINDOW_OPENGL);

	if (NULL == screen)
		return 1;

	SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, 0);

	if (NULL == renderer)
		return 1;

	/* Set window properties */
	SDL_RenderSetLogicalSize(renderer, SCRN_W, SCRN_H);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
	
	SDL_Texture * texture = SDL_CreateTexture(renderer,
	SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCRN_W, SCRN_H);

	srand(time(NULL));
	last = SDL_GetTicks();
	dt = 1000 / 60; /* miliseconds / frames */
	acc = 0.0;
	hidamari_init(&game);
	for (;;) {
		// Uncomment and change the number below to test lag!
		//usleep(100000);
		now = SDL_GetTicks();
		frame_time = now - last;
		last = now;
		acc += frame_time;

		while (acc >= dt) {
			while (SDL_PollEvent(&event)) {      
				switch (event.type) {
				case SDL_QUIT:
					goto endgame;
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) {
					case SDLK_w:
					case SDLK_k:
					case SDLK_UP:
						in = BTN_UP;
						break;
					case SDLK_s:
					case SDLK_j:
					case SDLK_DOWN:
						in = BTN_DOWN;
						break;
					case SDLK_d:
					case SDLK_l:
					case SDLK_RIGHT:
						in = BTN_RIGHT;
						break;
					case SDLK_a:
					case SDLK_h:
					case SDLK_LEFT:
						in = BTN_LEFT;
						break;
					case SDLK_q:
					case SDLK_u:
					case SDLK_z:
					case SDLK_LCTRL:
						in = BTN_L;
						break;
					case SDLK_e:
					case SDLK_i:
					case SDLK_x:
					case SDLK_RCTRL:
						in = BTN_R;
						break;
					case SDLK_RETURN:
					case SDLK_SPACE:
						in = BTN_B;
						break;
					default:
						in = BTN_NONE;
						break;
					}
					break;
				}
			}
			hidamari_update(&game, in);
			acc -= dt;
			in = BTN_NONE;
		}
		hidamari_render(&vga, &game);
		SDL_UpdateTexture(texture, NULL, vga.px, vga.w * sizeof(*vga.px));
		draw(renderer, texture);
	}
endgame:
	SDL_DestroyWindow(screen);
	SDL_DestroyRenderer(renderer);
}


	/* Loads the texture map for the game */
	//SDL_Surface *tileset_sf = IMG_Load("res/tileset/8px.png");
	//SDL_Texture *tileset_hw = SDL_CreateTextureFromSurface(renderer, tileset_sf);

	//SDL_Texture *texture = SDL_CreateTexture(renderer,
	//SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCRN_W, SCRN_H);
