/* See LICENSE file for copyright and license details */
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdlib.h>
#include <stdbool.h>

#define HT_TOMB ((void *)-1) /* Used to denote deleted elements */

#define HASHTABLE_INSTANTIATE(NAME, K, V, ALLOC, HASH, CMP) \
typedef struct NAME NAME; \
struct NAME {K *key; V *val; size_t used; size_t tombed; size_t size;}; \
static inline size_t \
NAME ## _index(NAME const *table, K key) \
{ \
	size_t i; \
	 \
	if (table->used == table->size) \
		return table->size; \
	i = HASH(key) % table->size; \
	while (NULL != table->key[i] && HT_TOMB != table->key[i]) { \
		if (!CMP(table->key[i], key)) \
			break; \
		i = (i + 1) % table->size; \
	} \
	return i; \
} \
\
static inline NAME * \
NAME ## _create(size_t size) \
{ \
	NAME *table = ALLOC(sizeof(NAME) \
			+ sizeof(K) * size \
			+ sizeof(V *) * size); \
	\
	table->key = (K *) (table + 1); \
	table->val = (V *) (table->key + size); \
	table->used = 0; \
	table->tombed = 0; \
	table->size = size; \
	memset(table->key, 0, sizeof(K) * size); \
	memset(table->val, 0, sizeof(V) * size); \
	return table; \
} \
\
static inline void \
NAME ## _destroy(NAME *table) \
{ \
	free(table); \
} \
\
static inline V \
NAME ## _find(NAME const *table, K key) \
{ \
	size_t index = NAME ## _index(table, key); \
	\
	if (index >= table->size \
	|| NULL == table->val[index] \
	|| HT_TOMB == table->val[index]) \
		return NULL; \
	return table->val[index]; \
} \
\
static inline V * \
NAME ## _get(NAME *table, K key) \
{ \
	size_t index = NAME ## _index(table, key); \
	\
	if (index >= table->size) \
		return NULL; \
	table->key[index] = key; \
	table->used += 1; \
	return &table->val[index]; \
} \
\
static inline V * \
NAME ## _at(NAME const *table, K key) \
{ \
	size_t index = NAME ## _index(table, key); \
	\
	if (index >= table->size) \
		return NULL; \
	return &table->val[index]; \
} \
\
static inline bool \
NAME ## _insert(NAME *table, K key, V val) \
{ \
	V *pp = NAME ## _get(table, key); \
	\
	if (!pp || *pp) \
		return false; \
	*pp = val; \
	return true; \
} \
\
static inline \
void \
NAME ## _delete(NAME *table, K key) \
{ \
	V *pp = NAME ## _at(table, key); \
	\
	if (!pp || !*pp) \
		return; \
	*pp = HT_TOMB; \
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
	if (table->used - table->tombed <= n) \
		return table; \
	new = NAME ## _create(n); \
	for (i = 0; i < table->size; ++i) { \
		if (NULL == table->key[i] || HT_TOMB == table->key[i]) \
			continue; \
		NAME ## _insert(new, table->key[i], table->val[i]); \
	} \
	NAME ## _destroy(table); \
	table = new; \
	return table; \
} \

#endif
