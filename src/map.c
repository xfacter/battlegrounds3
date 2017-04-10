/**
 * This file belongs to the 'Battlegrounds 3' game project.
 * Copyright 2009 xfacter
 * Copyright 2016 wickles
 * This work is licensed under the GPLv3
 * subject to all terms as reproduced in the included LICENSE file.
 */

#include <stdio.h>
#include <string.h>
#include <pspgu.h>
#include "xlib/xmem.h"
#include "xlib/xini.h"
#include "xlib/xmath.h"
#include "values.h"

#include "map.h"

#define BG3_HMP_MINLEVEL 3
#define PATH_MIN_Z 0.6f
#define ASTAR_MAP_FACTOR 4

bg3_map* bg3_load_map(int id)
{
	X_LOG("Attempting to load map %i...", id);
	if (id < 1) return NULL;
	bg3_map* m = (bg3_map*)x_malloc(sizeof(bg3_map));
	if (m == NULL) return NULL;
	m->spawns = NULL;
	m->astar_map = NULL;
	m->terrain_tex = NULL;
	m->sky_tex = NULL;
	m->hmp.vertices = NULL;
	m->hmp_lod.h = NULL;

	int i;
	char buffer[256];

	snprintf(buffer, 256, "./maps/%02i/map.ini", id);
	FILE* file = fopen(buffer, "r");
	if (file == NULL)
	{
		fclose(file);
		bg3_free_map(m);
		return NULL;
	}
	m->ambient = xIniGetFloat(file, "ambient", 0.0f);
	m->intensity= xIniGetFloat(file, "intensity", 0.0f);
	float direction = DEG_TO_RAD(90.0f - xIniGetFloat(file, "direction", 0.0f)) + X_PI;
	float pitch = DEG_TO_RAD(xIniGetFloat(file, "pitch", 0.0f));
	m->light_pos.x = x_cosf(direction)*x_cosf(pitch);
	m->light_pos.y = x_sinf(direction)*x_cosf(pitch);
	m->light_pos.z = x_sinf(pitch);
	m->players = xIniGetFloat(file, "players", 8);
	xIniGetString(file, "type", buffer, 0);
	if (strcmp(buffer, "DESERT") == 0)
	{
		m->type = ENV_DESERT;
		snprintf(buffer, 256, "./skies/desert.tga");
	}
	else if (strcmp(buffer, "ARCTIC") == 0)
	{
		m->type = ENV_ARCTIC;
		snprintf(buffer, 256, "./skies/arctic.tga");
	}
	else if (strcmp(buffer, "TROPICAL") == 0)
	{
		m->type = ENV_TROPICAL;
		snprintf(buffer, 256, "./skies/tropical.tga");
	}
	else
	{
		m->type = ENV_TROPICAL;
		snprintf(buffer, 256, "./skies/tropical.tga");
	}

	m->sky_tex = xTexLoadTGA(buffer, 0, X_TEX_TOP_IN_VRAM);
	/*
	if (m->sky_tex == NULL)
	{
		fclose(file);
		bg3_free_map(m);
		return NULL;
	}
	*/

	xIniGetString(file, "fog", buffer, 0);
	if (strcmp(buffer, "FOG_NONE") == 0)
	{
		m->fog = FOG_NONE;
	}
	else if (strcmp(buffer, "FOG_MEDIUM") == 0)
	{
		m->fog = FOG_MEDIUM;
	}
	else if (strcmp(buffer, "FOG_THICK") == 0)
	{
		m->fog = FOG_THICK;
	}
	else
	{
		m->fog = FOG_NONE;
	}

	xIniGetString(file, "wind", buffer, 0);
	if (strcmp(buffer, "WIND_NONE") == 0)
	{
		m->wind = WIND_NONE;
	}
	else if (strcmp(buffer, "WIND_MEDIUM") == 0)
	{
		m->wind = WIND_MEDIUM;
	}
	else if (strcmp(buffer, "WIND_FULL") == 0)
	{
		m->wind = WIND_FULL;
	}
	else
	{
		m->wind = WIND_NONE;
	}

	m->num_spawns = xIniGetInt(file, "num_spawns", 0);
	m->spawns = (ScePspIVector2*)x_malloc(m->num_spawns*sizeof(ScePspIVector2));
	if (m->spawns == NULL)
	{
		fclose(file);
		bg3_free_map(m);
		return NULL;
	}
	for (i = 0; i < m->num_spawns; i++)
	{
		snprintf(buffer, 256, "spawn[%i]", i+1);
		m->spawns[i].x = 0;
		m->spawns[i].y = 0;
		xIniGetVectori(file, buffer, &m->spawns[i].x, &m->spawns[i].y, NULL);
	}

	m->num_poi = xIniGetInt(file, "num_poi", 0);
	m->poi = (ScePspIVector2*)x_malloc(m->num_poi*sizeof(ScePspIVector2));
	if (m->poi == NULL)
	{
		/*
		fclose(file);
		bg3_free_map(m);
		return NULL;
		*/
		m->num_poi = 0;
	}
	for (i = 0; i < m->num_poi; i++)
	{
		snprintf(buffer, 256, "poi[%i]", i+1);
		m->poi[i].x = 0;
		m->poi[i].y = 0;
		xIniGetVectori(file, buffer, &m->poi[i].x, &m->poi[i].y, NULL);
	}

	m->num_powerups = xIniGetInt(file, "num_powerups", 0);
	m->powerups = (map_powerup*)x_malloc(m->num_powerups*sizeof(map_powerup));
	if (m->powerups == NULL)
	{
		/*
		fclose(file);
		bg3_free_map(m);;
		return NULL;
		*/
		m->num_powerups = 0;
	}
	for (i = 0; i < m->num_powerups; i++)
	{
		snprintf(buffer, 256, "powerup[%i].type", i+1);
		xIniGetString(file, buffer, buffer, NULL);
		if (strcmp(buffer, "ARMOR") == 0)
		{
			m->powerups[i].type = BG3_POWERUP_ARMOR;
		}
		else if (strcmp(buffer, "LASER") == 0)
		{
			m->powerups[i].type = BG3_POWERUP_LASER;
		}
		else if (strcmp(buffer, "TSHELL") == 0)
		{
			m->powerups[i].type = BG3_POWERUP_TSHELL;
		}
		else if (strcmp(buffer, "MISSILE") == 0)
		{
			m->powerups[i].type = BG3_POWERUP_MISSILE;
		}
		else
		{
			m->powerups[i].type = BG3_POWERUP_ARMOR;
		}
		snprintf(buffer, 256, "powerup[%i].pos", i+1);
		m->powerups[i].pos.x = 0;
		m->powerups[i].pos.y = 0;
		xIniGetVectori(file, buffer, &m->powerups[i].pos.x, &m->powerups[i].pos.y, NULL);
	}

	float tile_scale = xIniGetFloat(file, "tile_scale", 0.0f);
	float height_scale = xIniGetFloat(file, "height_scale", 0.0f);
	int width = xIniGetInt(file, "width", 0);
	fclose(file);

	snprintf(buffer, 256, "./maps/%02i/terrain.raw", id);
	if (xHeightmapLoad(&m->hmp, buffer, width, tile_scale, height_scale) != 0)
	{
		bg3_free_map(m);
		return NULL;
	}
	xHeightmapSetupLOD(&m->hmp_lod, &m->hmp, BG3_HMP_MINLEVEL);
	if (m->hmp_lod.h == NULL)
	{
		bg3_free_map(m);
		return NULL;
	}

	m->width = m->hmp.width-1;
#if 1
	m->astar_map = astar_create(m->width, m->width, SQR(m->width));
	if (m->astar_map == NULL)
	{
		bg3_free_map(m);
		return NULL;
	}
	int x, y;
	for (y = 0; y < m->astar_map->height; y++)
	{
		for (x = 0; x < m->astar_map->width; x++)
		{
			i = y*m->astar_map->width + x;
			//printf("x: %i, y: %i, i: %i\n", x, y, i);
			ScePspFVector3 n0, n1;
			normal_from_grid(&m->hmp, &n0, x, y, 0);
			normal_from_grid(&m->hmp, &n1, x, y, 1);
			float z = 0.5f*(n0.z + n1.z);
			//m->astar_map->nodes[i].cost = (z <= 0.8f ? 10 : 0);
			m->astar_map->nodes[i].cost = (z <= PATH_MIN_Z ? 10 : (int)(10*((1.0f-z)/(1.0f-PATH_MIN_Z))) );
			//printf("z: %f\n", z);
		}
	}
#else
	m->astar_map = astar_create(width/ASTAR_MAP_FACTOR, width/ASTAR_MAP_FACTOR, SQR(width/ASTAR_MAP_FACTOR));
	if (m->astar_map == NULL)
	{
		bg3_free_map(m);
		return NULL;
	}
	int x0, y0;
	for (y0 = 0; y0 < m->astar_map->height; y0++)
	{
		for (x0 = 0; x0 < m->astar_map->width; x0++)
		{
			float z = 0.0f;
			int x1, y1;
			for (y1 = 0; y1 < ASTAR_MAP_FACTOR; y1++)
			{
				for (x1 = 0; x1 < ASTAR_MAP_FACTOR; x1++)
				{
					//printf("%i, %i\n", x0*mul+x1, y0*mul+y1);
					ScePspFVector3 n;
					normal_from_grid(&m->hmp, &n, x0*ASTAR_MAP_FACTOR+x1, y0*ASTAR_MAP_FACTOR+y1, 0);
					z += n.z;
					normal_from_grid(&m->hmp, &n, x0*ASTAR_MAP_FACTOR+x1, y0*ASTAR_MAP_FACTOR+y1, 1);
					z += n.z;
				}
			}
			z /= SQR(ASTAR_MAP_FACTOR)*2;
			//printf("Z: %f\n", z);
			m->astar_map->nodes[y0*m->astar_map->width + x0].cost = (z <= PATH_MIN_Z ? 10 : (int)(10*((1.0f-z)/(1.0f-PATH_MIN_Z))) );
		}
	}
#endif

	snprintf(buffer, 256, "./maps/%02i/terrain.tga", id);
	m->terrain_tex = xTexLoadTGA(buffer, 0, X_TEX_TOP_IN_VRAM);
	/*
	if (m->terrain_tex == NULL)
	{
		bg3_free_map(m);
		return NULL;
	}
	*/

	X_LOG("map: type %i, ambient %f, intensity %f, light_pos (%f, %f, %f), num_spawns %i, spawn (%i, %i), num_poi %i, num_powerups %i", m->type, m->ambient, m->intensity, m->light_pos.x, m->light_pos.y, m->light_pos.z, m->num_spawns, m->spawns[0].x, m->spawns[0].y, m->num_poi, m->num_powerups);
	X_LOG("Successfully loaded map.");
	return m;
}

