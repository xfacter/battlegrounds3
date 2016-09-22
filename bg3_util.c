#include <math.h>
#include <pspgu.h>
#include <pspgum.h>
#include "xlib/xmem.h"
#include "xlib/xmath.h"
#include "xlib/xgraphics.h"

#include "bg3_util.h"

float bg3_ray_plane_collision(ScePspFVector3* out, ScePspFVector3* origin, ScePspFVector3* dir, ScePspFVector3* normal, ScePspFVector3* point)
{
    if (!origin || !dir || !normal || !point)
        return 0.0f;
    float d = -x_dotproduct(normal, point);
    float vd = x_dotproduct(normal, dir);
    //if ray points opposite the normal or is parallel, dont care, return an obvious error val
	//printf("vd = %f\n", vd);
    if (vd >= 0.0f)
        return HUGE_VAL;
    float v0 = -(x_dotproduct(normal, origin) + d);
    float t = v0 / vd;
    if (out)
    {
        out->x = origin->x + t*dir->x;
        out->y = origin->y + t*dir->y;
        out->z = origin->z + t*dir->z;
    }
	//printf("t = %f\n", t);
    return t;
}

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

float bg3_ray_heightmap_collision(xHeightmap* h, ScePspFVector3* out, ScePspFVector3* origin, ScePspFVector3* dir, float tmax, float tback)
{
    ScePspFVector3 n, p;
    float inv_scale, gridx, gridy, t0, gridx2, gridy2,
          inv_x, inv_y, tDeltaX, tDeltaY, tCur, tNext,
		  zCur, zNext, z0, z1, z2, z3;
    int x, y;
    if (!origin || !dir)
		return 0.0f;
    if (!h)
        goto end;
    if (!h->vertices || origin->x < 0.0f || origin->x >= h->tile_scale*h->width || origin->y < 0.0f || origin->y >= h->tile_scale*h->height)
        goto end;
	if (dir->x == 0.0f && dir->y == 0.0f)
	{
		if (dir->z < 0.0f)
		{
			float height = xHeightmapGetHeight(h, 0, origin->x, origin->y);
			if (origin->z - height < -dir->z*(tmax+tback))
			{
				p.x = origin->x;
				p.y = origin->y;
				p.z = height;
				t0 = (origin->z - height) / (-dir->z*(tmax+tback));
				goto collision;
			}
			else
			{
				goto end;
			}
		}
		else
		{
			goto end;
		}
	}
    inv_scale = 1.0f/h->tile_scale;
    inv_x = (dir->x == 0.0f ? HUGE_VAL : 1.0f/dir->x);
    inv_y = (dir->y == 0.0f ? HUGE_VAL : 1.0f/dir->y);
    gridx = origin->x*inv_scale;
	gridy = origin->y*inv_scale;
	//x = (dir->x < 0.0f && gridx == (int)gridx ? (int)gridx-1 : (int)gridx);
	//y = (dir->y < 0.0f && gridy == (int)gridy ? (int)gridy-1 : (int)gridy);
	x = (int)gridx;
	y = (int)gridy;
	//printf("x = %i, gridx = %f\n", x, gridx);
	//printf("y = %i, gridy = %f\n", y, gridy);
	tCur = 0.0f;
    while (tCur < tmax + tback)
    {
        if (x < 0 || x >= h->width-1 || y < 0 || y >= h->height-1)
            goto end;

		tDeltaX = (dir->x == 0.0f ? HUGE_VAL :
			(dir->x > 0.0f ? h->tile_scale*(x+1 - gridx)*inv_x :
			(gridx == x ? -h->tile_scale*inv_x : -h->tile_scale*(gridx - x)*inv_x)));
		tDeltaY = (dir->y == 0.0f ? HUGE_VAL :
			(dir->y > 0.0f ? h->tile_scale*(y+1 - gridy)*inv_y :
			(gridy == y ? -h->tile_scale*inv_y : -h->tile_scale*(gridy - y)*inv_y)));
		tNext = (tDeltaX < tDeltaY ? tCur + tDeltaX : tCur + tDeltaY);
		zCur = origin->z + tCur*dir->z;
		zNext = origin->z + tNext*dir->z;
		z0 = point_from_grid(h, x, y)->z;
		z1 = point_from_grid(h, x+1, y)->z;
		z2 = point_from_grid(h, x, y+1)->z;
		z3 = point_from_grid(h, x+1, y+1)->z;

		/*
		if (!(zCur > z0 && zCur > z1 && zCur > z2 && zCur > z3) && !(zNext > z0 && zNext > z1 && zNext > z2 && zNext > z3))
		{
		*/
		if (MAX(z0, MAX(z1, MAX(z2, z3))) >= zNext)
		{
			//check the bottom right triangle
			normal_from_grid(h, &n, x, y, 0);
			if ((t0 = bg3_ray_plane_collision(&p, origin, dir, &n, point_from_grid(h, x, y))) != HUGE_VAL)
			{
			//printf("t0 = %f\n", t0);
				gridx2 = p.x*inv_scale;
				gridy2 = p.y*inv_scale;
				if ((gridx2 >= x && gridx2 <= x+1) && (gridy2 >= y && gridy2 <= y+1) && (gridy2 - (int)gridy2 <= gridx2 - (int)gridx2) && (t0 <= tmax + tback))
					goto collision;
			}
			//check the top left triangle
			normal_from_grid(h, &n, x, y, 1);
			if ((t0 = bg3_ray_plane_collision(&p, origin, dir, &n, point_from_grid(h, x, y))) != HUGE_VAL)
			{
			//printf("t0 = %f\n", t0);
				gridx2 = p.x*inv_scale;
				gridy2 = p.y*inv_scale;
				if ((gridx2 >= x && gridx2 <= x+1) && (gridy2 >= y && gridy2 <= y+1) && (gridy2 - (int)gridy2 >= gridx2 - (int)gridx2) && (t0 <= tmax + tback))
					goto collision;
			}
		}

		tCur = tNext;
		gridx = (origin->x + tCur*dir->x)*inv_scale;
		gridy = (origin->y + tCur*dir->y)*inv_scale;
        if (tDeltaX < tDeltaY)
		{
            x += (dir->x > 0.0f ? 1 : -1);
		}
        else
		{
            y += (dir->y > 0.0f ? 1 : -1);
		}
    }
end:
	if (out)
	{
		out->x = origin->x + tmax*dir->x;
		out->y = origin->y + tmax*dir->y;
		out->z = origin->z + tmax*dir->z;
	}
    return tmax;
collision:
	if (out)
	{
		if (tback > 0.0f)
		{
			out->x = p.x - tback*dir->x;
			out->y = p.y - tback*dir->y;
			out->z = p.z - tback*dir->z;
		}
		else
		{
			*out = p;
		}
	}
    return t0 - tback;
}

