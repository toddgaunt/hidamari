/* See LICENSE file for copyright and license details */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hidamari.h"
#include "apso.h"

char *argv0;

void
usage()
{
	fprintf(stderr, "usage: %s <number of particles> <number of iterations>\n", argv0);
	exit(EXIT_FAILURE);
}

float
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
	} while (game.state == GS_GAME_PLAYING && game.field.lines < 60000);
	return game.field.lines;
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
	best = apso(4, n_iteration, n_particle, 3, 0, 1, 0.4, 0.1, 0.2,
			hidamari_fitness);
	printf("best position: (%f, %f, %f)\n",
			best[0],
			best[1],
			best[2]);
	free(best);
	return 0;
}
