#ifndef __MAP_H__
#define __MAP_H__

#include "xlib/xconfig.h"
#include "xlib/xheightmap.h"
#include "xlib/xtexture.h"
#include "astar.h"

#define ENV_DESERT		0
#define ENV_ARCTIC		1
#define ENV_TROPICAL	2

#define FOG_NONE	0
#define FOG_MEDIUM	1
#define FOG_THICK	2

#define WIND_NONE	0
#define WIND_MEDIUM	1
#define WIND_FULL	2

typedef struct map_powerup {
	int type;
	ScePspIVector2 pos;
} map_powerup;

typedef struct bg3_map {
	int width;
	int type;
	int fog;
	int wind;
	int players;
	float ambient;
	float intensity;
	ScePspFVector3 light_pos;
	int num_spawns;
	ScePspIVector2* spawns;
	int num_poi;
	ScePspIVector2* poi;
	int num_powerups;
	map_powerup* powerups;
	xHeightmap hmp;
	xHeightmapLOD hmp_lod;
	astar* astar_map;
	xTexture* terrain_tex;
	xTexture* sky_tex;
} bg3_map;

typedef struct bg3_map_preview {
	int players;
	int width;
	xTexture* terrain_tex;
} bg3_map_preview;

bg3_map* bg3_load_map(int id);

void bg3_free_map(bg3_map* m);

void bg3_map_random_spawn(bg3_map* m, ScePspFVector2* out);

void bg3_map_random_poi(bg3_map* m, ScePspIVector2* out);

void bg3_map_setup_env(bg3_map* m);

int bg3_map_to_astar(bg3_map* m, int id);

int bg3_astar_to_map(bg3_map* m, int id);

bg3_map_preview* bg3_load_map_preview(int id);

void bg3_free_map_preview(bg3_map_preview* p);

#endif