void bg3_get_ellipsoid_inverse_matrix(ScePspFMatrix3* m, ScePspFMatrix4* a, xVector3f* radii)
{
	xVec3Scale((xVector3f*)&m->x, (xVector3f*)&a->x, 1.0f/radii->x);
	xVec3Scale((xVector3f*)&m->y, (xVector3f*)&a->y, 1.0f/radii->y);
	xVec3Scale((xVector3f*)&m->z, (xVector3f*)&a->z, 1.0f/radii->z);
}

float bg3_ray_ellipsoid_collision(ScePspFMatrix3* inverse_matrix, xVector3f* pos, xVector3f* origin, xVector3f* dir)
{
	xVector3f rel_orig;
	xVec3Sub(&rel_orig, origin, pos);
	xVector3f e_orig, e_dir;
	xVec3Set(&e_orig,
		xVec3Dot(&rel_orig, (xVector3f*)&inverse_matrix->x),
		xVec3Dot(&rel_orig, (xVector3f*)&inverse_matrix->y),
		xVec3Dot(&rel_orig, (xVector3f*)&inverse_matrix->z));
	xVec3Set(&e_dir,
		xVec3Dot(dir, (xVector3f*)&inverse_matrix->x),
		xVec3Dot(dir, (xVector3f*)&inverse_matrix->y),
		xVec3Dot(dir, (xVector3f*)&inverse_matrix->z));
	float a, b, c;
	a = xVec3Dot(&e_dir, &e_dir);
	b = 2.0f*xVec3Dot(&e_orig, &e_dir);
	c = xVec3Dot(&e_orig, &e_orig) - 1.0f;
	float discrim = b*b - 4.0f*a*c;
	if (discrim < 0.0f) return -HUGE_VAL;
	float t = (-b - x_sqrtf(discrim)) / (2.0f*a);
	return t;
}

