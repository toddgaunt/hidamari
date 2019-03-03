#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
	
#include "ai.h"
#include "field.h"

#define PLAN_DEPTH 1
#define DEPTH 2

struct field_node {
	size_t g;
	size_t n_action;
	enum button *action;
	struct field field;
	struct field_node *parent;
	struct field_node *next;
};

/* Allocate _size_ bytes from a buffer, and shift the buffer over */
static void *
balloc(void **buffer, size_t size)
{
	void *mem = *buffer;
	*(uint8_t **)buffer += size;
	return mem;
}

/* Allocate a new node with a copy of _init_ as its state */
struct field_node *
mknode(void **region, struct field const *init)
{
	struct field_node *ret;

	ret = balloc(region, sizeof(*ret));
	memset(ret, 0, sizeof(*ret));
	memcpy(&ret->field, init, sizeof(struct field));
	return ret;
}

/* Derive a new state node from a parent node. The child will reflect
 * the state of the parent after taking _n_action_ actions.
 */
void
derive(void **region, struct field_node **stackp, struct field_node *parent,
		size_t n_action, enum button *action)
{
	size_t i;
	struct field_node *child = mknode(region, &parent->field);

	child->n_action = n_action;
	child->action = action;
	child->parent = parent;
	child->g = parent->g + 1;
	for (i = 0; i < n_action; ++i) {
		field_update(&child->field, action[i]);
	}
	child->next = *stackp;
	*stackp = child;
}

/* Expands a fieldnode by branching out on all possible permutations of the
 * current falling piece.
 *
 * Returns 0 if successful, or -1 if it runs out of memory.
 */
void
expand(void **region, struct field_node **stackp, struct field_node *parent)
{
	size_t i, j;
	enum button *tmp;

	for (i = 0; i < 3; ++i) {
		for (j = 0; j < 6; ++j) {
			tmp = balloc(region, i + j + 1);
			memset(tmp, BTN_R, i);
			memset(tmp + i, BTN_RIGHT, j);
			tmp[i + j] = BTN_B;
			derive(region, stackp, parent, i + j + 1, tmp);
			tmp = balloc(region, i + j + 1);
			memset(tmp, BTN_R, i);
			memset(tmp + i, BTN_LEFT, j);
			tmp[i + j] = BTN_B;
			derive(region, stackp, parent, i + j + 1, tmp);
		}
	}
}

/* Heuristic 1: Calculate the aggregate difference in height between all
 * columns.
 */
static int
h1(struct field *field)
{
	int top;
	size_t i, j;
	int score = 0;
	int heights[FIELD_WIDTH];

	for (i = 0; i < FIELD_WIDTH - 1; ++i) {
		for (j = 0; j < FIELD_HEIGHT; ++j) {
			if ((field->bitboard[j] & (2 << i)) == (2 << i)) {
				top = j;
			}
		}
		heights[i] = top;
	}
	for (i = 0; i < 9; ++i) {
		score += abs(heights[i] - heights[i + 1]);
	}
	return score;
}

/* Heuristic 2: Calculate the aggregate height of all columns. */
static int
h2(struct field *field)
{
	int top;
	size_t i, j;
	int score = 0;

	for (i = 2; i < (1 << (FIELD_WIDTH - 1)); i <<= 1) {
		for (j = 0; j < FIELD_HEIGHT; ++j) {
			if ((field->bitboard[j] & i) == i) {
				top = j;
			}
		}
		score += top;
	}
	return score;
}

/* Heuristic 3: Calculate the number of holes on the field. A hole is defined
 * as any open space with a filled space above it in the same column.
 */
static int
h3(struct field *field)
{
	int cnt;
	size_t i, j;
	int score = 0;

	for (i = 0; i < FIELD_WIDTH - 1; ++i) {
		cnt = 0;
		for (j = 0; j < FIELD_HEIGHT; ++j) {
			if ((field->bitboard[j] & (2 << i)) == (2 << i)) {
				score += cnt;
				cnt = 0;
			} else {
				cnt += 1;
			}
		}
	}
	return score;
}

/* Main evaluation function for a given state. Each of the heuristics
 * is multiplied by a certain weight depending on how valuable it is deemed.
 */
static uint32_t
eval(struct field *field, double weight[3])
{
	int score = 0;

    if (!field)
        return UINT32_MAX;

	score += weight[0] * h1(field);
	score += weight[1] * h2(field);
	score += weight[2] * h3(field);
	return score;
}

/*
 *
 * The Main loop and plan deviser.
 *
 */

/* Given a struct field_node, trace back up the tree it was created from to allocate
 * and return a vector of actions that must be taken in order to achieve the
 * goal state from the initial state fed into the program.
 */
static enum button *
mkplan(void **region, struct field_node *goal)
{
	enum button *planstr;
	struct field_node *at;
	size_t n_move = 0;
	size_t i;

	/* Move up the tree to the node where the plan will begin to be made */
	for (i = 0; i < DEPTH - PLAN_DEPTH; ++i) {
		goal = goal->parent;
	}
	for (at = goal; at->parent; at = at->parent) {
		n_move += at->n_action;
	}
	planstr = balloc(region, n_move + 1);
	planstr[n_move--] = BTN_NONE;
	for (at = goal; at; at = at->parent) {
		for (i = 0; i < at->n_action; ++i) {
			planstr[n_move - i] = at->action[at->n_action - i - 1];
		}
		n_move -= at->n_action;
	}
	return planstr;
}

size_t
ai_size()
{
	return pow(36, DEPTH) * (sizeof(struct field_node) + 10);
}

enum button const *
ai_plan(void *region, double weight[3], struct field const *init) {
	struct field_node *stack = NULL;
	struct field_node *goal = NULL;
	struct field_node *at = NULL;

	stack = mknode(&region, init);
	goal = NULL;
	while (stack) {
		at = stack;
		stack = stack->next;
		if (DEPTH == at->g) {
            /* Higher scores means worse fitness */
			if (eval(&at->field, weight) <= eval(&goal->field, weight))
				goal = at;
		} else {
		    expand(&region, &stack, at);
		}
	}
	return mkplan(&region, goal);
}
