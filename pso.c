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
#include "region.h"

char const *argv0;

typedef struct {
	int best_score;
	double best_weight[3];
	double weight[3];
	double velocity[3];
} Particle;

/* Swarm velocity function weights */
double const phi = 0.4; /* Inertia from original velocity */
double const alpha = 0.1; /* Inertia from personal best */
double const beta = 0.2; /* Inertia from the swarm's best */
/* Search range bounds */
int const b_lo = 1;
int const b_up = 50;
/* The swarm */
pthread_mutex_t swarm_lock;
atomic_int best_score_swarm = 0;
double best_weight_swarm[3] = {0};
Particle *p;
/* Arguments */
size_t n_iter;
size_t n_particle;

void
usage()
{
	fprintf(stderr, "usage: %s <number of particles> <number of iterations>\n", argv0);
	exit(EXIT_FAILURE);
}

void
display()
{
	int i, j;
	char buf[b_up][b_up];
	memset(buf, ' ', b_up * b_up);
	for (i = 0; i < (int)n_particle; ++i) {
		buf[(int)p[i].weight[0] % b_up]
		   [(int)p[i].weight[1] % b_up] = 48 + i;
	}
	for (i = 0; i < b_up; ++i) {
		for (j = 0; j < b_up; ++j) {
			putc(buf[i][j], stdout);
			putc(' ', stdout);
		}
		putc('\n', stdout);
	}
}

/* The fitness function for each particle. The fitness score is determined
 * by the in-game score the particle can achieve for playing for as long
 * as possible
 */
size_t
fitness(double weight[3])
{
	HidamariGame game;
	hidamari_init(&game);
	do {
		hidamari_pso_update(&game, weight);
	} while (game.state == GS_GAME_PLAYING);
	return game.field.score;
}

/* Generate a random number between [lower, upper] */
int
rfrange(int lower, int upper)
{
	return (rand() % (upper + 1 - lower)) + lower;
}

/* Simply a helper function to ensure thread safety when updating the global
 * swarm values
 */
void
update_swarm_weight(int score, double weight[3])
{
	pthread_mutex_lock(&swarm_lock);
	memcpy(best_weight_swarm, weight, sizeof(best_weight_swarm));
	atomic_store(&best_score_swarm, score);
	pthread_mutex_unlock(&swarm_lock);
}

void *
pso_work(void *arg)
{
	size_t i;
	int tmp;
	double rp, rg;
	size_t n = n_iter;
	size_t index = (size_t)arg;
	Particle *pi = &p[index];

	/* Initialize the particle */
	for (i = 0; i < 3; ++i) {
		pi->weight[i] = rfrange(b_lo, b_up);
		pi->best_weight[i] = pi->weight[i];
	}
	tmp = fitness(pi->weight);
	printf("particle %zu:%zu (%f, %f, %f) = %d\n",
			index,
			n_iter - n,
			pi->weight[0],
			pi->weight[1],
			pi->weight[2],
			tmp);
	if (tmp > atomic_load(&best_score_swarm))
		update_swarm_weight(tmp, pi->weight);
	for (i = 0; i < 3; ++i) {
		pi->velocity[i] = rfrange(-abs(b_up - b_lo),
					    abs(b_up - b_lo));
	}
	display();
	/* Continously move and evaluate the particle */
	while (--n) {
		pthread_mutex_lock(&swarm_lock);
		/* Update the particle's velocity */
		for (i = 0; i < 3; ++i) {
			rp = (double)rfrange(1, 100) / 100.0;
			rg = (double)rfrange(1, 100) / 100.0;
			/* The magical velocity formula */
			pi->velocity[i] = phi * pi->velocity[i]
				+ alpha * rp * (pi->best_weight[i]
						- pi->weight[i])
				+ beta * rg * (best_weight_swarm[i]
						- pi->weight[i]);
		}
		pthread_mutex_unlock(&swarm_lock);
		/* Update the particle's position */
		for (i = 0; i < 3; ++i)
			pi->weight[i] += pi->velocity[i];
		/* Evaluate the new fitness of the particle */
		tmp = fitness(pi->weight);
		printf("particle %zu:%zu (%f, %f, %f) = %d\n",
				index,
				n_iter - n,
				pi->weight[0],
				pi->weight[1],
				pi->weight[2],
				tmp);
		if (tmp > pi->best_score) {
			memcpy(pi->best_weight, pi->weight,
				sizeof(pi->best_weight));
			pi->best_score = tmp;
			if (tmp > atomic_load(&best_score_swarm))
				update_swarm_weight(tmp, pi->weight);
		}
		display();
	}
	return NULL;
}

/* Particle Swarm Optimization for Hidamari. The three dimensions being
 * optimized are the weights for each heuristic used by the AI to play
 * the game.
 */
int
main(int argc, char **argv)
{
	size_t i;
	pthread_t *thread;

	argv0 = argv[0];
	if (argc != 3)
		usage();
	n_particle = strtol(argv[1], NULL, 10);
	n_iter = strtol(argv[2], NULL, 10);
	p = malloc(sizeof(*p) * n_particle);
	thread = malloc(sizeof(pthread_t) * n_particle);
	pthread_mutex_init(&swarm_lock, NULL);
	srand(time(NULL));
	/* Start the swarm */
	for (i = 0; i < n_particle; ++i) {
		if(pthread_create(&thread[i], NULL, pso_work, (void *) i) != 0) {
			fprintf(stderr, "error: Could not create thread\n");
			exit(EXIT_FAILURE);
		}
	}
	/* End the swarm */
	for (i = 0; i < n_particle; ++i) {
		if (pthread_join(thread[i], NULL)) {
			fprintf(stderr, "error: Could not join thread\n");
			exit(EXIT_FAILURE);
		}
	}
	free(p);
	free(thread);
	printf("best particle: (%f, %f, %f) = %d\n",
		best_weight_swarm[0],
		best_weight_swarm[1],
		best_weight_swarm[2],
		best_score_swarm);
	return EXIT_SUCCESS;
}