/*
int bg3_ellipsoid_ellipsoid_collision(ScePspFMatrix3* inv_mat0, xVector3f* pos0, ScePspFMatrix3* inv_mat1, xVector3f* pos1)
{
	xVector3f rel_pos;
	xVec3Sub(&rel_pos, pos1, pos0);
	xVector3f e_dir1, e_dir2;
	xVec3Set(&e_dir1,
		xVec3Dot(&rel_pos, (xVector3f*)&inv_mat0->x),
		xVec3Dot(&rel_pos, (xVector3f*)&inv_mat0->y),
		xVec3Dot(&rel_pos, (xVector3f*)&inv_mat0->z));
	//xVec3Set(&e_dir1, -e_dir1.x, -e_dir1.y, -e_dir1.z);
	xVec3Set(&rel_pos, -rel_pos.x, -rel_pos.y, -rel_pos.z);
	xVec3Set(&e_dir2,
		xVec3Dot(&rel_pos, (xVector3f*)&inv_mat1->x),
		xVec3Dot(&rel_pos, (xVector3f*)&inv_mat1->y),
		xVec3Dot(&rel_pos, (xVector3f*)&inv_mat1->z));
	//return (xVec3SqLength(&e_dir2) <= SQR(2.0f));
	return (xVec3Dot(&e_dir1, &e_dir2) <= 2.0f);
}
*/

u32 bg3_set_blend(int mode, u32 fog)
{
	switch (mode)
	{
	case BLEND_ADD_SIMPLE:
		sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0xffffffff, 0xffffffff);
		return 0x000000;
	case BLEND_ADD_WITH_ALPHA:
		sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_FIX, 0, 0xffffffff);
		return 0x000000;
	case BLEND_AVERAGE_WITH_ALPHA:
		sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
		return fog;
	case BLEND_MUL_COLOR:
		sceGuBlendFunc(GU_ADD, GU_FIX, GU_SRC_COLOR, 0, 0);
		return 0xffffff;
	case BLEND_MUL_INV_COLOR:
		sceGuBlendFunc(GU_ADD, GU_FIX, GU_ONE_MINUS_SRC_COLOR, 0, 0);
		return 0x000000;
	}
	return fog;
}

void bg3_draw_outline(int x, int y, int w, int h, u32 c)
{
	CVertex2D* vertices = (CVertex2D*)sceGuGetMemory(5*sizeof(CVertex2D));
	vertices[0].color = c;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0;
	vertices[1].color = c;
	vertices[1].x = x + w;
	vertices[1].y = y;
	vertices[1].z = 0;
	vertices[2].color = c;
	vertices[2].x = x + w;
	vertices[2].y = y + h;
	vertices[2].z = 0;
	vertices[3].color = c;
	vertices[3].x = x;
	vertices[3].y = y + h;
	vertices[3].z = 0;
	vertices[4] = vertices[0];
	xGuSaveStates();
	sceGuDisable(GU_DEPTH_TEST);
	sceGuDisable(GU_TEXTURE_2D);
	sceGuDepthMask(GU_TRUE);
	sceGuDrawArray(GU_LINE_STRIP, CVertex2D_vtype|GU_TRANSFORM_2D, 5, 0, vertices);
	sceGuDepthMask(GU_FALSE);
	xGuLoadStates();
}

