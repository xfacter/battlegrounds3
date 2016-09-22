#include <stdio.h>
#include "astar.h"

#include "xlib/xmem.h"
#define malloc(x) x_malloc(x)
#define free(x) x_free(x)

#define ASTAR_REGULAR_DIST 10
#define ASTAR_DIAGONAL_DIST 14

astar* astar_create(int width, int height, int heap_max)
{
	astar* as = (astar*)malloc(sizeof(astar));
	if (as == NULL) return NULL;
	as->width = width;
	as->height = height;
	as->nodes = (astar_node*)malloc(width*height*sizeof(astar_node));
	as->heap_max = heap_max;
	as->open_heap = (int*)malloc(heap_max*sizeof(int));
	if (as->nodes == NULL || as->open_heap == NULL)
	{
		astar_free(as);
		return NULL;
	}
	return as;
}

void astar_free(astar* as)
{
	if (as != NULL)
	{
		if (as->nodes != NULL)
		{
			free(as->nodes);
		}
		if (as->open_heap != NULL)
		{
			free(as->open_heap);
		}
		free(as);
	}
}

astar_path* astar_create_path(int max_length)
{
	astar_path* path = (astar_path*)malloc(sizeof(astar_path));
	if (path == NULL) return NULL;
	path->length = 0;
	path->max_length = max_length;
	path->nodes = (int*)malloc(max_length*sizeof(int));
	if (path->nodes == NULL)
	{
		astar_free_path(path);
		return NULL;
	}
	return path;
}

void astar_free_path(astar_path* path)
{
	if (path != NULL)
	{
		if (path->nodes != NULL)
		{
			free(path->nodes);
		}
		free(path);
	}
}


static void read_path(astar_path* path, astar* as, int goal)
{
	if (path == NULL || as == NULL) return;
	//printf("reading back path...\n");
	path->length = 0;
	int idx = goal;
	int i;
	for (i = 0; i < path->max_length; i++)
	{
		//printf("reading back path, length = %i\n", path->length);
		if (idx < 0 || idx >= as->width*as->height)
			return;
		path->nodes[i] = idx;
		idx = as->nodes[idx].came_from;
		path->length += 1;
	}
}

#define HORIZ_DIST 10
#define DIAG_DIST 14
#define ABS(X) ((X) < 0 ? -(X) : (X))
#define H_ESTIMATE(X0,Y0,X1,Y1) (ABS(((X1)-(X0))) + ABS(((Y1)-(Y0))))

//add checks for path length and etc
void astar_find_path(astar_path* path, astar* as, int start, int goal)
{
	//printf("start: %i, goal: %i\n", start, goal);
	if (as == NULL || path == NULL) return;
	path->length = 0;
	if (start < 0 || start >= as->width*as->height || goal < 0 || goal >= as->width*as->height || start == goal) return;
	if (as->nodes[goal].cost >= ASTAR_MAX_COST) return;
	int horiz_dist = ASTAR_MAX_COST;
	int diag_dist = (int)(1.414f*ASTAR_MAX_COST);
	int i, j;
	for (i = 0; i < as->width*as->height; i++)
	{
		as->nodes[i].open = 0;
		//as->nodes[i].closed = 0;
		as->nodes[i].used = 0;
	}
	int x0 = start % as->width;
	int y0 = start / as->width;
	int x1 = goal % as->width;
	int y1 = goal / as->width;
	//add start to open list
	as->nodes[start].open = 1;
	as->nodes[start].came_from = -1;
	as->nodes[start].g_score = 0;
	as->nodes[start].f_score = horiz_dist*H_ESTIMATE(x0,y0,x1,y1);
	as->open_heap[0] = start;
	int num_open = 1;
	//need to check if num_open is greater than heap_max...
	while (num_open > 0)
	{
		//find node with lowest f value
		int idx = as->open_heap[0];
		if (idx == goal)
		{
			read_path(path, as, goal);
			return;
		}
		as->nodes[idx].open = 0;
		//as->nodes[idx].closed = 1;
		as->nodes[idx].used = 1;
		num_open -= 1;
		//move top item off the open list
		as->open_heap[0] = as->open_heap[num_open];
		int u;
		int v = 1;
		while (1)
		{
			//printf("moving top item off list\n");
			u = v;		
			if (2*u+1 <= num_open) //if both children exist
			{
				//Check if the F cost of the parent is greater than each child.
				//Select the lowest of the two children.
				if (as->nodes[as->open_heap[u-1]].f_score >= as->nodes[as->open_heap[2*u-1]].f_score)
					v = 2*u;
				if (as->nodes[as->open_heap[v-1]].f_score >= as->nodes[as->open_heap[2*u+1-1]].f_score)
					v = 2*u+1;		
			}
			else if (2*u <= num_open) //if only child #1 exists
			{
				//Check if the F cost of the parent is greater than child #1
				if (as->nodes[as->open_heap[u-1]].f_score >= as->nodes[as->open_heap[2*u-1]].f_score)
					v = 2*u;
			}

			if (u != v) //if parent's F is > one of its children, swap them
			{
				int temp = as->open_heap[u-1];
				as->open_heap[u-1] = as->open_heap[v-1];
				as->open_heap[v-1] = temp;
			}
			else
				break; //otherwise, exit loop
		}

		x0 = idx % as->width;
		y0 = idx / as->width;
		for (i = x0-1; i <= x0+1; i++)
		{
			for (j = y0-1; j <= y0+1; j++)
			{
				//printf("checking node\n");
				if (i < 0 || i >= as->width || j < 0 || j >= as->height || (i == x0 && j == y0))
					continue;
				int k = j*as->width + i;
				if (as->nodes[k].used || as->nodes[k].closed || as->nodes[k].cost >= ASTAR_MAX_COST)
					continue;
				int temp_g = as->nodes[idx].g_score + (as->nodes[idx].cost+1)*((i != x0 && j != y0) ? diag_dist : horiz_dist);
				int m = -1;
				if (!as->nodes[k].open)
				{
					num_open += 1;
					if (num_open >= as->heap_max)
						return;
					as->nodes[k].open = 1;
					as->nodes[k].came_from = idx;
					as->nodes[k].g_score = temp_g;
					as->nodes[k].f_score = temp_g + horiz_dist*H_ESTIMATE(i,j,x1,y1);
					m = num_open;
					as->open_heap[m-1] = k;
				}
				else if (temp_g < as->nodes[k].g_score)
				{
					as->nodes[k].came_from = idx;
					as->nodes[k].g_score = temp_g;
					as->nodes[k].f_score = temp_g + horiz_dist*H_ESTIMATE(i,j,x1,y1);
					for (m = 1; m <= num_open; m++)
					{
						if (as->open_heap[m-1] == k)
							break;
					}
				}
				while (m > 1) //While item hasn't bubbled to the top (m=1)	
				{
					//printf("pushing node to top\n");
					//Check if child's F cost is < parent's F cost. If so, swap them.	
					if (as->nodes[as->open_heap[m-1]].f_score <= as->nodes[as->open_heap[m/2-1]].f_score)
					{
						int temp = as->open_heap[m/2-1];
						as->open_heap[m/2-1] = as->open_heap[m-1];
						as->open_heap[m-1] = temp;
						m = m/2;
					}
					else
						break;
				}
			}
		}
	}
}

int astar_get_path(astar_path* path, int idx)
{
	if (path == NULL) return -1;
	if (idx < 0 || idx >= path->length) return -1;
	int node = path->length-1-idx;
	if (path->length > path->max_length)
	{
		node -= path->length - path->max_length;
	}
	return path->nodes[node];
}