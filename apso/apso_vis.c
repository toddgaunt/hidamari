/* See LICENSE file for copyright and license details */
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "heap.h"
#include "region.h"

#include "apso_internal.h"

int coord_x;
int coord_y;

void
update_buffer(char buf[1080][1080], Swarm const *s)
{
	size_t i;
	int x, y;

	memset(buf, ' ', sizeof(**buf) * 1080 * 1080);
	for (i = 0; i < s->n_pt; ++i) {
		x = (int)s->pt[i].position[0];
		y = (int)s->pt[i].position[1];
		if (x >= 1080 || y >= 1080
		||  x < 0 || y < 0)
			continue;
		buf[x][y] = '*';
	}
}

void
render(SDL_Renderer *renderer, char buf[1080][1080])
{
	int i, j;
	SDL_Rect dest_r = {.h = 2, .w = 2, .x = 0, .y = 0};

	SDL_RenderClear(renderer);
	/* Render the static grid */
	for (i = 0; i < 1000; ++i) {
		for (j = 0; j < 1000; ++j) {
			if ('*' != buf[i][j])
				continue;
			dest_r.x = i - 1;
			dest_r.y = j - 1;
			SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
			SDL_RenderFillRect(renderer, &dest_r);
		}
	}

	/* Background color */
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderPresent(renderer); 
}

static float
rfrange(int lower, int upper)
{
	lower *= 1000;
	upper *= 1000;

	return (float)((rand() % (upper + 1 - lower)) + lower) / 1000;
}

void
render_work(Swarm *s)
{
	uint32_t acc, dt;
	uint32_t last;
	uint32_t now;
	uint32_t frame_time;
	SDL_Window *screen;
	SDL_Event event;
	dt = 1000 / 60; /* miliseconds / frames */
	char buf[1080][1080];

	if (0 > SDL_Init(SDL_INIT_VIDEO))
		exit(EXIT_FAILURE);
	screen = SDL_CreateWindow("Swarm - SDL2",
	                           SDL_WINDOWPOS_UNDEFINED,
	                           SDL_WINDOWPOS_UNDEFINED,
	                           512, 512,
	                           SDL_WINDOW_RESIZABLE |
	                           SDL_WINDOW_OPENGL);
	if (!screen)
		exit(EXIT_FAILURE);
	SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, 0);
	if (!renderer)
		exit(EXIT_FAILURE);
	/* Set window properties */
	SDL_RenderSetLogicalSize(renderer, 1080, 1080);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);

	acc = 0;
	last = SDL_GetTicks();
	for (;;) {
		now = SDL_GetTicks();
		frame_time = now - last;
		last = now;
		acc += frame_time;

		while (acc >= dt) {
			while (SDL_PollEvent(&event)) {      
				switch (event.type) {
				case SDL_QUIT:
					exit(EXIT_SUCCESS);
				case SDL_MOUSEBUTTONDOWN:
					switch (event.button.button) {
					case SDL_BUTTON_LEFT:
						SDL_GetMouseState(&coord_x, &coord_y);
						printf("%d, %d\n", coord_x, coord_y);
						for (size_t i = 0; i < s->n_pt; ++i) {
							s->pt[i].p_score = -INFINITY;
							for (size_t j = 0; j < s->n_dimension; ++j) {
								/* s->pt[i].position[j] += */
								/* 	rfrange(-10, 10); */
								s->pt[i].velocity[j] =
									rfrange(-100, 100);
							}
						}
						s->g_score = -INFINITY;
						break;
					}
					break;
				}
			}
			update_buffer(buf, s);
			acc -= dt;
		}
		render(renderer, buf);
	}
}	

float *
apso_vis(
		size_t n_thread,
		size_t n_iteration,
		size_t n_particle,
		size_t n_dimension,
		int b_lo, int b_up,
		float phi, float alpha, float beta,
		float (*fitness)(float const *argv))
{
	size_t i;
	pthread_t *tid;
	Swarm *s;

	coord_x = 500;
	coord_y = 500;
	tid = malloc(sizeof(*tid) * n_thread);
	s = swarm_create(n_iteration, n_particle, n_dimension, b_lo, b_up,
			phi, alpha, beta, fitness);
	/* Begin the swarm */
	for (i = 0; i < n_thread; ++i) {
		if (pthread_create(&tid[i], NULL, apso_work, (void *) s) != 0) {
			fprintf(stderr, "error: Could not create thread\n");
			exit(EXIT_FAILURE);
		}
	}
	render_work(s);
	/* End the swarm */
	for (i = 0; i < n_thread; ++i) {
		if (pthread_join(tid[i], NULL)) {
			fprintf(stderr, "error: Could not join thread\n");
			exit(EXIT_FAILURE);
		}
	}
	swarm_destroy(s);
	free(tid);
	return s->g_position;
}

float
apso_vis_fitness(float const *position)
{
	usleep(100);
	int tmp = -sqrt(pow(coord_x - position[0], 2) + pow(coord_y - position[1], 2));
	
	if (tmp > -100)
		return -100;
	return tmp;

}
