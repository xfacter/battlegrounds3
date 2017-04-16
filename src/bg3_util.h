/**
 * This file belongs to the 'Battlegrounds 3' game project.
 * Copyright 2009 xfacter
 * Copyright 2016 wickles
 * This work is licensed under the GPLv3
 * subject to all terms as reproduced in the included LICENSE file.
 */

#pragma once

#include "xlab/xconfig.h"
#include "xlab/xbuffer.h"
#include "xlab/xheightmap.h"
#include "xlab/xtexture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct decal_vertex {
	s8 u, v;
	float x, y, z;
} decal_vertex;

typedef struct decal_geom {
	decal_vertex vertices[6];
} decal_geom;

typedef struct bg3_decals {
	int max_decals;
	int num_decals;
	int index;
	float size;
	decal_geom* decals;
} bg3_decals;

typedef struct bg3_shadow {
	ScePspFMatrix4 proj;
	xBuffer* buf0;
} bg3_shadow;

#define BLEND_ADD_SIMPLE			0
#define BLEND_ADD_WITH_ALPHA		1
#define BLEND_AVERAGE_WITH_ALPHA	2
#define BLEND_MUL_COLOR				3
#define BLEND_MUL_INV_COLOR			4

float bg3_ray_heightmap_collision(xHeightmap* h, ScePspFVector3* out, ScePspFVector3* origin, ScePspFVector3* dir, float tmax, float tback);

void bg3_get_ellipsoid_inverse_matrix(ScePspFMatrix3* m, ScePspFMatrix4* a, xVector3f* radii);

float bg3_ray_ellipsoid_collision(ScePspFMatrix3* inverse_matrix, xVector3f* pos, xVector3f* origin, xVector3f* dir);

u32 bg3_set_blend(int mode, u32 fog);

void bg3_print_full(int x, int y, u32 txt_col, u32 shadow_col, char* fmt, ...);

void bg3_print_text(int x, int y, char* fmt, ...);

void bg3_draw_outline(int x, int y, int w, int h, u32 c);

void bg3_draw_rect(int x, int y, int w, int h, u32 c);

void bg3_draw_box(int x, int y, int w, int h, u32 box, u32 outline);

void bg3_draw_vert_grad(int x, int y, int w, int h, u32 c0, u32 c1);

void bg3_draw_tex(xTexture* tex, int x, int y);

void bg3_draw_tex2(xTexture* tex, int x, int y, int w, int h);

void bg3_draw_tex_center(xTexture* tex, int x, int y);

void bg3_draw_sprite(xVector3f* up_left, xVector3f* pos, float size);

void bg3_draw_quad_billboard(xVector3f* cam, xVector3f* pos, xVector3f* len, float h0, float h1, u32 c0, u32 c1);

bg3_decals* bg3_create_decals(int num, float size);

void bg3_free_decals(bg3_decals* d);

void bg3_add_decal(bg3_decals* d, xHeightmap* h, ScePspFVector3* p);

void bg3_draw_decals(bg3_decals* d);

bg3_shadow* bg3_create_shadow(int psm, int width, int height);

void bg3_free_shadow(bg3_shadow* s);

void bg3_shadow_projection(ScePspFMatrix4* m, ScePspFVector3* center, ScePspFVector3* lightdir, float fovy, float dist);

int bg3_shadow_setrendertarget(bg3_shadow* s, ScePspFVector3* center, ScePspFVector3* lightdir, float fovy, float dist, float intensity, int clear);

void bg3_shadow_endrendertarget();

int bg3_shadow_pass_start(ScePspFMatrix4* proj);

int bg3_shadowbuf_pass_start(bg3_shadow* s);

void bg3_shadow_pass_end();

void bg3_envmap_pass_start(float angle, float tx, float ty);

void bg3_envmap_pass_end();

int bg3_check_visibility(xHeightmap* h, ScePspFVector3* eye, ScePspFVector3* center);

#ifdef __cplusplus
}
#endif