void bg3_free_map(bg3_map* m)
{
	X_LOG("Freeing map.");
	if (m != NULL)
	{
		if (m->spawns != NULL)
		{
			x_free(m->spawns);
		}
		if (m->poi != NULL)
		{
			x_free(m->poi);
		}
		if (m->powerups != NULL)
		{
			x_free(m->powerups);
		}
		if (m->astar_map != NULL)
		{
			astar_free(m->astar_map);
		}
		if (m->terrain_tex != NULL)
		{
			xTexFree(m->terrain_tex);
		}
		if (m->sky_tex != NULL)
		{
			xTexFree(m->sky_tex);
		}
		if (m->hmp.vertices != NULL)
		{
			xHeightmapFree(&m->hmp);
		}
		if (m->hmp_lod.h != NULL)
		{
			xHeightmapFreeLOD(&m->hmp_lod);
		}
		x_free(m);
	}
}

void bg3_map_random_spawn(bg3_map* m, ScePspFVector2* out)
{
	if (m == NULL || out == NULL || m->num_spawns <= 0) return;
	int rand = x_randi(0, m->num_spawns-1);
	out->x = m->spawns[rand].x*m->hmp.tile_scale + 0.5f*m->hmp.tile_scale;
	out->y = m->spawns[rand].y*m->hmp.tile_scale + 0.5f*m->hmp.tile_scale;
}

