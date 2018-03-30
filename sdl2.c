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

void
render(SDL_Renderer *renderer, SDL_Texture *texure, HidamariBuffer *buf)
{
}

int
main()
{
	HidamariGame game;
	uint32_t acc, dt;
	uint32_t last = SDL_GetTicks();
	uint32_t now;
	uint32_t frame_time;
	SDL_Window *screen;
	SDL_Event event;
	dt = 1000 / 60; /* miliseconds / frames */
	acc = 0.0;
	bool keypress = true;
	Button button = BUTTON_NONE;
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
	hidamari_init(&buf, &game);
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
					button = BUTTON_DOWN;
					break;
				case SDLK_d:
				case SDLK_l:
				case SDLK_RIGHT:
					button = BUTTON_RIGHT;
					break;
				case SDLK_a:
				case SDLK_j:
				case SDLK_LEFT:
					button = BUTTON_LEFT;
					break;
				case SDLK_e:
				case SDLK_o:
				case SDLK_UP:
				case SDLK_x:
					button = BUTTON_R;
					break;
				case SDLK_q:
				case SDLK_u:
				case SDLK_RCTRL:
				case SDLK_LCTRL:
					button = BUTTON_L;
					break;
				case SDLK_RETURN:
				case SDLK_SPACE:
					button = BUTTON_X;
					break;
				default:
					button = BUTTON_NONE;
					break;
				}
				break;
			}
		}

		while (acc >= dt) {
			hidamari_update(&buf, &game, button);
			acc -= dt;
			button = BUTTON_NONE;
		}

		render(renderer, tileset_hw, &buf);
	}

	SDL_DestroyWindow(screen);
	SDL_DestroyRenderer(renderer);
}