void bg3_draw_rect(int x, int y, int w, int h, u32 c)
{
	CVertex2D* vertices = (CVertex2D*)sceGuGetMemory(4*sizeof(CVertex2D));
	vertices[0].color = c;
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0;
	vertices[1].color = c;
	vertices[1].x = x + w;
	vertices[1].y = y;
	vertices[1].z = 0;
	vertices[2].color = c;
	vertices[2].x = x + w;
	vertices[2].y = y + h;
	vertices[2].z = 0;
	vertices[3].color = c;
	vertices[3].x = x;
	vertices[3].y = y + h;
	vertices[3].z = 0;
	xGuSaveStates();
	sceGuDisable(GU_DEPTH_TEST);
	sceGuDisable(GU_TEXTURE_2D);
	sceGuDepthMask(GU_TRUE);
	sceGuDrawArray(GU_TRIANGLE_FAN, CVertex2D_vtype|GU_TRANSFORM_2D, 4, 0, vertices);
	sceGuDepthMask(GU_FALSE);
	xGuLoadStates();
}

void bg3_draw_tex(xTexture* tex, int x, int y)
{
	if (tex == NULL) return;
	xTexDraw(tex, x, y, tex->width, tex->height, 0, 0, tex->width, tex->height);
}

void bg3_draw_tex_center(xTexture* tex, int x, int y)
{
	if (tex == NULL) return;
	xTexDraw(tex, x - tex->width/2, y - tex->height/2, tex->width, tex->height, 0, 0, tex->width, tex->height);
}

typedef struct {
	s8 u, v;
	//u32 c;
	float x, y, z;
} sprite_vertex;

#define sprite_vertex_vtype GU_TEXTURE_8BIT/*|GU_COLOR_8888*/|GU_VERTEX_32BITF

void bg3_draw_sprite(xVector3f* up_right, xVector3f* pos, float size)
{
	sprite_vertex* vertices = (sprite_vertex*)sceGuGetMemory(2*sizeof(sprite_vertex));
	vertices[0].u = 0;
	vertices[0].v = 0;
	//vertices[i*2+0].c = color32;
	vertices[0].x = pos->x - size*up_right->x;
	vertices[0].y = pos->y - size*up_right->y;
	vertices[0].z = pos->z - size*up_right->z;
	vertices[1].u = 127;
	vertices[1].v = 127;
	//vertices[i*2+1].c = color32;
	vertices[1].x = pos->x + size*up_right->x;
	vertices[1].y = pos->y + size*up_right->y;
	vertices[1].z = pos->z + size*up_right->z;
	sceGumDrawArray(GU_SPRITES, sprite_vertex_vtype|GU_TRANSFORM_3D, 2, 0, vertices);
}


void bg3_draw_quad_billboard(xVector3f* cam, xVector3f* pos, xVector3f* len, float h0, float h1, u32 c0, u32 c1)
{
	xVector3f p2, dir, up;

	xVec3Add(&p2, pos, len);
	xVec3Sub(&dir, cam, pos);
	xVec3Cross(&up, &dir, len);
	xVec3Normalize(&up, &up);
	xVec3Scale(&up, &up, 0.5f);

	TCVertexF* vertices = (TCVertexF*)sceGuGetMemory(4*sizeof(TCVertexF));
	vertices[0].color = c0;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;
    vertices[0].x = pos->x - h0*up.x;
    vertices[0].y = pos->y - h0*up.y;
	vertices[0].z = pos->z - h0*up.z;
	vertices[1].color = c0;
	vertices[1].u = 0.0f;
	vertices[1].v = 1.0f;
    vertices[1].x = pos->x + h0*up.x;
    vertices[1].y = pos->y + h0*up.y;
	vertices[1].z = pos->z + h0*up.z;
	vertices[2].color = c1;
	vertices[2].u = 1.0f;
	vertices[2].v = 1.0f;
    vertices[2].x = p2.x + h1*up.x;
    vertices[2].y = p2.y + h1*up.y;
	vertices[2].z = p2.z + h1*up.z;
	vertices[3].color = c1;
	vertices[3].u = 1.0f;
	vertices[3].v = 0.0f;
    vertices[3].x = p2.x - h1*up.x;
    vertices[3].y = p2.y - h1*up.y;
    vertices[3].z = p2.z - h1*up.z;
	sceGumDrawArray(GU_TRIANGLE_FAN, TCVertexF_vtype|GU_TRANSFORM_3D, 4, 0, vertices);
}

