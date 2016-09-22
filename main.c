#include <stdio.h>
#include <math.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspkernel.h>
#include "xlib/xlib.h"
#include "xlib/xmem.h"
#include "xlib/xctrl.h"
#include "xlib/xtime.h"
#include "xlib/xgraphics.h"
#include "xlib/xsound.h"
#include "xlib/xtexture.h"
#include "xlib/xmath.h"
#include "xlib/xtext.h"
#include "xlib/xobj.h"
#include "xlib/xheightmap.h"
#include "xlib/xini.h"
#include "xlib/xparticle.h"
#include "xlib/xbuffer.h"
#include "bg3_util.h"
#include "map.h"
#include "base.h"
#include "game.h"
#include "menu.h"

#define FADE_TIME 2.0f
#define FADE_WAIT 2.0f

int xMain()
{
	x_auto_srand();

	xTimeInit();
	xCtrlInit();

	xGuInit();
	xGuPerspective(75.0f, 0.5f, 1000.0f);
	xGuEnable(X_DITHER_SMOOTH);
	//xGuEnable(X_PSEUDO_AA);
	sceGuColor(0xffffffff);
	xGuTexMode(GU_TFX_MODULATE, 1);
	sceGuEnable(GU_TEXTURE_2D);
	xGuTexFilter(X_BILINEAR);

	sceGuEnable(GU_LIGHTING);
	sceGuEnable(GU_LIGHT0);

	xSoundInit(32);
	xSound3dSpeedOfSound(100.0f);

	xTexture* font_tex = xTexLoadTGA("./data/font.tga", 0, X_TEX_TOP_IN_VRAM|X_TEX_GRAY_TO_ALPHA);
	xBitmapFont* font = xTextLoadFont(font_tex, "./data/width.fw");
	xTextSetFont(font);
	xTextSetColor(0xffffffff);
	xTextSetScale(1.0f);
	xTextSetAlign(X_ALIGN_LEFT);

#ifndef X_DEBUG
	//xTexture* logo = xTexLoadTGA("./data/dashhacks.tga", 0, 0);

	int stage = 1;
	float fade = 1.0f;
	float time = 0.0f;
	xTimeUpdate();
	xGuEnable(X_WAIT_VBLANK);
	while (xRunning() && stage < 4)
	{
		xTimeUpdate();
		xGuClear(0xffffff);
		float dt = xTimeGetDeltaTime();
		if (stage == 1)
		{
			fade -= dt/FADE_TIME;
			if (fade <= 0.0f)
			{
				fade = 0.0f;
				stage = 2;
			}
		}
		else if (stage == 2)
		{
			time += dt;
			if (time >= FADE_WAIT)
			{
				stage = 3;
			}
		}
		else
		{
			fade += dt/FADE_TIME;
			if (fade >= 1.0f)
			{
				fade = 1.0f;
				stage = 4;
			}
		}
		sceGuDisable(GU_LIGHTING);
		sceGuDisable(GU_BLEND);
		//bg3_draw_tex(logo, 0, 0);
		bg3_draw_rect(0, 0, X_SCREEN_WIDTH, X_SCREEN_HEIGHT, GU_RGBA(0,0,0,0));
		xTextSetAlign(X_ALIGN_CENTER);
		xTextPrintf(X_SCREEN_WIDTH/2, X_SCREEN_HEIGHT/2, "Battlegrounds 3");
		sceGuEnable(GU_BLEND);
		sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
		bg3_draw_rect(0, 0, X_SCREEN_WIDTH, X_SCREEN_HEIGHT, GU_COLOR(0.0f, 0.0f, 0.0f, fade));
		xGuFrameEnd();
	}
	xGuDisable(X_WAIT_VBLANK);

	//xTexFree(logo);
#endif

	bg3_base* base = bg3_create_base();
	if (base == NULL) return 1;

	/*
	bg3_base_load_resources(base);
	bg3_base_init_effects(base);
	bg3_base_load_game(base, 1);
	if (!base->game.loaded) return 1;
	*/

	//void* memry = x_malloc(20*1024*1024);

    while(xRunning())
	{
		switch (base->state)
		{
		case BG3_STATE_MENU:
			bg3_menu_loop(base);
			break;
		case BG3_STATE_GAME:
			bg3_game_loop(base);
			break;
		default:
			xExit();
			break;
		}
    }

	bg3_destroy_base(base);
	xTexFree(font_tex);
	xTextFreeFont(font);

	xGuEnd();
	xSoundEnd();

	sceKernelExitGame();
    return 0;
}