void bg3_map_random_poi(bg3_map* m, ScePspIVector2* out)
{
	if (m == NULL || out == NULL) return;
	if (m->num_poi <= 0)
	{
		out->x = -1;
		out->y = -1;
	}
	else
	{
		int rand = x_randi(0, m->num_poi-1);
		*out = m->poi[rand];
	}

}

void bg3_map_setup_env(bg3_map* m)
{
	if (m == NULL) return;
	sceGuLight(0, GU_DIRECTIONAL, GU_DIFFUSE_AND_SPECULAR, &m->light_pos);
	sceGuLightColor(0, GU_DIFFUSE, 0xff7f7f7f);
	sceGuLightColor(0, GU_SPECULAR, 0xffffffff);
	sceGuSpecular(12.0f);
	sceGuAmbient(GU_COLOR(m->ambient, m->ambient, m->ambient, 1.0f));
	sceGuLightAtt(0, m->intensity, 0.0f, 0.0f);
	sceGuEnable(GU_LIGHT0);
}

int bg3_map_to_astar(bg3_map* m, int id)
{
	if (m == NULL) return -1;
	if (m->astar_map == NULL) return -1;
	int x = id % (m->hmp.width-1);
	int y = id / (m->hmp.width-1);
	x /= ASTAR_MAP_FACTOR;
	y /= ASTAR_MAP_FACTOR;
	return y*m->astar_map->width + x;
}