bg3_decals* bg3_create_decals(int num, float size)
{
	bg3_decals* d = x_malloc(sizeof(bg3_decals));
	if (d == NULL) return NULL;
	d->max_decals = num;
	d->num_decals = 0;
	d->index = 0;
	d->size = size;
	d->decals = x_malloc(num*sizeof(decal_geom));
	if (d->decals == NULL)
	{
		bg3_free_decals(d);
		return NULL;
	}
	return d;
}

void bg3_free_decals(bg3_decals* d)
{
	if (d != NULL)
	{
		if (d->decals != NULL)
		{
			x_free(d->decals);
		}
		x_free(d);
	}
}

void bg3_add_decal(bg3_decals* d, xHeightmap* h, ScePspFVector3* p)
{
	if (d == NULL || h == NULL || p == NULL) return;
	xVector3f right, fwd, up;
	xHeightmapGetNormal(h, (ScePspFVector3*)&up, p->x, p->y);
	if (up.z == 1.0f)
	{
		xVec3Set(&right, 1.0f, 0.0f, 0.0f);
		xVec3Set(&fwd, 0.0f, 1.0f, 0.0f);
	}
	else
	{
		xVector3f major = {0.0f, 0.0f, 1.0f};
		xVec3Cross(&right, &up, &major);
		xVec3Normalize(&right, &right);
		xVec3Cross(&fwd, &up, &right);
	}
	decal_vertex* vertices = X_UNCACHED(d->decals[d->index].vertices);
	vertices[0].u = 0;
	vertices[0].v = 0;
	vertices[0].x = p->x + 0.5f*d->size*(-right.x - fwd.x);
	vertices[0].y = p->y + 0.5f*d->size*(-right.y - fwd.y);
	vertices[0].z = p->z + 0.5f*d->size*(-right.z - fwd.z);
	vertices[1].u = 0;
	vertices[1].v = 127;
	vertices[1].x = p->x + 0.5f*d->size*(-right.x + fwd.x);
	vertices[1].y = p->y + 0.5f*d->size*(-right.y + fwd.y);
	vertices[1].z = p->z + 0.5f*d->size*(-right.z + fwd.z);
	vertices[2].u = 127;
	vertices[2].v = 127;
	vertices[2].x = p->x + 0.5f*d->size*(right.x + fwd.x);
	vertices[2].y = p->y + 0.5f*d->size*(right.y + fwd.y);
	vertices[2].z = p->z + 0.5f*d->size*(right.z + fwd.z);

	vertices[3].u = 127;
	vertices[3].v = 127;
	vertices[3].x = p->x + 0.5f*d->size*(right.x + fwd.x);
	vertices[3].y = p->y + 0.5f*d->size*(right.y + fwd.y);
	vertices[3].z = p->z + 0.5f*d->size*(right.z + fwd.z);
	vertices[4].u = 127;
	vertices[4].v = 0;
	vertices[4].x = p->x + 0.5f*d->size*(right.x - fwd.x);
	vertices[4].y = p->y + 0.5f*d->size*(right.y - fwd.y);
	vertices[4].z = p->z + 0.5f*d->size*(right.z - fwd.z);
	vertices[5].u = 0;
	vertices[5].v = 0;
	vertices[5].x = p->x + 0.5f*d->size*(-right.x - fwd.x);
	vertices[5].y = p->y + 0.5f*d->size*(-right.y - fwd.y);
	vertices[5].z = p->z + 0.5f*d->size*(-right.z - fwd.z);

	d->index += 1;
	if (d->index >= d->max_decals)
		d->index = 0;
	if (d->num_decals < d->max_decals)
		d->num_decals += 1;
}

