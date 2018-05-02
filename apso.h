/* See LICENSE file for copyright and license details */
#ifndef APSO_H
#define APSO_H

#include <stdlib.h>

float *
apso(size_t n_thread,
     size_t n_iteration,
     size_t n_particle,
     size_t n_dimension,
     int b_lo, int b_up,
     float phi, float alpha, float beta,
     float (*fitness)(float const *argv));

float *
apso_vis(
		size_t n_thread,
		size_t n_iteration,
		size_t n_particle,
		size_t n_dimension,
		int b_lo, int b_up,
		float phi, float alpha, float beta,
		float (*fitness)(float const *argv));

#endif
