#ifndef VEC2_H
#define VEC2_H

#define VEC2(x, y) {.this = {(x), (y)}}

struct vec2 {
	union {
		int this[2];
		struct {
			int x;
			int y;
		};
		struct {
			int w;
			int h;
		};
	};
};

static struct vec2 vec2_N = {.this = {0, 1}};
static struct vec2 vec2_E = {.this = {1, 0}};
static struct vec2 vec2_S = {.this = {0, -1}};
static struct vec2 vec2_W = {.this = {-1, 0}};
static struct vec2 vec2_1 = {.this = {1, 1}};
static struct vec2 vec2_0 = {.this = {0, 0}};

int
vec2_angle(struct vec2 a, struct vec2 b);

int
vec2_distance(struct vec2 a, struct vec2 b);

struct vec2
vec2_dot(struct vec2 a, struct vec2 b);

struct vec2
vec2_lerp(struct vec2 a, struct vec2 b, int t);

int
vec2_magnitude(struct vec2 a);

int
vec2_sqrmagnitude(struct vec2 a);

struct vec2
vec2_moveto(struct vec2 a, struct vec2 b, int maxdelta);

struct vec2
vec2_normalize(struct vec2 a);

struct vec2
vec2_add(struct vec2 a, int s);

struct vec2
vec2_sub(struct vec2 a, int s);

struct vec2
vec2_mul(struct vec2 a, int s);

struct vec2
vec2_div(struct vec2 a, int s);

#endif
