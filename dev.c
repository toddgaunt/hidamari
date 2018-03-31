/* See LICENSE file for copyright and license details */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unistd.h>

#include "hidamari.h"
#include "heap.h"
#include "hashtable.h"

static inline size_t
hash(int i)
{
	size_t const p = 16777619;
	size_t hash = 2166136261u;

	hash = (hash ^ i) * p;
	hash += hash << 13;
	hash ^= hash >> 7;
	hash += hash << 3;
	hash ^= hash >> 17;
	hash += hash << 5;
	return hash;
}

static inline size_t
ht_hash(char const *str)
{
	size_t const p = 16777619;
 	size_t hash = 2166136261u;

	for (; *str; ++str)
		hash = (hash ^ *str) * p;
 	hash += hash << 13;
 	hash ^= hash >> 7;
 	hash += hash << 3;
 	hash ^= hash >> 17;
 	hash += hash << 5;
 	return hash;
 }

static int
intcmp(int a, int b)
{
	return a - b;
}

HEAP_INSTANTIATE(HeapStr, int8_t, malloc, intcmp)

static void
dump_field(HidamariPlayField *field)
{
	int i;
	size_t x, y;
	char buf[HIDAMARI_WIDTH][HIDAMARI_HEIGHT];

	for (x = 0; x < HIDAMARI_WIDTH; ++x) {
		for (y = 0; y < HIDAMARI_HEIGHT; ++y) {
			if (field->grid[y] & 1 << x) {
				buf[x][y] = '#';
			} else {
				buf[x][y] = '.';
			}
		}
	}
	printf("shape: %d\n", field->current.shape);
	for (i = 0; i < 4; ++i) {
		x = hidamari_orientation[field->current.shape]
		                              [field->current.orientation]
		                              [i].x + field->current.pos.x;
		y = hidamari_orientation[field->current.shape]
		                        [field->current.orientation]
		                        [i].y + field->current.pos.y;
		printf("x, y: %zu, %zu\n", x, y);
		buf[x][y] = '$';
	}
	printf("--\n");
	for (x = 0; x < HIDAMARI_WIDTH; ++x) {
		for (y = 0; y < HIDAMARI_HEIGHT; ++y) {
			putc(buf[x][y], stdout);
			putc(' ', stdout);
		}
		putc('\n', stdout);
	}
	printf("--\n");
}

int
main()
{
	HidamariBuffer buf;
	HidamariGame game;

	srand(time(NULL));
	hidamari_setup(&buf, &game);
	game.field.grid[0] |= 7 << 1;
	while(1) {
		//usleep(100000);
		hidamari_update(&buf, &game, BUTTON_NONE);
		dump_field(&game.field);
	}
}
