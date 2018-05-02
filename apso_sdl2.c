/* See LICENSE file for copyright and license details */
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "ai.h"
#include "hidamari.h"
#include "heap.h"

char const *argv0;

typedef struct {
	int id;
	size_t iter;
	float p_score;
	float *p_position;
	float *position;
	float *velocity;
} Particle;

int
particle_cmp(Particle const *a, Particle const *b);

HEAP_INSTANTIATE(wq, Particle *, malloc, particle_cmp)
typedef wq WorkQueue;

typedef struct {
	/* Synchronization */
	pthread_mutex_t g_lock;
	pthread_mutex_t queue_lock;
	WorkQueue *queue;

	/* Parameters */
	size_t n_iteration;
	int b_lo;
	int b_up;
	float phi;
	float alpha;
	float beta;
	float (*fitness)(float const *argv);

	/* Swarm state */
	size_t n_dimension;
	float g_score;
	float *g_position;
	size_t n_pt;
	Particle *pt;
} Swarm;

void
usage()
{
	fprintf(stderr, "usage: %s <number of particles> <number of iterations>\n", argv0);
	exit(EXIT_FAILURE);
}

int
swarm_queue_push(Swarm *s, Particle *p)
{
	int ret;

	pthread_mutex_lock(&s->queue_lock);
	ret = wq_push(s->queue, p);
	pthread_mutex_unlock(&s->queue_lock);
	return ret;
}

Particle *
swarm_queue_pop(Swarm *s)
{
	Particle *ret;

	pthread_mutex_lock(&s->queue_lock);
	ret = wq_pop(s->queue);
	pthread_mutex_unlock(&s->queue_lock);
	return ret;
}

int
particle_cmp(Particle const *a, Particle const *b)
{
	return a->iter - b->iter;
}

/* Generate a random number between [lower, upper] */
float
rfrange(int lower, int upper)
{
	lower *= 1000;
	upper *= 1000;

	return (float)((rand() % (upper + 1 - lower)) + lower) / 1000;
}

float
swarm_g_position_load(Swarm *s, size_t index)
{
	float tmp;

	pthread_mutex_lock(&s->g_lock);
	tmp = s->g_position[index];
	pthread_mutex_unlock(&s->g_lock);
	return tmp;
}

void
swarm_g_compare_store(Swarm *s, float score, float *position)
{
	pthread_mutex_lock(&s->g_lock);
	if (score > s->g_score) {
		s->g_score = score;
		memcpy(s->g_position, position, sizeof(float) * s->n_dimension);
	}
	pthread_mutex_unlock(&s->g_lock);
}

void *
apso_work(void *arg)
{
	size_t i;
	Particle *p;
	Swarm *s = arg;
	float score;
	float rp, rg;

	/* Continously move and evaluate the particles */
	while ((p = swarm_queue_pop(s))) {
		/* Update the particle's velocity */
		for (i = 0; i < s->n_dimension; ++i) {
			rp = rfrange(0, 1);
			rg = rfrange(0, 1);
			/* The magical velocity formula */
			p->velocity[i] = 
				+ s->phi * p->velocity[i]
				+ s->alpha * rp * (p->p_position[i]
						- p->position[i])
				+ s->beta * rg * (swarm_g_position_load(s, i)
						- p->position[i]);
		}
		/* Update the particle's position */
		for (i = 0; i < s->n_dimension; ++i)
			p->position[i] += p->velocity[i];
		/* Evaluate the new fitness of the particle */
		score = s->fitness(p->position);
		if (score > p->p_score) {
			memcpy(p->p_position, p->position,
				sizeof(*p->p_position) * s->n_dimension);
			p->p_score = score;
			swarm_g_compare_store(s, score, p->position);
		}
		/* if (p->iter < s->n_iteration) { */
			p->iter += 1;
			swarm_queue_push(s, p);
		/* } */
	}
	return NULL;
}

