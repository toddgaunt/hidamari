/* See LICENSE file for copyright and license details */
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ai.h"
#include "hidamari.h"
#include "heap.h"

char const *argv0;

typedef struct {
	int id;
	size_t iter;
	int p_score;
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
	int (*fitness)(float const *argv);

	/* Swarm state */
	size_t n_dimension;
	int g_score;
	float *g_position;
	size_t n_pt;
	Particle *pt;
} Swarm;

void display(Swarm *s)
{
	int i, j;
	char buf[s->b_up][s->b_up];

	memset(buf, ' ', sizeof(buf));
	for (i = 0; i < (int)s->n_pt; ++i) {
		if (s->pt[i].position[0] >= s->b_up
		|| s->pt[i].position[1] >= s->b_up)
			continue;
		if (s->pt[i].id < 10) {
			buf[(int)s->pt[i].position[0]][(int)s->pt[i].position[1]] = 48 + s->pt[i].id;
		} else {
			buf[(int)s->pt[i].position[0]][(int)s->pt[i].position[1]] = 65 - 10 + s->pt[i].id;
		}
	}

	for (i = 0; i < s->b_up; ++i) {
		for (j = 0; j < s->b_up; ++j) {
			putc(buf[i][j], stdout);
			putc(' ', stdout);
		}
		putc('\n', stdout);
	}
}

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
swarm_g_compare_store(Swarm *s, int score, float *position)
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
	int score;

	/* Continously move and evaluate the particles */
	while ((p = swarm_queue_pop(s))) {
		/* Update the particle's velocity */
		for (i = 0; i < s->n_dimension; ++i) {
			/* The magical velocity formula */
			p->velocity[i] = 
				//rfrange(0, 1)
				+ s->phi * p->velocity[i]
				+ s->alpha * rfrange(0, 1) * (p->p_position[i]
						- p->position[i])
				+ s->beta * rfrange(0, 1) * (swarm_g_position_load(s, i)
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
		}
		swarm_g_compare_store(s, score, p->position);
		if (p->iter < s->n_iteration) {
			p->iter += 1;
			swarm_queue_push(s, p);
		}
	}
	return NULL;
}

float *
apso(
		size_t n_thread,
		size_t n_iteration,
		size_t n_particle,
		size_t n_dimension,
		int b_lo, int b_up,
		float phi, float alpha, float beta,
		int (*fitness)(float const *argv))
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
		s.pt[i].p_score = 0;
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
	for (i = 0; i < n_thread - 1; ++i) {
		if (pthread_create(&tid[i], NULL, apso_work, (void *) &s) != 0) {
			fprintf(stderr, "error: Could not create thread\n");
			exit(EXIT_FAILURE);
		}
	}
	apso_work(&s);
	/* End the swarm */
	for (i = 0; i < n_thread - 1; ++i) {
		if (pthread_join(tid[i], NULL)) {
			fprintf(stderr, "error: Could not join thread\n");
			exit(EXIT_FAILURE);
		}
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

int
hidamari_fitness(float const *position)
{
	HidamariGame game;
	double weight[3];

	weight[0] = position[0];
	weight[1] = position[1];
	weight[2] = position[2];
	hidamari_init(&game);
	do {
		hidamari_pso_update(&game, weight);
	} while (game.state == GS_GAME_PLAYING && game.field.score < 5000);
	return game.field.score;
}

int
distance_fitness(float const *position)
{
	return -sqrt(pow(10 - position[0], 2) + pow(10 - position[1], 2));

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
	best = apso(4, n_iteration, n_particle, 2, -100, 100, 0.3, 0.1, 0.6,
			distance_fitness);
	printf("best position: (%f, %f)\n",
			best[0],
			best[1]);
	free(best);
	return 0;
}
