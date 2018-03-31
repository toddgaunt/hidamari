/* See LICENSE file for copyright and license details */
#ifndef REGION_H
#define REGION_H

#include <stdlib.h>

void *
region_alloc(void *region, size_t n);

/* Reset a region for re-use */
void
region_clear(void *region);

void *
region_create(size_t n);

void
region_destroy(void const *region);

#endif