void
render(SDL_Renderer *renderer, Swarm *s)
{
	int i, j;
	SDL_Rect dest_r = {.h = 2, .w = 2, .x = 0, .y = 0};
	char buf[1000][1000];

	memset(buf, ' ', sizeof(buf));
	for (i = 0; i < (int)s->n_pt; ++i) {
		if (s->pt[i].position[0] * 10 >= 1000
		|| s->pt[i].position[0] * 10 < 0
		|| s->pt[i].position[1] * 10 >= 1000
		|| s->pt[i].position[1] * 10 < 0)
			continue;
		buf[(int)(s->pt[i].position[0] * 10)][(int)(s->pt[i].position[1] * 10)] = '*';
	}
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

float *
apso(
		size_t n_thread,
		size_t n_iteration,
		size_t n_particle,
		size_t n_dimension,
		int b_lo, int b_up,
		float phi, float alpha, float beta,
		float (*fitness)(float const *argv))
{
	size_t i, j;
	Swarm s;
	pthread_t *tid;

	tid = malloc(sizeof(*tid) * n_thread);

	/* Configure the swarm */
	s.n_iteration = n_iteration;
	s.n_dimension = n_dimension;
	s.b_lo = b_lo;
	s.b_up = b_up;
	s.alpha = alpha;
	s.beta = beta;
	s.phi = phi;
	s.fitness = fitness;
	s.g_score = INT32_MIN;
	s.g_position = calloc(n_dimension, sizeof(*s.g_position));
	s.n_pt = n_particle;
	s.pt = malloc(sizeof(*s.pt) * n_particle);
	s.queue = wq_create(n_particle);
	pthread_mutex_init(&s.queue_lock, NULL);
	pthread_mutex_init(&s.g_lock, NULL);

	/* Initialize the swarm */
	for (i = 0; i < n_particle; ++i) {
		s.pt[i].id = i;
		s.pt[i].iter = 1;
		s.pt[i].p_score = INT32_MIN;
		s.pt[i].p_position = malloc(sizeof(*s.pt[i].position) * n_dimension);
		s.pt[i].position = malloc(sizeof(*s.pt[i].position) * n_dimension);
		s.pt[i].velocity = malloc(sizeof(*s.pt[i].position) * n_dimension);
		for (j = 0; j < n_dimension; ++j) {
			s.pt[i].position[j] = rfrange(b_lo, b_up);
			s.pt[i].p_position[j] = s.pt[i].position[j];
		}
		for (j = 0; j < n_dimension; ++j) {
			s.pt[i].velocity[j] = rfrange(-abs(b_up - b_lo),
						    abs(b_up - b_lo));
		}
		swarm_queue_push(&s, &s.pt[i]);
	}
	/* Begin the swarm */
	for (i = 0; i < n_thread; ++i) {
		if (pthread_create(&tid[i], NULL, apso_work, (void *) &s) != 0)
			return NULL;
	}
	//apso_work(&s);
	{
		uint32_t acc, dt;
		uint32_t last = SDL_GetTicks();
		uint32_t now;
		uint32_t frame_time;
		SDL_Window *screen;
		SDL_Event event;
		dt = 1000 / 60; /* miliseconds / frames */

		if (SDL_Init(SDL_INIT_VIDEO) < 0)
			exit(EXIT_FAILURE);
		screen = SDL_CreateWindow("Swarm - SDL2",
						      SDL_WINDOWPOS_UNDEFINED,
						      SDL_WINDOWPOS_UNDEFINED,
						      512, 512,
						      SDL_WINDOW_RESIZABLE |
						      SDL_WINDOW_OPENGL);
		if (NULL == screen)
			exit(EXIT_FAILURE);
		SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, 0);
		if (NULL == renderer)
			exit(EXIT_FAILURE);
		/* Set window properties */
		SDL_RenderSetLogicalSize(renderer, 1000, 1000);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);

		acc = 0;
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
					}
				}
				acc -= dt;
			}
			render(renderer, &s);
		}
	}	
	/* End the swarm */
	for (i = 0; i < n_thread; ++i) {
		if (pthread_join(tid[i], NULL))
			return NULL;
	}
	for (i = 0; i < n_particle; ++i) {
		free(s.pt[i].p_position);
		free(s.pt[i].position);
		free(s.pt[i].velocity);
	}
	free(tid);
	free(s.pt);
	wq_destroy(s.queue);
	return s.g_position;
}

float
distance_fitness(float const *position)
{
	//usleep(100);
	return - sqrt(pow(50 - position[0], 2.0) + pow(50 - position[1], 2.0));
}

int
main(int argc, char **argv)
{
	size_t n_particle;
	size_t n_iteration;
	float *best;

	srand(time(NULL));
	argv0 = argv[0];
	if (argc != 3)
		usage();
	n_particle = strtol(argv[1], NULL, 10);
	n_iteration = strtol(argv[2], NULL, 10);
	best = apso(4, n_iteration, n_particle, 2, 0, 100, 0.004, 0.001, 0.002,
			distance_fitness);
	printf("best position: (%f, %f)\n",
			best[0],
			best[1]);
	free(best);
	return 0;
}