void bg3_draw_decals(bg3_decals* d)
{
	if (d == NULL) return;
	sceGuDepthOffset(100);

	xGuSaveStates();
	sceGuDisable(GU_LIGHTING);
	sceGuDisable(GU_DITHER);
	sceGuDepthMask(GU_TRUE);

	sceGumDrawArray(GU_TRIANGLES, GU_TEXTURE_8BIT|GU_VERTEX_32BITF|GU_TRANSFORM_3D, 6*d->num_decals, 0, d->decals);

	sceGuDepthMask(GU_FALSE);
	xGuLoadStates();

	sceGuDepthOffset(0);
}

bg3_shadow* bg3_create_shadow(int psm, int width, int height)
{
	bg3_shadow* s = (bg3_shadow*)x_malloc(sizeof(bg3_shadow));
	if (s == NULL) return NULL;
	//s->proj = ...;
	s->buf0 = xBufferConstruct(psm, width, height);
	if (s->buf0 == NULL)
	{
		bg3_free_shadow(s);
		return NULL;
	}
	return s;
}

void bg3_free_shadow(bg3_shadow* s)
{
	if (s != NULL)
	{
		if (s->buf0 != NULL)
		{
			x_free(s->buf0);
		}
		x_free(s);
	}
}

ScePspFMatrix4 save_projection;
ScePspFMatrix4 save_view;

void bg3_shadow_projection(ScePspFMatrix4* m, ScePspFVector3* center, ScePspFVector3* lightdir, float fovy, float dist)
{
	if (!m) return;
	gumLoadIdentity(m);
	gumPerspective(m, fovy, 1.0f, 0.0f, 1000.0f);
	//sceGumOrtho(-2.0f, 2.0f, -2.0f, 2.0f, 1000.0f, 0.0f);
	ScePspFVector3 eye = {center->x + dist*lightdir->x, center->y + dist*lightdir->y, center->z + dist*lightdir->z};
	ScePspFVector3 up = {0.0f, 0.0f, 1.0f};
	ScePspFMatrix4 view;
	gumLoadIdentity(&view);
	gumLookAt(&view, &eye, center, &up);
	gumMultMatrix(m, m, &view);
}

int bg3_shadow_setrendertarget(bg3_shadow* s, ScePspFVector3* center, ScePspFVector3* lightdir, float fovy, float dist, float intensity, int clear)
{
	if (!s || !center || !lightdir) return 1;
	if (!s->buf0) return 1;
	if (xBufferSetRenderTarget(s->buf0) != 0) return 1;
	if (clear) xGuClear(0xffffffff);
	sceGumMatrixMode(GU_PROJECTION);
	sceGumStoreMatrix(&save_projection);
	sceGumLoadIdentity();
	sceGumPerspective(fovy, 1.0f, 0.0f, 1000.0f);
	//sceGumOrtho(-2.0f, 2.0f, -2.0f, 2.0f, 1000.0f, 0.0f);
	sceGumStoreMatrix(&s->proj);
	sceGumMatrixMode(GU_VIEW);
	sceGumStoreMatrix(&save_view);
	sceGumLoadIdentity();
	ScePspFVector3 eye = {center->x + dist*lightdir->x, center->y + dist*lightdir->y, center->z + dist*lightdir->z};
	ScePspFVector3 up = {0.0f, 0.0f, 1.0f};
	sceGumLookAt(&eye, center, &up);
	ScePspFMatrix4 view;
	sceGumStoreMatrix(&view);
	gumMultMatrix(&s->proj, &s->proj, &view);
	sceGumMatrixMode(GU_MODEL);
	xGuSaveStates();
	sceGuDisable(GU_LIGHTING);
	sceGuDisable(GU_TEXTURE_2D);
	sceGuDisable(GU_DITHER);
	sceGuDisable(GU_DEPTH_TEST);
	//sceGuDisable(GU_CULL_FACE); //temporary fix - i dont know whats causing the artifacts!
	sceGuDepthMask(GU_TRUE);
	sceGuColor(GU_COLOR(intensity, intensity, intensity, 1.0f));
	return 0;
}

