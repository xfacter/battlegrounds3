#include <stdarg.h>
#include <pspgu.h>
#include <pspgum.h>
#include "xlib/xmath.h"
#include "xlib/xmem.h"
#include "xlib/xlib.h"
#include "xlib/xtime.h"
#include "xlib/xctrl.h"
#include "xlib/xgraphics.h"

#include "menu.h"

#define SCORCH_DECALS 16
#define BULLET_DECALS 64

#define TEX_SMALL_SIZE	64
#define TEX_LARGE_SIZE	128
#define TEX_FAR_LEFT	-10
#define TEX_LEFT		1*X_SCREEN_WIDTH/4
#define TEX_MIDDLE		2*X_SCREEN_WIDTH/4
#define TEX_RIGHT		3*X_SCREEN_WIDTH/4
#define TEX_FAR_RIGHT	X_SCREEN_WIDTH+10

#define MENU_INTERP_TIME 0.25f
#define MENU_FADE_TIME 1.0f

typedef struct preview {
	int id;
	bg3_map_preview* preview;
} preview;

typedef struct previews {
	int num;
	int max;
	preview* previews;
} previews;

void bg3_menu_free_previews(previews* p);

previews* bg3_menu_load_previews(int max)
{
	previews* p = (previews*)x_malloc(sizeof(previews));
	if (p == NULL) return NULL;
	p->previews = (preview*)x_malloc(max*sizeof(preview));
	if (p->previews == NULL)
	{
		bg3_menu_free_previews(p);
		return NULL;
	}
	p->max = max;
	p->num = 0;
	int i;
	for (i = 0; i < max; i++)
	{
		bg3_map_preview* map_preview = bg3_load_map_preview(i+1);
		if (map_preview != NULL)
		{
			p->previews[p->num].id = i+1;
			p->previews[p->num].preview = map_preview;
			p->num += 1;
		}
	}
	return p;
}

void bg3_menu_free_previews(previews* p)
{
	if (p != NULL)
	{
		if (p->previews != NULL)
		{
			int i;
			for (i = 0; i < p->num; i++)
			{
				bg3_free_map_preview(p->previews[i].preview);
			}
			x_free(p->previews);
		}
		x_free(p);
	}
}

void bg3_menu_draw_tex_center(xTexture* tex, int x, int y, int w, int h)
{
	if (tex == NULL) return;
	xTexDraw(tex, x - w/2, y - h/2, w, h, 0, 0, tex->width, tex->height);
}

void bg3_menu_draw_rect_center(int x, int y, int w, int h, u32 c)
{
	bg3_draw_rect(x-w/2, y-h/2, w, h, c);
}

float interpolate(float x, float y, float t)
{
	return x + (y-x)*t;
}

void bg3_print_text_alpha(int x, int y, float alpha, char* fmt, ...)
{
	char buffer[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buffer, 256, fmt, ap);
	va_end(ap);
	xTextSetColor(GU_COLOR(0.0f, 0.0f, 0.0f, alpha));
	xTextPrintf(x-1, y-1, buffer);
	xTextSetColor(GU_COLOR(1.0f, 1.0f, 1.0f, alpha));
	xTextPrintf(x, y, buffer);
	xTextSetColor(0xffffffff);
}

