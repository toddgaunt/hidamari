/* See LICENSE file for copyright and license details */
#ifndef RALLOC_H
#define RALLOC_H

#include <stdlib.h>

#include "region.h"

void *ralloc_region;

void
ralloc_aquire(size_t n);

void
ralloc_release(void);

void *
ralloc(size_t n);

void
ralloc_clear(void);

#endif
