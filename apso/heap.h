/* See LICENSE file for copyright and license details */
#ifndef HEAP_H
#define HEAP_H

#include <stdlib.h>
#include <stdint.h>

#define HEAP_INSTANTIATE(NAME, T, ALLOC, CMP) \
typedef struct NAME NAME; \
struct NAME { \
	size_t len; \
	size_t size; \
	T *vec; \
}; \
\
static inline void \
NAME ## _move_up(NAME *heap, size_t index) \
{ \
	size_t smallest; \
	size_t left; \
	size_t right; \
	T swap; \
	\
	while (index >= 1) { \
		smallest = index; \
		left = 2 * index; \
		right = 2 * index + 1; \
		if (left < heap->len \
		&& 0 > CMP(heap->vec[left], heap->vec[smallest])) \
			smallest = left; \
		if (right < heap->len \
		&& 0 > CMP(heap->vec[right], heap->vec[smallest])) \
			smallest = right; \
		if (smallest == index) \
			break; \
		swap = heap->vec[index]; \
		heap->vec[index] = heap->vec[smallest]; \
		heap->vec[smallest] = swap; \
		index = index / 2; \
	} \
} \
\
static inline void \
NAME ## _move_down(NAME *heap, size_t index) \
{ \
	size_t smallest; \
	size_t left; \
	size_t right; \
	T swap; \
	\
	for (;;) { \
		smallest = index; \
		left = 2 * index; \
		right = 2 * index + 1; \
		if (left <= heap->len \
		&& 0 > CMP(heap->vec[left], heap->vec[smallest])) \
			smallest = left; \
		if (right <= heap->len \
		&& 0 > CMP(heap->vec[right], heap->vec[smallest])) \
			smallest = right; \
		if (smallest == index) \
			break; \
		swap = heap->vec[index]; \
		heap->vec[index] = heap->vec[smallest]; \
		heap->vec[smallest] = swap; \
		index = smallest; \
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
	ret->vec = (T *) (ret + 1); \
	ret->size = n; \
	ret->len = 0; \
	/* Decrement the array by one, since index 0 will never be used */ \
	ret->vec -= 1; \
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
	heap->vec[++heap->len] = elem; \
	NAME ## _move_up(heap, heap->len / 2); \
	return 0; \
} \
\
static inline int \
NAME ## _pop(NAME *heap, T *ret) \
{ \
	if (0 == heap->len) \
		return -1; \
	*ret = heap->vec[1]; \
	heap->vec[1] = heap->vec[heap->len--]; \
	NAME ## _move_down(heap, 1); \
	return 0; \
}

#endif
