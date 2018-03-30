/* See LICENSE file for copyright and license details */
#ifndef HEAP_H
#define HEAP_H

#include <stdlib.h>
#include <stdint.h>

#define HEAP_LEFT(i) (2 * (i) + 1)
#define HEAP_RIGHT(i) (2 * (i) + 2)
#define HEAP_PARENT(i) (((i) - 1) / 2)

#define HEAP_INSTANTIATE(NAME, T, ALLOC, CMP) \
typedef struct NAME NAME; \
struct NAME { \
	size_t len; \
	size_t size; \
	T *vec; \
}; \
\
static inline void \
NAME ## _move_up(NAME *heap) \
{ \
	size_t parent = HEAP_PARENT(heap->len - 1); \
	size_t child = 0; \
	T *data = heap->vec; \
	T swap; \
	\
	if (heap->len <= 1) \
		return; \
	for (;;) { \
		if (HEAP_LEFT(parent) < heap->len \
		&& 0 > CMP(data[HEAP_LEFT(parent)], data[parent])) { \
			child = HEAP_LEFT(parent); \
		} else if (HEAP_RIGHT(parent) < heap->len \
		&& 0 > CMP(data[HEAP_RIGHT(parent)], \
			data[parent])) { \
			child = HEAP_RIGHT(parent); \
		} else { \
			/* Neither child violates heap property */ \
			break; \
		} \
		/* Swap the child with it's parent */ \
		swap = data[parent]; \
		data[parent] = data[child]; \
		data[child] = swap; \
		\
		if (0 == parent) \
			break; \
		\
		parent = HEAP_PARENT(parent); \
	} \
} \
\
static inline void \
NAME ## _move_down(NAME *heap, size_t index) \
{ \
	size_t child = 0; \
	T *data = heap->vec; \
	T swap; \
	\
	for (;;) { \
		if (HEAP_RIGHT(index) >= heap->len \
		&& HEAP_LEFT(index) >= heap->len) { \
			break; \
		} else if (HEAP_RIGHT(index) >= heap->len) { \
			child = HEAP_LEFT(index); \
		} else if (HEAP_LEFT(index) >= heap->len) { \
			child = HEAP_RIGHT(index); \
		} else { \
			printf("foo\n"); \
			if (0 > CMP(data[HEAP_LEFT(index)], \
					  data[HEAP_RIGHT(index)])) { \
				child = HEAP_LEFT(index); \
			} else { \
				child = HEAP_RIGHT(index); \
			} \
		} \
		/* Swap the child with it's parent */ \
		swap = data[index]; \
		data[index] = data[child]; \
		data[child] = swap; \
		\
		index = child; \
	} \
} \
\
static inline NAME * \
NAME ## _create(size_t n) \
{ \
	NAME *ret = ALLOC(sizeof(NAME) + sizeof(T) * n); \
	\
	if (!ret) \
		return NULL; \
	memset(ret, 0, sizeof(NAME) + sizeof(T) * n); \
	ret->vec = (T *) (ret + 1); \
	ret->size = n; \
	return ret; \
} \
\
static inline void \
NAME ## _destroy(NAME *heap) \
{ \
	free(heap); \
} \
\
static inline int \
NAME ## _push(NAME *heap, T elem) \
{ \
	if (heap->len >= heap->size) \
		return -1; \
	heap->vec[heap->len++] = elem; \
	NAME ## _move_up(heap); \
	return 0; \
} \
\
static inline T \
NAME ## _pop(NAME *heap) \
{ \
	T ret = heap->vec[0]; \
	heap->vec[0] = heap->vec[--heap->len]; \
	NAME ## _move_down(heap, 0); \
	return ret; \
}

#endif
