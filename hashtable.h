/* See LICENSE file for copyright and license details */
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#define HT_USED 1
#define HT_NONE 0
#define HT_TOMB -1 /* Used to denote deleted elements */

/* Instantiate a generic hashtable. The hashtable type name will be *NAME*,
 * with keys being of type *K*, and values being of type *V*. The allocator
 * for the hashtable will be *ALLOC*, which must match the type signature of
 * malloc. The hash function and comparison function used will be *HASH*
 * and *CMP* respectively
 *
 * e.g.
 *	static inline size_t
 *	hash(char const *str)
 *	{
 *		size_t const p = 16777619;
 *		size_t hash = 2166136261u;
 *
 *		for (; *str; ++str)
 *			hash = (hash ^ *str) * p;
 *		hash += hash << 13;
 *		hash ^= hash >> 7;
 *		hash += hash << 3;
 *		hash ^= hash >> 17;
 *		hash += hash << 5;
 *		return hash;
 *	}
 *
 *	HASHTABLE_INSTANTIATE(hts, char const *, bool, hash, strcmp)
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
#define HASHTABLE_INSTANTIATE(NAME, K, V, ALLOC, HASH, CMP) \
typedef struct NAME NAME; \
struct NAME { \
	int8_t *state; \
	K *key; \
	V *val; \
	size_t used; \
	size_t tombed; \
	size_t size; \
}; \
\
static inline size_t \
NAME ## _index(NAME const *table, K key) \
{ \
	size_t i; \
	size_t begin; \
	 \
	begin = HASH(key) % table->size; \
	i = begin; \
	while (HT_NONE != table->state[i]) { \
		if (HT_USED == table->state[i] && !CMP(table->key[i], key)) \
			break; \
		i = (i + 1) % table->size; \
		if (begin == i) \
			return table->size; \
	} \
	return i; \
} \
\
static inline NAME * \
NAME ## _create(size_t size) \
{ \
	assert(0 != size); \
	\
	NAME *table = ALLOC(sizeof(NAME) \
			+ sizeof(int8_t) * size \
			+ sizeof(K) * size \
			+ sizeof(V) * size); \
	\
	table->state = (int8_t *) (table + 1); \
	table->key = (K *) (table->state + size); \
	table->val = (V *) (table->key + size); \
	table->used = 0; \
	table->tombed = 0; \
	table->size = size; \
	memset(table->state, HT_NONE, sizeof(int8_t) * size); \
	return table; \
} \
\
static inline void \
NAME ## _destroy(NAME *table) \
{ \
	free(table); \
} \
\
static inline bool \
NAME ## _contains(NAME const *table, K key) \
{ \
	size_t index = NAME ## _index(table, key); \
	\
	if (index >= table->size \
	|| HT_USED != table->state[index]) \
		return false; \
	return true; \
} \
\
static inline V * \
NAME ## _get(NAME *table, K key) \
{ \
	size_t index = NAME ## _index(table, key); \
	\
	if (index >= table->size) \
		return NULL; \
	if (HT_USED != table->state[index]) { \
		table->used += 1; \
		table->state[index] = HT_USED; \
		table->key[index] = key; \
		memset(&table->val[index], 0, sizeof(V)); \
	} \
	return &table->val[index]; \
} \
\
static inline V * \
NAME ## _at(NAME const *table, K key) \
{ \
	size_t index = NAME ## _index(table, key); \
	\
	if (index >= table->size \
	|| HT_USED != table->state[index]) \
		return NULL; \
	return &table->val[index]; \
} \
\
static inline void \
NAME ## _delete(NAME *table, K key) \
{ \
	size_t index = NAME ## _index(table, key); \
	\
	if (index >= table->size \
	|| HT_USED != table->state[index]) \
		return; \
	table->state[index] = HT_TOMB; \
	table->tombed += 1; \
} \
\
static inline NAME * \
NAME ## _rehash(NAME *table, size_t n) \
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
