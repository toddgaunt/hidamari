
float
vec2_angle(struct vec2 a, struct vec2 b)
{
}

float
vec2_distance(struct vec2 a, struct vec2 b)
{
}

struct vec2
vec2_dot(struct vec2 a, struct vec2 b)
{
}

struct vec2
vec2_lerp(struct vec2 a, struct vec2 b, float t)
{
}

float
vec2_magnitude(struct vec2 a)
{
}

float
vec2_sqrmagnitude(struct vec2 a)
{
}

struct vec2
vec2_moveto(struct vec2 a, struct vec2 b, float maxdelta)
{
}

struct vec2
vec2_normalized(struct vec2 a)
{
}

struct vec2
vec2_times(struct vec2 a, float s)
{
	a.x *= s;
	a.y *= s;
	return  a;
}
