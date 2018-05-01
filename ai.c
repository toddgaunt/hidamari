#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
	
#include "ai.h"
#include "hidamari.h"
#include "region.h"

#define DEPTH 2
#define DEPTH_IGNORED (DEPTH - DEPTH)

FieldNode *
create_node(void *region, HidamariPlayField const *init)
{
	FieldNode *ret;

	ret = region_alloc(region, sizeof(*ret));
	if (!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	memcpy(&ret->field, init, sizeof(HidamariPlayField));
	return ret;
}

int
derive(void *region, FieldNode **stackp, FieldNode *parent,
		size_t n_action, Button *action)
{
	size_t i;
	FieldNode *child = create_node(region, &parent->field);

	if (!child)
		return -1;
	child->n_action = n_action;
	child->action = action;
	child->parent = parent;
	child->g = parent->g + 1;
	for (i = 0; i < n_action; ++i) {
		hidamari_field_update(&child->field, action[i]);
	}
	child->next = *stackp;
	*stackp = child;
	return 0;
}

/* Expands a fieldnode by branching out on all possible permutations of the
 * current hidamari.
 *
 * Returns 0 if successful, or -1 if it runs out of memory.
 */
int
expand(void *region, FieldNode **stackp, FieldNode *parent)
{
	size_t i, j;
	Button *tmp;

	for (i = 0; i < 3; ++i) {
		for (j = 0; j < 6; ++j) {
			tmp = region_alloc(region, i + j + 1);
			memset(tmp, BUTTON_R, i);
			memset(tmp + i, BUTTON_RIGHT, j);
			tmp[i + j] = BUTTON_B;
			if (0 > derive(region, stackp, parent, i + j + 1, tmp))
				return -1;
			tmp = region_alloc(region, i + j + 1);
			memset(tmp, BUTTON_R, i);
			memset(tmp + i, BUTTON_LEFT, j);
			tmp[i + j] = BUTTON_B;
			if (0 > derive(region, stackp, parent, i + j + 1, tmp))
				return -1;
		}
	}
	return 0;
}

/* ---
 * Heuristics to evaluate how good a state is.
 */

/* Heuristic 1: Calculate the aggregate difference in height between all
 * columns.
 */
static int
h1(FieldNode *np)
{
	int top;
	size_t i, j;
	int score = 0;
	int heights[10];

	for (i = 0; i < HIDAMARI_WIDTH - 1; ++i) {
		for (j = 0; j < HIDAMARI_HEIGHT; ++j) {
			if ((np->field.grid[j] & (2 << i)) == (2 << i)) {
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
h2(FieldNode *np)
{
	int top;
	size_t i, j;
	int score = 0;

	for (i = 2; i < (1 << (HIDAMARI_WIDTH - 1)); i <<= 1) {
		for (j = 0; j < HIDAMARI_HEIGHT; ++j) {
			if ((np->field.grid[j] & i) == i) {
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
h3(FieldNode *np)
{
	int cnt;
	size_t i, j;
	int score = 0;

	for (i = 0; i < HIDAMARI_WIDTH - 1; ++i) {
		cnt = 0;
		for (j = 0; j < HIDAMARI_HEIGHT; ++j) {
			if ((np->field.grid[j] & (2 << i)) == (2 << i)) {
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
static int
evaluate(FieldNode *np)
{
	int score = 0;

	score += 3 * h1(np);
	score += 2 * h2(np);
	score += 10 * h3(np);
	return score;
}

/* ---
 * The Main loop and plan deviser.
 */

/* Given a FieldNode, trace back up the tree it was created from to allocate
 * and return a vector of actions that must be taken in order to achieve the
 * goal state from the initial state fed into the program.
 */
static Button *
mkplan(void *region, FieldNode *goal)
{
	Button *planstr;
	FieldNode *fp;
	size_t n_move = 0;
	size_t i;

	for (i = 0; i < DEPTH_IGNORED; ++i) {
		goal = goal->parent;
	}
	for (fp = goal; fp->parent; fp = fp->parent) {
		n_move += fp->n_action + 2;
	}
	planstr = region_alloc(region, n_move + 1);
	planstr[n_move--] = BUTTON_NONE;
	for (fp = goal; fp; fp = fp->parent) {
		planstr[n_move--] = BUTTON_DOWN;
		planstr[n_move--] = BUTTON_DOWN;
		for (i = 0; i < fp->n_action; ++i) {
			planstr[n_move - i] = fp->action[fp->n_action - i - 1];
		}
		n_move -= fp->n_action;
	}
	return planstr;
}

size_t
ai_size_requirement()
{
	return pow(36, DEPTH) * (sizeof(FieldNode) + 10);
}

Button const *
ai_plan(void *region, HidamariPlayField const *init) {
	FieldNode *stack;
	FieldNode *goal;
	FieldNode *fp;

	stack = create_node(region, init);
	goal = NULL;
	while (stack) {
		fp = stack;
		stack = stack->next;
		if (DEPTH == fp->g) {
			/* Evaluate the current goal state for "goodness" */
			if (!goal) {
				goal = fp;
			} else if (evaluate(fp) < evaluate(goal)) {
				goal = fp;
			}
		} else {
			if (0 > expand(region, &stack, fp))
				return NULL;
		}
	}
	return mkplan(region, goal);
}