void bg3_shadow_endrendertarget()
{
	sceGuColor(0xffffffff);
	sceGuDepthMask(GU_FALSE);
	xGuLoadStates();
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadMatrix(&save_view);
	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadMatrix(&save_projection);
	sceGumMatrixMode(GU_MODEL);
	xGuRenderToScreen();
}

ScePspFMatrix4 texProjScaleTrans = {
	{0.5f, 0.0f, 0.0f, 0.0f},
	{0.0f, -0.5f, 0.0f, 0.0f},
	{0.0f, 0.0f, 1.0f, 0.0f},
	{0.5f, 0.5f, 0.0f, 1.0f}
};

int bg3_shadow_pass_start(ScePspFMatrix4* proj)
{
	if (!proj) return 1;
	sceGuTexMapMode(GU_TEXTURE_MATRIX, 0, 0);
	sceGuTexProjMapMode(GU_POSITION);
	sceGumMatrixMode(GU_MODEL);
	ScePspFMatrix4 world, texmat;
	sceGumStoreMatrix(&world);
	gumMultMatrix(&texmat, &texProjScaleTrans, proj);
	gumMultMatrix(&texmat, &texmat, &world);
	sceGuSetMatrix(GU_TEXTURE, &texmat);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);
	sceGuColor(0xffffffff);
	xGuSaveStates();
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_SRC_COLOR, 0, 0);
	sceGuDisable(GU_DITHER);
	sceGuDisable(GU_LIGHTING);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuDepthMask(GU_TRUE);
	return 0;
}

/* must be called AFTER world transform! this means you have to call for each object with different position */
int bg3_shadowbuf_pass_start(bg3_shadow* s)
{
	if (!s) return 1;
	if (!s->buf0) return 1;
	//xBuffer4x4Pcf(s->buf0, s->buf1);
	//xBufferDrawA2B(s->buf0, s->buf1);
	xBufferSetImage(s->buf0);
	return bg3_shadow_pass_start(&s->proj);
}

void bg3_shadow_pass_end()
{
	sceGuDepthMask(GU_FALSE);
	xGuLoadStates();
	sceGuTexMapMode(GU_TEXTURE_COORDS, 0, 0);
}

void bg3_envmap_pass_start(float angle, float tx, float ty)
{
	sceGuDepthMask(GU_TRUE);
	xGuSaveStates();
	sceGuDisable(GU_LIGHTING);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_COLOR, GU_FIX, 0, 0xffffffff);
	float s, c;
	x_sincos(angle, &s, &c);
	ScePspFVector3 envmapMatrixColumns[2] = {
		{ c,  s, tx },
		{ -s, c, ty }
	};
	sceGuLight( 2, GU_DIRECTIONAL, GU_DIFFUSE, &envmapMatrixColumns[0] );
	sceGuLight( 3, GU_DIRECTIONAL, GU_DIFFUSE, &envmapMatrixColumns[1] );
	sceGuTexMapMode(GU_ENVIRONMENT_MAP, 2, 3);
}

void bg3_envmap_pass_end()
{
	sceGuTexMapMode(0, 0, 0);
	xGuLoadStates();
	sceGuDepthMask(GU_FALSE);
}

int bg3_check_visibility(xHeightmap* h, ScePspFVector3* eye, ScePspFVector3* center)
{
	if (!h || !eye || !center) return -1.0f;
	xVector3f dir;
	xVec3Sub(&dir, (xVector3f*)center, (xVector3f*)eye);
	return (bg3_ray_heightmap_collision(h, 0, eye, (ScePspFVector3*)&dir, 1.0f, X_EPSILON) >= 1.0f);
}
