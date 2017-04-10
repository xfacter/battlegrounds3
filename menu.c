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

/* main menu
	- Singleplayer
	- Multiplayer
	- Stats
	- Options
	- Quit
*/

enum menu_screens {
	MENU_SCREEN_PRESS_START,
	MENU_SCREEN_MAIN,
	MENU_SCREEN_SINGLEPLAYER,
	MENU_SCREEN_MULTIPLAYER,
	MENU_SCREEN_STATS,
	MENU_SCREEN_OPTIONS,
	MENU_SCREEN_QUIT,
	MENU_SCREEN_CHOOSE_MAP,
	MENU_SCREEN_SETUP_GAME
};

enum menu_interp {
	INTERP_LEFT,
	INTERP_NONE,
	INTERP_RIGHT
};

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
	xTextPrintf(x+1, y+1, buffer);
	xTextSetColor(GU_COLOR(1.0f, 1.0f, 1.0f, alpha));
	xTextPrintf(x, y, buffer);
	xTextSetColor(0xffffffff);
}

int inverted;
float deadzone;
int control_style;

void menu_save_settings(bg3_base* base)
{
	base->inverted = inverted;
	base->deadzone = deadzone;
	base->control_style = control_style;

	FILE* file = fopen("./config.ini", "w");
	if (file == NULL) return;
	fprintf(file, "[config]\r\ninverted = %s\r\ndeadzone = %.2f\r\nstyle = %s", (inverted == 0 ? "false" : "true"), deadzone, (control_style == 0 ? "ANALOG_LOOK" : "ANALOG_MOVE"));
	fclose(file);
}

void bg3_menu_loop(bg3_base* base)
{
	if (base == NULL) return;
	base->transition = BG3_FADE_IN;
	base->fade = 1.0f;
	bg3_map* map = bg3_load_map(99);
	if (map == NULL)
	{
		xExit();
		return;
	}

	ScePspFMatrix4 view;
	gumLoadIdentity(&view);
	view.w.x = 60.0f;
	view.w.y = 30.0f;
	view.w.z = xHeightmapGetHeight(&map->hmp, 0, view.w.x, view.w.y) + 2.5f;
	gumRotateX(&view, DEG_TO_RAD(90.0f));

	xSoundBuffer* music = xSoundLoadBufferWav("./data/title.wav");
	xSoundPlay(music);

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
		xExit();
		return;
	}

	int screen = (base->started ? MENU_SCREEN_MAIN : MENU_SCREEN_PRESS_START);
	float time = 0.0f;
	int exiting = 0;

	int selection = 0;
	float interp_time = 0.0f;
	int interp = INTERP_NONE;
	float update = 0.0f;

#define DEFAULT_SCORE_TARGET_ID 1
#define DEFAULT_GAME_TIME_ID 1
#define DEFAULT_SPAWN_TIME_ID 2
#define DEFAULT_LASER_AMMO 4
#define DEFAULT_TSHELL_AMMO 4
#define DEFAULT_MISSILE_AMMO 2

	bg3_game_values game_values = {
		BG3_GAME_DM,
		DEFAULT_SCORE_TARGET_ID,
		DEFAULT_GAME_TIME_ID,
		DEFAULT_SPAWN_TIME_ID,
		DEFAULT_LASER_AMMO,
		DEFAULT_TSHELL_AMMO,
		DEFAULT_MISSILE_AMMO
	};

