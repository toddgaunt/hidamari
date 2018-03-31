/* See LICENSE file for copyright and license details */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "region.h"

typedef struct {
	size_t sp;
	size_t size;
	uint8_t *mem;
} Region;

static bool
overflows(size_t a, size_t b)
{
	return a > SIZE_MAX - b ? true: false;
}

void *
region_alloc(void *handle, size_t n)
{
	void *ret;
	Region *region = handle;

	if (overflows(region->sp, n) || region->sp + n > region->size)
		return NULL;
	ret = &region->mem[region->sp];
	region->sp += n;
	return ret;
}

void
region_clear(void *handle)
{
	Region *region = handle;

	region->sp = 0;
}

void *
region_create(size_t n)
{
	Region *region = malloc(sizeof(*region) + n);

	region->mem = (uint8_t *) (region + 1);
	region->sp = 0;
	region->size = n;
	return region;
}

void
region_destroy(void const *region)
{
	free((void *)region);
}