void bg3_menu_loop(bg3_base* base)
{
	if (base == NULL) return;
	bg3_map* map = bg3_load_map(99);
	if (map == NULL)
	{
		base->state = BG3_EXIT;
		return;
	}

	ScePspFMatrix4 view;
	gumLoadIdentity(&view);
	view.w.x = 60.0f;
	view.w.y = 30.0f;
	view.w.z = xHeightmapGetHeight(&map->hmp, 0, view.w.x, view.w.y) + 2.5f;
	gumRotateX(&view, DEG_TO_RAD(90.0f));

	xObj* sky_obj = xObjLoad("./data/sky.obj", 1);
	xTexture* smoke_tex = xTexLoadTGA("./data/smoke.tga", 0, 0);
	xParticleSystem* smoke_ps = xParticleSystemConstruct(64);
	if (smoke_ps != NULL)
	{
		smoke_ps->pos.x = view.w.x + 5.0f;
		smoke_ps->pos.y = view.w.y + 10.0f;
		smoke_ps->pos.z = xHeightmapGetHeight(&map->hmp, 0, smoke_ps->pos.x, smoke_ps->pos.y);
		xVec3Set(&smoke_ps->pos_rand, 1.0f, 1.0f, 1.0f);
		xVec3Set(&smoke_ps->vel, 0.0f, 0.0f, 4.0f);
		xVec3Set(&smoke_ps->vel_rand, 1.0f, 1.0f, 2.0f);
		xVec3Set(&smoke_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&smoke_ps->colors[0], 0.0f, 0.0f, 0.0f, 0.5f);
		xCol4Set(&smoke_ps->colors[1], 0.75f, 0.75f, 0.75f, 0.5f);
		xCol4Set(&smoke_ps->colors[2], 0.0f, 0.0f, 0.0f, 0.5f);
		smoke_ps->num_cols = 3;
		smoke_ps->sizes[0] = 2.0f;
		smoke_ps->sizes[1] = 8.0f;
		smoke_ps->num_sizes = 2;
		smoke_ps->size_rand = 1.0f;
		smoke_ps->life = 4.0f;
		smoke_ps->life_rand = 1.0f;
		smoke_ps->rate = 16;
		smoke_ps->prim = X_PARTICLE_SPRITES;

		int i;
		for (i = 0; i < 60; i++)
		{
			xParticleSystemUpdate(smoke_ps, 1.0f/60);
		}
	}
	xTexture* scorch_tex = xTexLoadTGA("./data/scorch.tga", 0, X_TEX_TOP_IN_VRAM);
	bg3_decals* scorch_decals = bg3_create_decals(SCORCH_DECALS, 4.0f);
	bg3_add_decal(scorch_decals, &map->hmp, (ScePspFVector3*)&smoke_ps->pos);
	xTexture* bulletmark_tex = xTexLoadTGA("./data/bulletmark.tga", 0, X_TEX_TOP_IN_VRAM);
	bg3_decals* bullet_decals = bg3_create_decals(BULLET_DECALS, 0.5f);
	int i;
	for (i = 1; i < SCORCH_DECALS; i++)
	{
		ScePspFVector3 pos;
		pos.x = view.w.x + x_randf(-20.0f, 20.0f);
		pos.y = view.w.y + x_randf(0.0f, 40.0f);
		pos.z = xHeightmapGetHeight(&map->hmp, 0, pos.x, pos.y);
		bg3_add_decal(scorch_decals, &map->hmp, &pos);
	}
	for (i = 0; i < BULLET_DECALS; i++)
	{
		ScePspFVector3 pos;
		pos.x = view.w.x + x_randf(-20.0f, 20.0f);
		pos.y = view.w.y + x_randf(0.0f, 40.0f);
		pos.z = xHeightmapGetHeight(&map->hmp, 0, pos.x, pos.y);
		bg3_add_decal(bullet_decals, &map->hmp, &pos);
	}


	previews* p = bg3_menu_load_previews(50);
	if (p == NULL)
	{
		base->state = BG3_EXIT;
		return;
	}

	int stage = (base->started ? 1 : 0);
	float fade = 1.0f;
	float time = 0.0f;
	int choice = 0;
	float dir = 0.0f;
	float update = 0.0f;

#define LASER_AMMO 10
#define TSHELL_AMMO 10
#define MISSILE_AMMO 5

	int menu_option = 3;
	int laser_ammo = LASER_AMMO;
	int tshell_ammo = TSHELL_AMMO;
	int missile_ammo = MISSILE_AMMO;

	xTimeUpdate();
	while (xRunning() && base->state == BG3_MENU)
	{
		xTimeUpdate();
		float dt = xTimeGetDeltaTime();
		time += dt;
		xCtrlUpdate(dt);
		if (stage == 0)
		{
			if (xCtrlPress(PSP_CTRL_START) || xCtrlPress(PSP_CTRL_CROSS))
			{
				stage = 1;
				base->started = 1;
				fade = 0.0f;
			}
		}
		else if (stage == 1)
		{
			if (xCtrlPress(PSP_CTRL_LEFT))
			{
				if (dir == 0.0f && choice > 0)
				{
					dir = -1.0f;
				}
			}
			if (xCtrlPress(PSP_CTRL_RIGHT))
			{
				if (dir == 0.0f && choice < p->num-1)
				{
					dir = 1.0f;
				}
			}
			if (xCtrlTap(PSP_CTRL_CROSS))
			{
				if (dir == 0.0f)
				{
					stage = 2;
					menu_option = 3;
					laser_ammo = LASER_AMMO;
					tshell_ammo = TSHELL_AMMO;
					missile_ammo = MISSILE_AMMO;
				}
			}
		}
#define MENU_UPDATE_TIME 0.1f
		else if (stage == 2)
		{
			update += dt;
			if (xCtrlTap(PSP_CTRL_UP))
			{
				if (menu_option > 0)
					menu_option -= 1;
			}
			if (xCtrlTap(PSP_CTRL_DOWN))
			{
				if (menu_option < 3)
					menu_option += 1;
			}
			if (xCtrlPress(PSP_CTRL_LEFT))
			{
				if (update >= MENU_UPDATE_TIME)
				{
					update = 0.0f;
					switch (menu_option)
					{
					case 0:
						laser_ammo -= 1;
						break;
					case 1:
						tshell_ammo -= 1;
						break;
					case 2:
						missile_ammo -= 1;
						break;
					}
					if (laser_ammo < 0)
						laser_ammo = 0;
					if (tshell_ammo < 0)
						tshell_ammo = 0;
					if (missile_ammo < 0)
						missile_ammo = 0;
				}

			}
			if (xCtrlPress(PSP_CTRL_RIGHT))
			{
				if (update >= MENU_UPDATE_TIME)
				{
					update = 0.0f;
					switch (menu_option)
					{
					case 0:
						laser_ammo += 1;
						break;
					case 1:
						tshell_ammo += 1;
						break;
					case 2:
						missile_ammo += 1;
						break;
					}
					if (laser_ammo > (int)LASER_MAX_AMMO)
						laser_ammo = (int)LASER_MAX_AMMO;
					if (tshell_ammo > TSHELL_MAX_AMMO)
						tshell_ammo = TSHELL_MAX_AMMO;
					if (missile_ammo > MISSILE_MAX_AMMO)
						missile_ammo = MISSILE_MAX_AMMO;
				}
			}
			if (xCtrlTap(PSP_CTRL_CROSS))
			{
				if (menu_option == 3)
				{
					bg3_base_load_game(base, p->previews[choice].id);
					if (base->game.loaded)
					{
						base->game.spawn_ammo_laser = laser_ammo;
						base->game.spawn_ammo_tshells = tshell_ammo;
						base->game.spawn_ammo_missiles = missile_ammo;
						bg3_base_load_resources(base);
						bg3_base_init_effects(base);
						stage = 3;
						fade = 0.0f;
						xTimeUpdate();
					}
				}
			}
			if (xCtrlTap(PSP_CTRL_CIRCLE))
			{
				stage = 1;
			}
		}

		xParticleSystemUpdate(smoke_ps, dt);

		xGuClear(0xffffffff);

		sceGuDisable(GU_FOG);
		sceGuDisable(GU_LIGHTING);
		sceGuDisable(GU_BLEND);
		sceGuEnable(GU_TEXTURE_2D);
		xGuTexFilter(X_BILINEAR);

		sceGumMatrixMode(GU_VIEW);
		sceGumLoadMatrix(&view);
		sceGumFastInverse();
		
		sceGumMatrixMode(GU_MODEL);

		sceGumLoadIdentity();
		sceGumTranslate((ScePspFVector3*)&view.w);
		xGumScale(500.0f, 500.0f, 500.0f);
		xTexSetImage(map->sky_tex);
		sceGuDepthMask(GU_TRUE);
		xObjDraw(sky_obj, 0);
		sceGuDepthMask(GU_FALSE);

		sceGumLoadIdentity();

		xTexSetImage(map->terrain_tex);
		xHeightmapDraw(&map->hmp);

		sceGuEnable(GU_BLEND);

		xTexSetImage(scorch_tex);
		bg3_set_blend(BLEND_MUL_COLOR, 0);
		bg3_draw_decals(scorch_decals);

		xTexSetImage(bulletmark_tex);
		bg3_set_blend(BLEND_MUL_INV_COLOR, 0);
		bg3_draw_decals(bullet_decals);

		bg3_set_blend(BLEND_MUL_INV_COLOR, 0);
		xTexSetImage(smoke_tex);
		xParticleSystemRender(smoke_ps, &view);

		if (stage == 0 || stage == 1)
		{
			fade -= dt/MENU_FADE_TIME;
			if (fade < 0.0f)
			{
				fade = 0.0f;
			}
			if (stage == 0)
			{
				xTextSetAlign(X_ALIGN_CENTER);
				float alpha = 0.6f + 0.4f*x_sinf(2*time);
				bg3_print_text_alpha(X_SCREEN_WIDTH/2, 200, alpha, "Press Start");
			}
		}
		if (stage > 0)
		{
			if (stage == 3)
			{
				fade += dt/MENU_FADE_TIME;
				if (fade >= 1.0f)
				{
					fade = 1.0f;
					base->state = BG3_GAME_DM;
				}
			}

			if (dir < 0.0f)
			{
				if (dir + dt/MENU_INTERP_TIME > 0.0f)
				{
					dir = 0.0f;
					choice -= 1;
				}
				else
				{
					dir += dt/MENU_INTERP_TIME;
				}
			}
			else if (dir > 0.0f)
			{
				if (dir - dt/MENU_INTERP_TIME < 0.0f)
				{
					dir = 0.0f;
					choice += 1;
				}
				else
				{
					dir -= dt/MENU_INTERP_TIME;
				}
			}

			sceGuDisable(GU_BLEND);

			int x, y, size;

			if (stage == 1)
			{
				if (dir < 0.0f && choice > 1)
				{
					//far left
					x = (int)interpolate((float)TEX_FAR_LEFT, (float)TEX_LEFT, (dir == 0.0f ? 0.0f : 1.0f-x_absf(dir)));
					size = TEX_SMALL_SIZE;
					bg3_menu_draw_rect_center(x, X_SCREEN_HEIGHT/2, size+2, size+2, 0x000000);
					bg3_menu_draw_tex_center(p->previews[choice-2].preview->terrain_tex, x, X_SCREEN_HEIGHT/2, size, size);
				}

				if (choice > 0)
				{
					//left
					x = (int)interpolate((float)TEX_LEFT, (float)(dir >= 0.0f ? -10 : TEX_MIDDLE), (dir == 0.0f ? 0.0f : 1.0f-x_absf(dir)));
					size = (int)interpolate((float)TEX_SMALL_SIZE, (float)(dir >= 0.0f ? TEX_SMALL_SIZE : TEX_LARGE_SIZE), (dir == 0.0f ? 0.0f : 1.0f-x_absf(dir)));
					bg3_menu_draw_rect_center(x, X_SCREEN_HEIGHT/2, size+2, size+2, 0x000000);
					bg3_menu_draw_tex_center(p->previews[choice-1].preview->terrain_tex, x, X_SCREEN_HEIGHT/2, size, size);
				}

				//middle
				x = (int)interpolate((float)TEX_MIDDLE, (float)(dir > 0.0f ? TEX_LEFT : TEX_RIGHT), (dir == 0.0f ? 0.0f : 1.0f-x_absf(dir)));
				size = (int)interpolate((float)TEX_LARGE_SIZE, (float)TEX_SMALL_SIZE, (dir == 0.0f ? 0.0f : 1.0f-x_absf(dir)));
				bg3_menu_draw_rect_center(x, X_SCREEN_HEIGHT/2, size+2, size+2, 0x000000);
				bg3_menu_draw_tex_center(p->previews[choice].preview->terrain_tex, x, X_SCREEN_HEIGHT/2, size, size);

				if (choice < p->num-1)
				{
					//right
					x = (int)interpolate((float)TEX_RIGHT, (float)(dir <= 0.0f ? X_SCREEN_WIDTH+10 : TEX_MIDDLE), (dir == 0.0f ? 0.0f : 1.0f-x_absf(dir)));
					size = (int)interpolate((float)TEX_SMALL_SIZE, (float)(dir <= 0.0f ? TEX_SMALL_SIZE : TEX_LARGE_SIZE), (dir == 0.0f ? 0.0f : 1.0f-x_absf(dir)));
					bg3_menu_draw_rect_center(x, X_SCREEN_HEIGHT/2, size+2, size+2, 0x000000);
					bg3_menu_draw_tex_center(p->previews[choice+1].preview->terrain_tex, x, X_SCREEN_HEIGHT/2, size, size);
				}

				if (dir > 0.0f && choice < p->num-2)
				{
					//far right
					x = (int)interpolate((float)TEX_FAR_RIGHT, (float)TEX_RIGHT, (dir == 0.0f ? 0.0f : 1.0f-x_absf(dir)));
					size = TEX_SMALL_SIZE;
					bg3_menu_draw_rect_center(x, X_SCREEN_HEIGHT/2, size+2, size+2, 0x000000);
					bg3_menu_draw_tex_center(p->previews[choice+2].preview->terrain_tex, x, X_SCREEN_HEIGHT/2, size, size);
				}

				xTextSetAlign(X_ALIGN_CENTER);
				bg3_print_text(X_SCREEN_WIDTH/2, 220, "%ix%i", p->previews[choice].preview->width, p->previews[choice].preview->width);
				bg3_print_text(X_SCREEN_WIDTH/2, 235, "%i Players", p->previews[choice].preview->players);
				xTextSetAlign(X_ALIGN_LEFT);
			}
			else
			{
				x = X_SCREEN_WIDTH/2;
				y = 150;
				size = TEX_SMALL_SIZE;
				bg3_menu_draw_rect_center(x, X_SCREEN_HEIGHT/2, size+2, size+2, 0x000000);
				bg3_menu_draw_tex_center(p->previews[choice].preview->terrain_tex, x, X_SCREEN_HEIGHT/2, size, size);
#define MENU_START_X 170
#define MENU_START_Y 200
#define MENU_VALUE_X 310
				//draw box
				x = MENU_START_X - 10;
				y = MENU_START_Y + menu_option*15;
				int width = MENU_VALUE_X - x + 10;
				int height = 16;
				bg3_draw_rect(x, y, width, height, 0xff7f7f7f);
				bg3_draw_outline(x-1, y-1, width+1, height+1, 0xff000000);

				xTextSetAlign(X_ALIGN_LEFT);
				y = MENU_START_Y;
				bg3_print_text(MENU_START_X, y, "Laser Ammo:");
				y += 15;
				bg3_print_text(MENU_START_X, y, "Tank Shell Ammo:");
				y += 15;
				bg3_print_text(MENU_START_X, y, "Missile Ammo:");

				xTextSetAlign(X_ALIGN_RIGHT);
				y = MENU_START_Y;
				bg3_print_text(MENU_VALUE_X, y, "%i", laser_ammo);
				y += 15;
				bg3_print_text(MENU_VALUE_X, y, "%i", tshell_ammo);
				y += 15;
				bg3_print_text(MENU_VALUE_X, y, "%i", missile_ammo);

				xTextSetAlign(X_ALIGN_CENTER);
				bg3_print_text(X_SCREEN_WIDTH/2, y+15, "Start Game");
			}
		}

		sceGuEnable(GU_BLEND);

		bg3_set_blend(BLEND_AVERAGE_WITH_ALPHA, 0);
		bg3_draw_tex_center(base->logo_tex, X_SCREEN_WIDTH/2, 40);

		bg3_set_blend(BLEND_AVERAGE_WITH_ALPHA, 0);
		bg3_draw_rect(0, 0, X_SCREEN_WIDTH, X_SCREEN_HEIGHT, GU_COLOR(0.0f, 0.0f, 0.0f, fade));

		xGuFrameEnd();
	}

	bg3_free_decals(bullet_decals);
	xTexFree(bulletmark_tex);
	bg3_free_decals(scorch_decals);
	xTexFree(scorch_tex);
	xParticleSystemDestroy(smoke_ps);
	xTexFree(smoke_tex);
	xObjFree(sky_obj);
	bg3_menu_free_previews(p);
	bg3_free_map(map);
}