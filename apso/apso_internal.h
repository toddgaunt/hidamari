/* See LICENSE file for copyright and license details */
#ifndef APSO_INTERNAL_H
#define APSO_INTERNAL_H

#include <pthread.h>
#include <stdlib.h>

#include "heap.h"

typedef struct Particle Particle;
typedef struct Swarm Swarm;
typedef struct wq WorkQueue;

struct Particle {
	int id;
	size_t iter;
	float p_score;
	float *p_position;
	float *position;
	float *velocity;
};

struct Swarm{
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
};

static int
particle_cmp(Particle const *a, Particle const *b)
{
	return a->iter - b->iter;
}

HEAP_INSTANTIATE(wq, Particle *, malloc, particle_cmp)

Swarm *
swarm_create(
		size_t n_iteration,
		size_t n_particle,
		size_t n_dimension,
		int b_lo, int b_up,
		float phi, float alpha, float beta,
		float (*fitness)(float const *argv));

void
swarm_destroy(Swarm *s);

void *
apso_work(void *arg);

#endif