#define GAME_SETUP_OPTIONS 7

	int map_sel = 0;

	bg3_stats stats;

	xTimeUpdate();
	while (xRunning() && base->state == BG3_STATE_MENU)
	{
		xTimeUpdate();
		float dt = xTimeGetDeltaTime();
		time += dt;
		xCtrlUpdate(dt);
		switch (screen)
		{
		case MENU_SCREEN_PRESS_START:
			if (xCtrlPress(PSP_CTRL_START) || xCtrlPress(PSP_CTRL_CROSS))
			{
				screen = MENU_SCREEN_MAIN;
				base->started = 1;
			}
			break;
		case MENU_SCREEN_MAIN:
			if (xCtrlTap(PSP_CTRL_UP))
			{
				if (selection > 0)
					selection -= 1;
			}
			if (xCtrlTap(PSP_CTRL_DOWN))
			{
				if (selection < 4)
					selection += 1;
			}
			if (xCtrlTap(PSP_CTRL_CROSS))
			{
				switch (selection)
				{
				case 0: screen = MENU_SCREEN_SINGLEPLAYER; break;
				case 1: screen = MENU_SCREEN_MULTIPLAYER; break;
				case 2:
					bg3_load_stats(&stats);
					screen = MENU_SCREEN_STATS;
					break;
				case 3:
					inverted = base->inverted;
					deadzone = base->deadzone;
					control_style = base->control_style;
					screen = MENU_SCREEN_OPTIONS;
					break;
				case 4: screen = MENU_SCREEN_QUIT; break;
				}
				selection = 0;
			}
			break;
		case MENU_SCREEN_SINGLEPLAYER:
			if (xCtrlTap(PSP_CTRL_UP))
			{
				if (selection > 0)
					selection -= 1;
			}
			if (xCtrlTap(PSP_CTRL_DOWN))
			{
				if (selection < 3)
					selection += 1;
			}
			if (xCtrlTap(PSP_CTRL_CROSS))
			{
				if (selection < 3)
				{
					switch (selection)
					{
					case 0: game_values.game_type = BG3_GAME_DM; break;
					case 1: game_values.game_type = BG3_GAME_TDM; break;
					case 2: game_values.game_type = BG3_GAME_CTF; break;
					}
					screen = MENU_SCREEN_CHOOSE_MAP;
					interp = INTERP_NONE;
					interp_time = 0.0f;
				}
				else
				{
					screen = MENU_SCREEN_MAIN;
				}
				selection = 0;
			}
			if (xCtrlTap(PSP_CTRL_CIRCLE))
			{
				screen = MENU_SCREEN_MAIN;
				selection = 0;
			}
			break;
		case MENU_SCREEN_MULTIPLAYER:
			if (xCtrlTap(PSP_CTRL_CROSS) || xCtrlTap(PSP_CTRL_CIRCLE))
			{
				screen = MENU_SCREEN_MAIN;
				selection = 1;
			}
			break;
		case MENU_SCREEN_STATS:
			if (xCtrlTap(PSP_CTRL_CROSS) || xCtrlTap(PSP_CTRL_CIRCLE))
			{
				screen = MENU_SCREEN_MAIN;
				selection = 2;
			}
			break;
		case MENU_SCREEN_OPTIONS:
			if (xCtrlTap(PSP_CTRL_UP))
			{
				if (selection > 0)
				{
					selection -= 1;
				}
			}
			if (xCtrlTap(PSP_CTRL_DOWN))
			{
				if (selection < 3)
				{
					selection += 1;
				}
			}
			switch (selection)
			{
			case 0:
				//inverted
				if (xCtrlTap(PSP_CTRL_LEFT) || xCtrlTap(PSP_CTRL_RIGHT))
				{
					inverted = !inverted;
				}
				break;
			case 1:
				//deadzone
				if (xCtrlTap(PSP_CTRL_LEFT))
				{
					if (deadzone > 0.0f)
					{
						deadzone -= 0.05f;
						if (deadzone < 0.0f)
						{
							deadzone = 0.0f;
						}
					}
				}
				if (xCtrlTap(PSP_CTRL_RIGHT))
				{
					if (deadzone < 1.0f)
					{
						deadzone += 0.05f;
						if (deadzone > 1.0f)
						{
							deadzone = 1.0f;
						}
					}
				}
				break;
			case 2:
				//style
				if (xCtrlTap(PSP_CTRL_LEFT) || xCtrlTap(PSP_CTRL_RIGHT))
				{
					control_style = !control_style;
				}
				break;
			case 3:
				//apply
				if (xCtrlTap(PSP_CTRL_CROSS))
				{
					menu_save_settings(base);
					screen = MENU_SCREEN_MAIN;
					selection = 0;
				}
				break;
			}
			if (xCtrlTap(PSP_CTRL_CIRCLE))
			{
				screen = MENU_SCREEN_MAIN;
				selection = 3;
			}
			break;
		case MENU_SCREEN_QUIT:
			if (xCtrlTap(PSP_CTRL_CIRCLE))
			{
				screen = MENU_SCREEN_MAIN;
				selection = 4;
			}
			if (xCtrlTap(PSP_CTRL_LEFT) || xCtrlTap(PSP_CTRL_RIGHT))
			{
				selection = !selection;
			}
			switch (selection)
			{
			case 0:
				if (xCtrlTap(PSP_CTRL_CROSS))
				{
					screen = MENU_SCREEN_MAIN;
					selection = 4;
				}
				break;
			case 1:
				if (xCtrlTap(PSP_CTRL_CROSS))
				{
					exiting = 1;
					base->transition = BG3_FADE_OUT;
					base->fade = 0.0f;
				}
				break;
			}
			break;
		case MENU_SCREEN_CHOOSE_MAP:
			if (interp == INTERP_NONE)
			{
				if (xCtrlPress(PSP_CTRL_LEFT))
				{
					if (selection > 0)
					{
						interp = INTERP_LEFT;
						interp_time = 0.0f;
					}
				}
				if (xCtrlPress(PSP_CTRL_RIGHT))
				{
					if (selection < p->num-1)
					{
						interp = INTERP_RIGHT;
						interp_time = 0.0f;
					}
				}
				if (xCtrlTap(PSP_CTRL_CROSS))
				{
					screen = MENU_SCREEN_SETUP_GAME;
					map_sel = selection;
					selection = GAME_SETUP_OPTIONS-1;
					game_values.score_target_id = DEFAULT_SCORE_TARGET_ID;
					game_values.game_time_id = DEFAULT_GAME_TIME_ID;
					game_values.spawn_time_id = DEFAULT_SPAWN_TIME_ID;
					game_values.spawn_ammo_laser = DEFAULT_LASER_AMMO;
					game_values.spawn_ammo_tshells = DEFAULT_TSHELL_AMMO;
					game_values.spawn_ammo_missiles = DEFAULT_MISSILE_AMMO;
				}
				if (xCtrlTap(PSP_CTRL_CIRCLE))
				{
					screen = MENU_SCREEN_SINGLEPLAYER;
					selection = 0;
				}
			}
			break;
		case MENU_SCREEN_SETUP_GAME:
			update += dt;
			if (xCtrlTap(PSP_CTRL_UP))
			{
				if (selection > 0)
					selection -= 1;
			}
			if (xCtrlTap(PSP_CTRL_DOWN))
			{
				if (selection < GAME_SETUP_OPTIONS-1)
					selection += 1;
			}
#define MENU_UPDATE_TIME 0.15f
			if (xCtrlPress(PSP_CTRL_LEFT))
			{
				if (update >= MENU_UPDATE_TIME)
				{
					update = 0.0f;
					switch (selection)
					{
					case 0:
						if (game_values.score_target_id > 0)
							game_values.score_target_id -= 1;
						break;
					case 1:
						if (game_values.game_time_id > 0)
							game_values.game_time_id -= 1;
						break;
					case 2:
						if (game_values.spawn_time_id > 0)
							game_values.spawn_time_id -= 1;
						break;
					case 3:
						if (game_values.spawn_ammo_laser > 0)
							game_values.spawn_ammo_laser -= 1;
						break;
					case 4:
						if (game_values.spawn_ammo_tshells > 0)
							game_values.spawn_ammo_tshells -= 1;
						break;
					case 5:
						if (game_values.spawn_ammo_missiles > 0)
							game_values.spawn_ammo_missiles -= 1;
						break;
					}
				}

			}
			if (xCtrlPress(PSP_CTRL_RIGHT))
			{
				if (update >= MENU_UPDATE_TIME)
				{
					update = 0.0f;
					switch (selection)
					{
					case 0:
						if (game_values.score_target_id < BG3_GAME_SCORES_NUM-1)
							game_values.score_target_id += 1;
						break;
					case 1:
						if (game_values.game_time_id < BG3_GAME_TIMES_NUM-1)
							game_values.game_time_id += 1;
						break;
					case 2:
						if (game_values.spawn_time_id < BG3_GAME_SPAWN_NUM-1)
							game_values.spawn_time_id += 1;
						break;
					case 3:
						if (game_values.spawn_ammo_laser < LASER_MAX_AMMO)
							game_values.spawn_ammo_laser += 1;
						break;
					case 4:
						if (game_values.spawn_ammo_tshells < TSHELL_MAX_AMMO)
							game_values.spawn_ammo_tshells += 1;
						break;
					case 5:
						if (game_values.spawn_ammo_missiles < MISSILE_MAX_AMMO)
							game_values.spawn_ammo_missiles += 1;
						break;
					}
				}
			}
			if (xCtrlTap(PSP_CTRL_CROSS))
			{
				if (selection == GAME_SETUP_OPTIONS-1)
				{
					bg3_base_load_game(base, p->previews[map_sel].id, &game_values);
					if (base->game.loaded)
					{
						bg3_base_init_effects(base);
						bg3_base_load_resources(base);
						base->transition = BG3_FADE_OUT;
						base->fade = 0.0f;
						xTimeUpdate();
					}
				}
			}
			if (xCtrlTap(PSP_CTRL_CIRCLE))
			{
				screen = MENU_SCREEN_CHOOSE_MAP;
				selection = 0;
			}
			break;
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

		switch (base->transition)
		{
		case BG3_FADE_IN:
			base->fade -= dt/MENU_FADE_TIME;
			if (base->fade <= 0.0f)
			{
				base->fade = 0.0f;
				base->transition = BG3_NO_TRANSITION;
			}
			break;
		case BG3_FADE_OUT:
			base->fade += dt/MENU_FADE_TIME;
			if (base->fade >= 1.0f)
			{
				base->fade = 1.0f;
				base->transition = BG3_NO_TRANSITION;
				if (exiting)
				{
					base->state = BG3_STATE_EXIT;
				}
				else
				{
					base->state = BG3_STATE_GAME;
				}

			}
			break;
		}

		bg3_set_blend(BLEND_AVERAGE_WITH_ALPHA, 0);

#define MENU_BOX_WIDTH 250
#define MENU_BOX_HEIGHT 100
#define MENU_BOX_Y 100

#define MENU_TEXT_HEIGHT	15
#define MENU_SELECT_HEIGHT	16

#define MENU_NO_X			(X_SCREEN_WIDTH/2-40)
#define MENU_YES_X			(X_SCREEN_WIDTH/2+40)
#define MENU_YESNO_WIDTH	50

#define MENU_START_X		(X_SCREEN_WIDTH/2-100)
#define MENU_START_Y		110
#define MENU_VALUE_X		(X_SCREEN_WIDTH/2+100)

#define MENU_START2_X 260
#define MENU_VALUE2_X 420
#define MENU_START2_Y 80

		int x, y;
		int width;

		if (!exiting)
		{
			bg3_draw_tex_center(base->logo_tex, X_SCREEN_WIDTH/2, 40);

			if (screen == MENU_SCREEN_PRESS_START)
			{
				xTextSetAlign(X_ALIGN_CENTER);
				float alpha = 0.6f + 0.4f*x_sinf(2*time);
				bg3_print_text_alpha(X_SCREEN_WIDTH/2, 200, alpha, "Press Start");
			}
			else if (screen == MENU_SCREEN_CHOOSE_MAP)
			{
				if (interp != INTERP_NONE)
				{
					interp_time += dt/MENU_INTERP_TIME;
					if (interp_time >= 1.0f)
					{
						if (interp == INTERP_LEFT)
						{
							selection -= 1;
						}
						else
						{
							selection += 1;
						}
						interp = INTERP_NONE;
					}
				}

				if (interp == INTERP_LEFT && selection > 1)
				{
					//far left
					x = (int)interpolate((float)TEX_FAR_LEFT, (float)TEX_LEFT, interp_time);
					width = TEX_SMALL_SIZE;
					bg3_menu_draw_rect_center(x, X_SCREEN_HEIGHT/2, width+2, width+2, 0xff000000);
					bg3_menu_draw_tex_center(p->previews[selection-2].preview->terrain_tex, x, X_SCREEN_HEIGHT/2, width, width);
				}

				if (selection > 0)
				{
					//left
					x = (int)interpolate((float)TEX_LEFT, (float)(interp == INTERP_RIGHT ? TEX_FAR_LEFT : TEX_MIDDLE), (interp == INTERP_NONE ? 0.0f : interp_time));
					width = (int)interpolate((float)TEX_SMALL_SIZE, (float)(interp == INTERP_RIGHT ? TEX_SMALL_SIZE : TEX_LARGE_SIZE), (interp == INTERP_NONE ? 0.0f : interp_time));
					bg3_menu_draw_rect_center(x, X_SCREEN_HEIGHT/2, width+2, width+2, 0xff000000);
					bg3_menu_draw_tex_center(p->previews[selection-1].preview->terrain_tex, x, X_SCREEN_HEIGHT/2, width, width);
				}

				//middle
				x = (int)interpolate((float)TEX_MIDDLE, (float)(interp == INTERP_RIGHT ? TEX_LEFT : TEX_RIGHT), (interp == INTERP_NONE ? 0.0f : interp_time));
				width = (int)interpolate((float)TEX_LARGE_SIZE, (float)TEX_SMALL_SIZE, (interp == INTERP_NONE ? 0.0f : interp_time));
				bg3_menu_draw_rect_center(x, X_SCREEN_HEIGHT/2, width+2, width+2, 0xff000000);
				bg3_menu_draw_tex_center(p->previews[selection].preview->terrain_tex, x, X_SCREEN_HEIGHT/2, width, width);

				if (selection < p->num-1)
				{
					//right
					x = (int)interpolate((float)TEX_RIGHT, (float)(interp == INTERP_LEFT ? TEX_FAR_RIGHT : TEX_MIDDLE), (interp == INTERP_NONE ? 0.0f : interp_time));
					width = (int)interpolate((float)TEX_SMALL_SIZE, (float)(interp == INTERP_LEFT ? TEX_SMALL_SIZE : TEX_LARGE_SIZE), (interp == INTERP_NONE ? 0.0f : interp_time));
					bg3_menu_draw_rect_center(x, X_SCREEN_HEIGHT/2, width+2, width+2, 0xff000000);
					bg3_menu_draw_tex_center(p->previews[selection+1].preview->terrain_tex, x, X_SCREEN_HEIGHT/2, width, width);
				}

				if (interp == INTERP_RIGHT && selection < p->num-2)
				{
					//far right
					x = (int)interpolate((float)TEX_FAR_RIGHT, (float)TEX_RIGHT, interp_time);
					width = TEX_SMALL_SIZE;
					bg3_menu_draw_rect_center(x, X_SCREEN_HEIGHT/2, width+2, width+2, 0xff000000);
					bg3_menu_draw_tex_center(p->previews[selection+2].preview->terrain_tex, x, X_SCREEN_HEIGHT/2, width, width);
				}

				xTextSetAlign(X_ALIGN_CENTER);
				x = X_SCREEN_WIDTH/2;
				y = 205;
				bg3_print_text(x, y, p->previews[selection].preview->name);
				y += MENU_TEXT_HEIGHT;
				bg3_print_text(x, y, "%ix%i", p->previews[selection].preview->width, p->previews[selection].preview->width);
				y += MENU_TEXT_HEIGHT;
				bg3_print_text(x, y, "%i Players", p->previews[selection].preview->players);
			}
			else if (screen == MENU_SCREEN_SETUP_GAME)
			{

				x = X_SCREEN_WIDTH/4;
				y = X_SCREEN_HEIGHT/2;
				width = TEX_LARGE_SIZE;
				bg3_menu_draw_rect_center(x, y, width+2, width+2, 0xff000000);
				bg3_menu_draw_tex_center(p->previews[map_sel].preview->terrain_tex, x, y, width, width);

				y = MENU_START2_Y + selection*MENU_TEXT_HEIGHT;
				width = MENU_VALUE2_X - MENU_START2_X + 20;
				bg3_draw_box(MENU_START2_X - 10, y, width, MENU_SELECT_HEIGHT, 0xff7f7f7f, 0xff000000);

				y = MENU_START2_Y;
				xTextSetAlign(X_ALIGN_LEFT);
				bg3_print_text(MENU_START2_X, y, "Score Limit:");
				xTextSetAlign(X_ALIGN_RIGHT);
				bg3_print_text(MENU_VALUE2_X, y, "%i", bg3_game_scores[game_values.score_target_id]);
				y += MENU_TEXT_HEIGHT;
				xTextSetAlign(X_ALIGN_LEFT);
				bg3_print_text(MENU_START2_X, y, "Time Limit:");
				xTextSetAlign(X_ALIGN_RIGHT);
				bg3_print_text(MENU_VALUE2_X, y, "%i:%02i", bg3_game_times[game_values.game_time_id]/60, bg3_game_times[game_values.game_time_id]%60);
				y += MENU_TEXT_HEIGHT;
				xTextSetAlign(X_ALIGN_LEFT);
				bg3_print_text(MENU_START2_X, y, "Spawn Time:");
				xTextSetAlign(X_ALIGN_RIGHT);
				bg3_print_text(MENU_VALUE2_X, y, "%i s", bg3_game_spawns[game_values.spawn_time_id]);
				y += MENU_TEXT_HEIGHT;
				xTextSetAlign(X_ALIGN_LEFT);
				bg3_print_text(MENU_START2_X, y, "Laser Ammo:");
				xTextSetAlign(X_ALIGN_RIGHT);
				bg3_print_text(MENU_VALUE2_X, y, "%i", game_values.spawn_ammo_laser);
				y += MENU_TEXT_HEIGHT;
				xTextSetAlign(X_ALIGN_LEFT);
				bg3_print_text(MENU_START2_X, y, "Tank Shell Ammo:");
				xTextSetAlign(X_ALIGN_RIGHT);
				bg3_print_text(MENU_VALUE2_X, y, "%i", game_values.spawn_ammo_tshells);
				y += MENU_TEXT_HEIGHT;
				xTextSetAlign(X_ALIGN_LEFT);
				bg3_print_text(MENU_START2_X, y, "Missile Ammo:");
				xTextSetAlign(X_ALIGN_RIGHT);
				bg3_print_text(MENU_VALUE2_X, y, "%i", game_values.spawn_ammo_missiles);
				y += MENU_TEXT_HEIGHT;
				xTextSetAlign(X_ALIGN_CENTER);
				bg3_print_text(MENU_START2_X + (MENU_VALUE2_X-MENU_START2_X)/2, y, "Start Game");
			}
			else
			{

				bg3_draw_box(X_SCREEN_WIDTH/2-MENU_BOX_WIDTH/2, MENU_BOX_Y, MENU_BOX_WIDTH, MENU_BOX_HEIGHT, 0x7f555555, 0xff000000);

				switch (screen)
				{
				case MENU_SCREEN_MAIN:
					y = MENU_START_Y + selection*MENU_TEXT_HEIGHT;
					width = 100;
					bg3_draw_box(X_SCREEN_WIDTH/2-width/2, y, width, MENU_SELECT_HEIGHT, 0xff7f7f7f, 0xff000000);

					xTextSetAlign(X_ALIGN_CENTER);
					y = MENU_START_Y;
					bg3_print_text(X_SCREEN_WIDTH/2, y, "Singleplayer");
					y += MENU_TEXT_HEIGHT;
					bg3_print_text(X_SCREEN_WIDTH/2, y, "Multiplayer");
					y += MENU_TEXT_HEIGHT;
					bg3_print_text(X_SCREEN_WIDTH/2, y, "View Stats");
					y += MENU_TEXT_HEIGHT;
					bg3_print_text(X_SCREEN_WIDTH/2, y, "Options");
					y += MENU_TEXT_HEIGHT;
					bg3_print_text(X_SCREEN_WIDTH/2, y, "Quit");
					break;
				case MENU_SCREEN_SINGLEPLAYER:
					y = MENU_START_Y + selection*MENU_TEXT_HEIGHT;
					width = 130;
					bg3_draw_box(X_SCREEN_WIDTH/2-width/2, y, width, MENU_SELECT_HEIGHT, 0xff7f7f7f, 0xff000000);

					xTextSetAlign(X_ALIGN_CENTER);
					y = MENU_START_Y;
					bg3_print_text(X_SCREEN_WIDTH/2, y, "Deathmatch");
					y += MENU_TEXT_HEIGHT;
					bg3_print_text(X_SCREEN_WIDTH/2, y, "Team Deathmatch");
					y += MENU_TEXT_HEIGHT;
					bg3_print_text(X_SCREEN_WIDTH/2, y, "Capture The Flag");
					y += MENU_TEXT_HEIGHT;
					bg3_print_text(X_SCREEN_WIDTH/2, y, "Back");
					break;
				case MENU_SCREEN_MULTIPLAYER:
					y = MENU_START_Y + selection*MENU_TEXT_HEIGHT;
					width = 130;
					bg3_draw_box(X_SCREEN_WIDTH/2-width/2, y, width, MENU_SELECT_HEIGHT, 0xff7f7f7f, 0xff000000);

					xTextSetAlign(X_ALIGN_CENTER);
					y = MENU_START_Y;
					bg3_print_text(X_SCREEN_WIDTH/2, y, "Back");
					break;
				case MENU_SCREEN_STATS:
					y = MENU_START_Y + selection*MENU_TEXT_HEIGHT;
					width = 130;
					bg3_draw_box(X_SCREEN_WIDTH/2-width/2, y, width, MENU_SELECT_HEIGHT, 0xff7f7f7f, 0xff000000);

					xTextSetAlign(X_ALIGN_CENTER);
					y = MENU_START_Y;
					bg3_print_text(X_SCREEN_WIDTH/2, y, "Back");
					y += MENU_TEXT_HEIGHT;
					xTextSetAlign(X_ALIGN_LEFT);
					bg3_print_text(MENU_START_X, y, "Games Played:");
					xTextSetAlign(X_ALIGN_RIGHT);
					bg3_print_text(MENU_VALUE_X, y, "%i", stats.games_played);
					y += MENU_TEXT_HEIGHT;
					xTextSetAlign(X_ALIGN_LEFT);
					bg3_print_text(MENU_START_X, y, "Kills:");
					xTextSetAlign(X_ALIGN_RIGHT);
					bg3_print_text(MENU_VALUE_X, y, "%i", stats.score_history.kills);
					y += MENU_TEXT_HEIGHT;
					xTextSetAlign(X_ALIGN_LEFT);
					bg3_print_text(MENU_START_X, y, "Assists:");
					xTextSetAlign(X_ALIGN_RIGHT);
					bg3_print_text(MENU_VALUE_X, y, "%i", stats.score_history.assists);
					y += MENU_TEXT_HEIGHT;
					xTextSetAlign(X_ALIGN_LEFT);
					bg3_print_text(MENU_START_X, y, "Deaths:");
					xTextSetAlign(X_ALIGN_RIGHT);
					bg3_print_text(MENU_VALUE_X, y, "%i", stats.score_history.deaths);
					y += MENU_TEXT_HEIGHT;
					xTextSetAlign(X_ALIGN_LEFT);
					bg3_print_text(MENU_START_X, y, "Suicides:");
					xTextSetAlign(X_ALIGN_RIGHT);
					bg3_print_text(MENU_VALUE_X, y, "%i", stats.score_history.suicides);
					y += MENU_TEXT_HEIGHT;
					xTextSetAlign(X_ALIGN_LEFT);
					bg3_print_text(MENU_START_X, y, "Shots Fired:");
					xTextSetAlign(X_ALIGN_RIGHT);
					bg3_print_text(MENU_VALUE_X, y, "%.2f", stats.score_history.shots_fired);
					y += MENU_TEXT_HEIGHT;
					xTextSetAlign(X_ALIGN_LEFT);
					bg3_print_text(MENU_START_X, y, "Shots Hit:");
					xTextSetAlign(X_ALIGN_RIGHT);
					bg3_print_text(MENU_VALUE_X, y, "%.2f", stats.score_history.shots_hit);
					y += MENU_TEXT_HEIGHT;
					xTextSetAlign(X_ALIGN_LEFT);
					bg3_print_text(MENU_START_X, y, "Objectives Met:");
					xTextSetAlign(X_ALIGN_RIGHT);
					bg3_print_text(MENU_VALUE_X, y, "%i", stats.score_history.objectives_achieved);
					break;
				case MENU_SCREEN_OPTIONS:
					y = MENU_START_Y + selection*MENU_TEXT_HEIGHT;
					width = MENU_VALUE_X - MENU_START_X + 20;
					bg3_draw_box(MENU_START_X - 10, y, width, MENU_SELECT_HEIGHT, 0xff7f7f7f, 0xff000000);

					y = MENU_START_Y;
					xTextSetAlign(X_ALIGN_LEFT);
					bg3_print_text(MENU_START_X, y, "Look Inversion:");
					xTextSetAlign(X_ALIGN_RIGHT);
					bg3_print_text(MENU_VALUE_X, y, "%s", (inverted ? "Yes" : "No"));
					y += MENU_TEXT_HEIGHT;
					xTextSetAlign(X_ALIGN_LEFT);
					bg3_print_text(MENU_START_X, y, "Analog Deadzone:");
					xTextSetAlign(X_ALIGN_RIGHT);
					bg3_print_text(MENU_VALUE_X, y, "%.2f", deadzone);
					y += MENU_TEXT_HEIGHT;
					xTextSetAlign(X_ALIGN_LEFT);
					bg3_print_text(MENU_START_X, y, "Control Style:");
					xTextSetAlign(X_ALIGN_RIGHT);
					bg3_print_text(MENU_VALUE_X, y, "%s", (control_style ? "Analog Move" : "Analog Look"));
					y += MENU_TEXT_HEIGHT;
					xTextSetAlign(X_ALIGN_CENTER);
					bg3_print_text(X_SCREEN_WIDTH/2, y, "Apply");
					break;
				case MENU_SCREEN_QUIT:
					xTextSetAlign(X_ALIGN_CENTER);
					y = MENU_START_Y;
					bg3_print_text(X_SCREEN_WIDTH/2, y, "Are you sure?");
					y += MENU_TEXT_HEIGHT;
					if (selection == 0)
					{
						bg3_draw_box(MENU_NO_X-MENU_YESNO_WIDTH/2, y, MENU_YESNO_WIDTH, MENU_SELECT_HEIGHT, 0xff7f7f7f, 0xff000000);
					}
					else
					{
						bg3_draw_box(MENU_YES_X-MENU_YESNO_WIDTH/2, y, MENU_YESNO_WIDTH, MENU_SELECT_HEIGHT, 0xff7f7f7f, 0xff000000);
					}
					bg3_print_text(MENU_NO_X, y, "No");
					bg3_print_text(MENU_YES_X, y, "Yes");
					break;
				}
			}
		}

		bg3_set_blend(BLEND_AVERAGE_WITH_ALPHA, 0);
		bg3_draw_rect(0, 0, X_SCREEN_WIDTH, X_SCREEN_HEIGHT, GU_COLOR(0.0f, 0.0f, 0.0f, base->fade));

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

	xSoundSetStateAll(X_SOUND_STOP, 0);
	xSoundFreeBuffer(music);

	bg3_free_map(map);
}
