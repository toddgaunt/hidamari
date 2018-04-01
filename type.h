#ifndef TYPE_H
#define TYPE_H

#include <stdlib.h>
#include <stdint.h>

/* Documentation types. */
typedef uint8_t u1;
typedef uint8_t u4;
typedef uint16_t u12;

/* Core types */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef size_t usize;

#define VEC2(x, y) {.raw = {(x), (y)}}

typedef struct {
	union {
		struct {
			int x;
			int y;
		};
		struct {
			int w;
			int h;
		};
		int raw[2];
	};
} Vec2;

#endif
