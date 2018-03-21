#ifndef TYPE_H
#define TYPE_H

#include <stdint.h>

typedef uint8_t u4;
typedef uint8_t u8;
typedef uint16_t u10;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;

#define VEC2(x, y) (Vec2){.raw = {(x), (y)}}

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
