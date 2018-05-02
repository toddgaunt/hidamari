/* See LICENSE file for copyright and license details */
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "heap.h"
#include "region.h"

#include "apso_internal.h"

/* Thread-safe queue push() operation */
static int
swarm_queue_push(Swarm *s, Particle *p)
{
	int err;

	pthread_mutex_lock(&s->queue_lock);
	err = wq_push(s->queue, p);
	pthread_mutex_unlock(&s->queue_lock);
	return err;
}

/* Thread-safe queue pop() operation */
static int
swarm_queue_pop(Swarm *s, Particle **ret)
{
	int err;

	pthread_mutex_lock(&s->queue_lock);
	err = wq_pop(s->queue, ret);
	pthread_mutex_unlock(&s->queue_lock);
	return err;
}

/* Generate a random number between [lower, upper] */
static float
rfrange(int lower, int upper)
{
	lower *= 1000;
	upper *= 1000;

	return (float)((rand() % (upper + 1 - lower)) + lower) / 1000;
}

static float
swarm_g_position_load(Swarm *s, size_t index)
{
	float tmp;

	pthread_mutex_lock(&s->g_lock);
	tmp = s->g_position[index];
	pthread_mutex_unlock(&s->g_lock);
	return tmp;
}

static void
swarm_g_compare_store(Swarm *s, float score, float *position)
{
	pthread_mutex_lock(&s->g_lock);
	if (score > s->g_score) {
		s->g_score = score;
		memcpy(s->g_position, position, sizeof(float) * s->n_dimension);
	}
	pthread_mutex_unlock(&s->g_lock);
}

Swarm *
swarm_create(
		size_t n_iteration,
		size_t n_particle,
		size_t n_dimension,
		int b_lo, int b_up,
		float phi, float alpha, float beta,
		float (*fitness)(float const *argv))
{
	size_t i, j;
	Swarm *ret;
	void *r = region_create(
		sizeof(*ret)
		+ sizeof(*ret->g_position) * n_dimension
		+ sizeof(*ret->pt) * n_particle
		+ sizeof(*ret->pt[0].p_position) * n_dimension * n_particle
		+ sizeof(*ret->pt[0].position) * n_dimension * n_particle
		+ sizeof(*ret->pt[0].velocity) * n_dimension * n_particle);

	ret = region_alloc(r, sizeof(*ret));
	/* Configure the swarm */
	ret->n_iteration = n_iteration;
	ret->n_dimension = n_dimension;
	ret->b_lo = b_lo;
	ret->b_up = b_up;
	ret->alpha = alpha;
	ret->beta = beta;
	ret->phi = phi;
	ret->fitness = fitness;
	ret->g_score = -INFINITY;
	ret->g_position = region_alloc(r, sizeof(*ret->g_position) * n_dimension);
	ret->n_pt = n_particle;
	ret->pt = region_alloc(r, sizeof(*ret->pt) * n_particle);
	ret->queue = wq_create(n_particle);
	pthread_mutex_init(&ret->queue_lock, NULL);
	pthread_mutex_init(&ret->g_lock, NULL);

	/* Initialize the swarm */
	for (i = 0; i < n_particle; ++i) {
		ret->pt[i].id = i;
		ret->pt[i].iter = 1;
		ret->pt[i].p_score = -INFINITY;
		ret->pt[i].p_position = region_alloc(r,
				sizeof(*ret->pt[i].p_position) * n_dimension);
		ret->pt[i].position = region_alloc(r,
				sizeof(*ret->pt[i].position) * n_dimension);
		ret->pt[i].velocity = region_alloc(r,
				sizeof(*ret->pt[i].velocity) * n_dimension);
		for (j = 0; j < n_dimension; ++j) {
			ret->pt[i].position[j] = rfrange(b_lo, b_up);
			ret->pt[i].p_position[j] = ret->pt[i].position[j];
		}
		for (j = 0; j < n_dimension; ++j) {
			ret->pt[i].velocity[j] = rfrange(-abs(b_up - b_lo),
						    abs(b_up - b_lo));
		}
		swarm_queue_push(ret, &ret->pt[i]);
	}
	return ret;
}

void
swarm_destroy(Swarm *s)
{
	wq_destroy(s->queue);
	region_destroy(((uint8_t *)s) - region_size());
}

void *
apso_work(void *arg)
{
	size_t i;
	Particle *p;
	Swarm *s = arg;
	float score;

	/* Continously move and evaluate the particles */
	while (0 == swarm_queue_pop(s, &p)) {
		/* Update the particle's velocity */
		for (i = 0; -INFINITY != s->g_score && i < s->n_dimension; ++i) {
			/* The magical velocity formula */
			p->velocity[i] = 
				+ s->phi * p->velocity[i]
				+ s->alpha * rfrange(0, 1)
					* (p->p_position[i]
					- p->position[i])
				+ s->beta * rfrange(0, 1)
					* (swarm_g_position_load(s, i)
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
		if (0 == s->n_iteration || p->iter < s->n_iteration) {
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
		float (*fitness)(float const *argv))
{
	size_t i;
	Swarm *s;
	pthread_t *tid;
	float *ret;

	tid = malloc(sizeof(*tid) * n_thread);
	s = swarm_create(n_iteration, n_particle, n_dimension, b_lo, b_up,
			phi, alpha, beta, fitness);

	/* Begin the swarm */
	for (i = 0; i < n_thread - 1; ++i) {
		if (pthread_create(&tid[i], NULL, apso_work, (void *) s) != 0) {
			fprintf(stderr, "error: Could not create thread\n");
			exit(EXIT_FAILURE);
		}
	}
	apso_work(s);
	/* End the swarm */
	for (i = 0; i < n_thread - 1; ++i) {
		if (pthread_join(tid[i], NULL)) {
			fprintf(stderr, "error: Could not join thread\n");
			exit(EXIT_FAILURE);
		}
	}
	free(tid);
	ret = malloc(sizeof(*ret) * s->n_dimension);
	memcpy(ret, s->g_position, sizeof(*ret) * s->n_dimension);
	swarm_destroy(s);
	return ret;
}
