/* See LICENSE file for copyright and license details */
#include <stdlib.h>

#include "ralloc.h"
#include "region.h"

void *ralloc_region;

void
ralloc_aquire(size_t n)
{
	ralloc_region = region_create(n);
}

void
ralloc_release()
{
	region_destroy(ralloc_region);
}

void *
ralloc(size_t n)
{
	return region_alloc(ralloc_region, n);
}

void
ralloc_clear(void)
{
	region_clear(ralloc_region);
}
