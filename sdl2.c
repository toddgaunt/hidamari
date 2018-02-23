/* See LICENSE file for copyright and license details */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unistd.h>

#include "hidamari.h"

#define TILE_S 16

static void
render(SDL_Renderer *renderer, SDL_Texture *tileset, HidamariBuffer *buf)
{
	int i, j;
	SDL_Rect src_r = {.h = TILE_S, .w = TILE_S, .x = 0, .y = 0};
	SDL_Rect dest_r = {.h = TILE_S, .w = TILE_S, .x = 0, .y = 0};
	HidamariShape tile;
	HidamariFlag flag;

	SDL_RenderClear(renderer);
	/* Render the static grid */
	for (i = 0; i < HIDAMARI_BUFFER_WIDTH; ++i) {
		for (j = 0; j < HIDAMARI_BUFFER_HEIGHT; ++j) {
			tile = buf->tile[HIDAMARI_BUFFER_WIDTH - 1 - i][HIDAMARI_BUFFER_HEIGHT - 1 -j] & HIDAMARI_TILE_MASK;
			flag = buf->tile[HIDAMARI_BUFFER_WIDTH - 1 - i][HIDAMARI_BUFFER_HEIGHT - 1 -j] & HIDAMARI_FLAG_MASK;
			if (HIDAMARI_NONE == tile)
				continue;
			dest_r.x = TILE_S * (HIDAMARI_BUFFER_WIDTH - 1 - i);
			dest_r.y = TILE_S * j;
			if (HIDAMARI_TRANSPARENT == (flag & HIDAMARI_TRANSPARENT)) {
				SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
			 	SDL_RenderFillRect(renderer, &dest_r);
			} else {
				src_r.x = TILE_S * (tile % 14);
				src_r.y = TILE_S * (tile / 14);
				SDL_RenderCopy(renderer, tileset, &src_r, &dest_r);
			}
		}
	}

	/* Background color */
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderPresent(renderer); 
}

int
main()
{
	Playfield field;
	uint32_t acc, dt;
	uint32_t last = SDL_GetTicks();
	uint32_t now;
	uint32_t frame_time;
	SDL_Window *screen;
	SDL_Event event;
	dt = 1000 / 60; /* miliseconds / frames */
	acc = 0.0;
	bool keypress = true;
	Action action = ACTION_NONE;
	HidamariBuffer buf;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return EXIT_FAILURE;
	screen = SDL_CreateWindow("Hidamari - SDL2",
	                                      SDL_WINDOWPOS_UNDEFINED,
	                                      SDL_WINDOWPOS_UNDEFINED,
	                                      512, 512,
	                                      SDL_WINDOW_RESIZABLE |
	                                      SDL_WINDOW_OPENGL);
	if (NULL == screen)
		return 1;
	SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, 0);
	if (NULL == renderer)
		return 1;
	/* Set window properties */
	SDL_RenderSetLogicalSize(renderer, TILE_S * HIDAMARI_BUFFER_WIDTH, TILE_S * HIDAMARI_BUFFER_HEIGHT);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
	/* Loads the texture map for the game */
	SDL_Surface *tileset_sf = IMG_Load("res/tileset/default.png");
	SDL_Texture *tileset_hw = SDL_CreateTextureFromSurface(renderer, tileset_sf);

	srand(time(NULL));
	hidamari_init(&buf, &field);
	while(keypress) {
		// Uncomment and change the number below to test lag!
		//usleep(100000);
		now = SDL_GetTicks();
		frame_time = now - last;
		last = now;
		acc += frame_time;

		while (SDL_PollEvent(&event)) {      
			if (SDL_QUIT == event.type)
				keypress = false;
			switch (event.type) {
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
				case SDLK_s:
				case SDLK_k:
				case SDLK_DOWN:
					action = ACTION_MV_D;
					break;
				case SDLK_d:
				case SDLK_l:
				case SDLK_RIGHT:
					action = ACTION_MV_R;
					break;
				case SDLK_a:
				case SDLK_j:
				case SDLK_LEFT:
					action = ACTION_MV_L;
					break;
				case SDLK_e:
				case SDLK_o:
				case SDLK_UP:
				case SDLK_x:
					action = ACTION_ROT_R;
					break;
				case SDLK_q:
				case SDLK_u:
				case SDLK_RCTRL:
				case SDLK_LCTRL:
					action = ACTION_ROT_L;
					break;
				case SDLK_RETURN:
				case SDLK_SPACE:
					action = ACTION_HARD_DROP;
					break;
				default:
					action = ACTION_NONE;
					break;
				}
				break;
			}
		}

		while (acc >= dt) {
			hidamari_update(&buf, &field, action);
			acc -= dt;
			action = ACTION_NONE;
		}

		render(renderer, tileset_hw, &buf);
	}

	SDL_DestroyWindow(screen);
	SDL_DestroyRenderer(renderer);
}
