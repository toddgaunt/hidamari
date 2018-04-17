/* See LICENSE file for copyright and license details */
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#define VEC_ZERO {0, 0, NULL}

/* Instantiate a generic vector. The hashtable type name will be *NAME*,
 * with keys being of type *K*, and values being of type *V*. The allocator
 * for the hashtable will be *ALLOC*, which must match the type signature of
 * malloc. The hash function and comparison function used will be *HASH*
 * and *CMP* respectively
 *
 * e.g.
 *	HASHTABLE_INSTANTIATE(stack, int, malloc)
 *
 *	int
 *	main()
 *	{
 *		hts *table;
 *
 *		table = hts_create(32);
 *		*hts_get(table, "hello") = true;
 *		hts_delete(table, "hello");
 *		printf("used: %zu, tombed: %zu\n", table->used, table->tombed);
 *	}
 * */
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
