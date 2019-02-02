/* See LICENSE file for copyright and license details */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unistd.h>

#include "ai.h"
#include "hidamari.h"
#include "region.h"

#define SCRN_W 256
#define SCRN_H 192
#define TILE_S 16

void
render(SDL_Renderer *renderer, SDL_Texture *texture, HidamariBuffer *buf)
{
	int i, j;
	SDL_Rect src_r = {.h = TILE_S, .w = TILE_S, .x = 0, .y = 0};
	SDL_Rect dest_r = {.h = TILE_S, .w = TILE_S, .x = 0, .y = 0};
	char tile;

	SDL_RenderClear(renderer);
	/* Render the static grid */
	for (i = 0; i < HIDAMARI_BUFFER_WIDTH; ++i) {
		for (j = 0; j < HIDAMARI_BUFFER_HEIGHT; ++j) {
			tile = buf->tile[HIDAMARI_BUFFER_WIDTH - 1 - i]
				[HIDAMARI_BUFFER_HEIGHT - 1 -j];
			if (TILE_SPACE == tile)
				continue;
			dest_r.x = TILE_S * (HIDAMARI_BUFFER_WIDTH - 1 - i);
			dest_r.y = TILE_S * j;
			src_r.x = TILE_S * (tile % 16);
			src_r.y = TILE_S * (tile / 16);
			SDL_RenderCopy(renderer, texture, &src_r, &dest_r);
		}
	}

	/* Background color */
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderPresent(renderer); 
}

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
	uint32_t acc, dt;
	uint32_t last = SDL_GetTicks();
	uint32_t now;
	uint32_t frame_time;
	SDL_Window *screen;
	SDL_Event event;
	dt = 1000 / 60; /* miliseconds / frames */
	acc = 0.0;
	Button button = BTN_NONE;
	HidamariGame game = {0};

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return EXIT_FAILURE;

	screen = SDL_CreateWindow("Hidamari - SDL2",
	                                      SDL_WINDOWPOS_UNDEFINED,
	                                      SDL_WINDOWPOS_UNDEFINED,
	                                      256, 192,
	                                      SDL_WINDOW_RESIZABLE |
	                                      SDL_WINDOW_OPENGL);

	if (NULL == screen)
		return 1;

	SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, 0);

	if (NULL == renderer)
		return 1;

	/* Set window properties */
	SDL_RenderSetLogicalSize(renderer, 512, 512);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);

	/* Loads the texture map for the game */
	SDL_Surface *tileset_sf = IMG_Load("res/tileset/default.png");
	SDL_Texture *tileset_hw = SDL_CreateTextureFromSurface(renderer, tileset_sf);

	SDL_Texture * texture = SDL_CreateTexture(renderer,
	SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCRN_W, SCRN_H);

	srand(time(NULL));
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
						button = BTN_UP;
						break;
					case SDLK_s:
					case SDLK_j:
					case SDLK_DOWN:
						button = BTN_DOWN;
						break;
					case SDLK_d:
					case SDLK_l:
					case SDLK_RIGHT:
						button = BTN_RIGHT;
						break;
					case SDLK_a:
					case SDLK_h:
					case SDLK_LEFT:
						button = BTN_LEFT;
						break;
					case SDLK_q:
					case SDLK_u:
					case SDLK_z:
					case SDLK_LCTRL:
						button = BTN_L;
						break;
					case SDLK_e:
					case SDLK_i:
					case SDLK_x:
					case SDLK_RCTRL:
						button = BTN_R;
						break;
					case SDLK_RETURN:
					case SDLK_SPACE:
						button = BTN_B;
						break;
					default:
						button = BTN_NONE;
						break;
					}
					break;
				}
			}
			hidamari_update(&game, button);
			acc -= dt;
			button = BTN_NONE;
		}
		//usleep((dt - acc) * 1000);
		hidamari_render(&game);
		//SDL_UpdateTexture(texture, NULL, game.vga.buf, game.vga.w * sizeof(*game.vga.buf));
		//draw(renderer, texture);
		render(renderer, tileset_hw, &game.buf);
	}
endgame:
	SDL_DestroyWindow(screen);
	SDL_DestroyRenderer(renderer);
}
