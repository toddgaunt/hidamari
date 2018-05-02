/* See LICENSE file for copyright and license details */
#ifndef VECTOR_H
#define VECTOR_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#define VEC_ZERO {0, 0, NULL}

#define VECTOR_INSTANTIATE(NAME, T, ALLOC) \
typedef struct NAME NAME; \
struct NAME { \
	size_t len; \
	size_t size; \
	T *data; \
}; \
\
static inline int \
NAME ## _push(NAME *table, T elem) \
{ \
} \
\
static inline T \
NAME ## _peek(NAME *table) \
{ \
} \
static inline T \
NAME ## _pop(NAME *table) \
{ \
} \
\
static inline V * \
NAME ## _get(NAME *table, K key) \
{ \
} \
\
static inline NAME * \
NAME ## _grow(NAME *table, size_t n) \
{ \
	size_t i; \
	NAME *new; \
	\
	/* Cannot resize without losing elements */ \
	if (table->used - table->tombed > n) \
		return table; \
	new = NAME ## _create(n); \
	for (i = 0; i < table->size; ++i) { \
		if (HT_USED != table->state[i]) \
			continue; \
		* NAME ## _get(new, table->key[i]) = table->val[i]; \
	} \
	NAME ## _destroy(table); \
	return new; \
}

#endif
