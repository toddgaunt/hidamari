/* See LICENSE file for copyright and license details */
#ifndef AI_H
#define AI_H

#include "hidamari.h"

enum {
	AI_THREAD_START,
	AI_THREAD_DONE,
	AI_THREAD_TERMINATE,
};

/* Compute the minimum size of the region needed by ai_plan() */
size_t
ai_size();

/* Perform a depth-first search on the state-space of tetris until exhaustion.
 * Each state that reaches the depth bound of DEPTH is evaluated, and compared
 * against the current best state. One the search completes, a plan is made for
 * the state that evaluated to the lowest score.
 *
 * Parameters:
 *	- region: A pre-allocated memory region for the search to use. If not
 *	a sufficient size, the search will fail.
 *	- init: The initial state for the AI to search from.
 *
 * Return: An array of button inputs devised by the AI in order to achieve
 *	at a desirable state.
 */
button const *
ai_plan(void *region, double weight[3], struct field const *init);

#endif
