/**
 * This file belongs to the 'Battlegrounds 3' game project.
 * Copyright 2009 xfacter
 * Copyright 2016 wickles
 * This work is licensed under the LGPLv3
 * subject to all terms as reproduced in the included LICENSE file.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define ASTAR_MAX_COST 10

/* TODO:
	- add considerations for diagonal movement
*/

typedef struct astar_path {
	int length;
	int max_length;
	int* nodes;
} astar_path;

typedef struct astar_node {
	unsigned int cost : 5;
	unsigned int open : 1;
	unsigned int closed : 1; //static
	unsigned int used : 1;
	int came_from;
	int g_score;
	int f_score;
} astar_node;

typedef struct astar {
	int width;
	int height;
	astar_node* nodes;
	int heap_max;
	int* open_heap;
} astar;

astar* astar_create(int width, int height, int heap_max);

void astar_free(astar* as);

astar_path* astar_create_path(int max_length);

void astar_free_path(astar_path* path);

void astar_find_path(astar_path* path, astar* as, int start, int goal);

int astar_get_path(astar_path* path, int idx);

#ifdef __cplusplus
}
#endif