int bg3_astar_to_map(bg3_map* m, int id)
{
	if (m == NULL) return -1;
	if (m->astar_map == NULL) return -1;
	int x = id % m->astar_map->width;
	int y = id / m->astar_map->width;
	x = x*ASTAR_MAP_FACTOR + ASTAR_MAP_FACTOR/2;
	y = y*ASTAR_MAP_FACTOR + ASTAR_MAP_FACTOR/2;
	return y*(m->hmp.width-1) + x;
}

bg3_map_preview* bg3_load_map_preview(int id)
{
	X_LOG("Attempting to load map preview...");
	bg3_map_preview* p = (bg3_map_preview*)x_malloc(sizeof(bg3_map_preview));
	if (p == NULL) return NULL;
	char buffer[256];

	snprintf(buffer, 256, "./maps/%02i/map.ini", id);
	FILE* file = fopen(buffer, "r");
	xIniGetString(file, "name", p->name, "Name Not Found");
	p->players = xIniGetFloat(file, "players", 8);
	p->width = xIniGetInt(file, "width", 0) - 1;
	fclose(file);

	snprintf(buffer, 256, "./maps/%02i/terrain.tga", id);
	p->terrain_tex = xTexLoadTGA(buffer, 0, 0);
	if (p->terrain_tex == NULL)
	{
		bg3_free_map_preview(p);
		return NULL;
	}

	X_LOG("Successfully loaded map preview.");
	return p;
}

void bg3_free_map_preview(bg3_map_preview* p)
{
	X_LOG("Freeing map preview.");
	if (p != NULL)
	{
		if (p->terrain_tex != NULL)
		{
			xTexFree(p->terrain_tex);
		}
		x_free(p);
	}
}
