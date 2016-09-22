#include <stdio.h>
#include <string.h>
#include <pspgu.h>
#include <pspgum.h>
#include "xlib/xmem.h"
#include "xlib/xmath.h"
#include "xlib/xini.h"

#include "base.h"

bg3_base* bg3_create_base()
{
	bg3_base* base = (bg3_base*)x_malloc(sizeof(bg3_base));
	if (base == NULL) return NULL;
	memset(&base->resources, 0, sizeof(bg3_resources));
	base->resources.loaded = 0;
	memset(&base->effects, 0, sizeof(bg3_effects));
	base->effects.loaded = 0;
	memset(&base->game, 0, sizeof(bg3_game));
	base->game.loaded = 0;
	base->game.player = 0;
	base->game.num_players = 0;
	base->game.ai_update_time = 0.0f;
	base->game.fog_near = 1000.0f;
	base->game.fog_far = 1000.0f;
	base->game.fog_color = 0xffffff;
	base->game.paused = 0;
	base->state = BG3_MENU;
	base->started = 0;

	base->logo_tex = xTexLoadTGA("./data/logo.tga", 0, 0);

	base->hover_height = 0.0f;
	xVec3Set((xVector3f*)&base->mgun_offset, 0.0f, 0.0f, 0.0f);
	xVec3Set((xVector3f*)&base->laser_offset, 0.0f, 0.0f, 0.0f);
	xVec3Set((xVector3f*)&base->tshell_offset, 0.0f, 0.0f, 0.0f);
	xVec3Set((xVector3f*)&base->missile_offset, 0.0f, 0.0f, 0.0f);
	xVec3Set((xVector3f*)&base->hit_ellipsoid_pos, 0.0f, 0.0f, 0.0f);
	xVec3Set((xVector3f*)&base->hit_ellipsoid_radii, 1.0f, 1.0f, 1.0f);
	FILE* file = fopen("./data/tank.ini", "r");
	base->hover_height = xIniGetFloat(file, "hover_height", base->hover_height);
	xIniGetVectorf(file, "mgun_offset", &base->mgun_offset.x, &base->mgun_offset.y, &base->mgun_offset.z);
	xIniGetVectorf(file, "laser_offset", &base->laser_offset.x, &base->laser_offset.y, &base->laser_offset.z);
	xIniGetVectorf(file, "tshell_offset", &base->tshell_offset.x, &base->tshell_offset.y, &base->tshell_offset.z);
	xIniGetVectorf(file, "missile_offset", &base->missile_offset.x, &base->missile_offset.y, &base->missile_offset.z);
	xIniGetVectorf(file, "hit_ellipsoid_pos", &base->hit_ellipsoid_pos.x, &base->hit_ellipsoid_pos.y, &base->hit_ellipsoid_pos.z);
	xIniGetVectorf(file, "hit_ellipsoid_radii", &base->hit_ellipsoid_radii.x, &base->hit_ellipsoid_radii.y, &base->hit_ellipsoid_radii.z);
	fclose(file);

	file = fopen("./config.ini", "r");
	base->inverted = xIniGetInt(file, "inverted", 1);
	base->deadzone = xIniGetFloat(file, "deadzone", 0.1f);
	fclose(file);

	return base;
}

void bg3_destroy_base(bg3_base* base)
{
	if (base == NULL) return;
	bg3_base_free_game(base);
	bg3_base_free_effects(base);
	bg3_base_free_resources(base);
	x_free(base);
}

void bg3_base_load_resources(bg3_base* base)
{
	if (base == NULL) return;
	if (base->resources.loaded) return;
	base->resources.sky_obj = xObjLoad("./data/sky.obj", 1);
	base->resources.sphere_obj = xObjLoad("./data/sphere.obj", 1);
	base->resources.tank_base = xObjLoad("./data/tank_base.obj", 1);
	base->resources.tank_top = xObjLoad("./data/tank_top.obj", 1);
	base->resources.tank_turret = xObjLoad("./data/tank_turret.obj", 1);
	if (base->game.map->type == ENV_ARCTIC)
		base->resources.tank_tex = xTexLoadTGA("./data/tank_arctic.tga", 0, X_TEX_TOP_IN_VRAM);
	else
		base->resources.tank_tex = xTexLoadTGA("./data/tank_desert.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.smoke_tex = xTexLoadTGA("./data/dirt_particle.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.dirt_particle_tex = xTexLoadTGA("./data/smoke.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.bulletmark_tex = xTexLoadTGA("./data/bulletmark.tga", 0, X_TEX_TOP_IN_VRAM);
	//base->resources.lasermark_tex = xTexLoadTGA("./data/lasermark.tga", 0, X_TEX_TOP_IN_VRAM);
	//base->resources.bullet_tex = xTexLoadTGA("./data/bullet.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.crosshair_tex = xTexLoadTGA("./data/crosshair.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.shadow_tex = xTexLoadTGA("./data/shadow.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.flash_tex = xTexLoadTGA("./data/flash.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.explosion_tex = xTexLoadTGA("./data/explosion.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.laser_tex = xTexLoadTGA("./data/laser.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.tshell_tex = xTexLoadTGA("./data/tshell.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.lasermark_tex = xTexLoadTGA("./data/lasermark.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.muzzleflash_tex = xTexLoadTGA("./data/muzzleflash.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.scorch_tex = xTexLoadTGA("./data/scorch.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.shield_tex = xTexLoadTGA("./data/shield.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.shield_icon = xTexLoadTGA("./data/shield_icon.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.mgun_icon = xTexLoadTGA("./data/mgun_icon.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.laser_icon = xTexLoadTGA("./data/laser_icon.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.tshell_icon = xTexLoadTGA("./data/tshell_icon.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.missile_icon = xTexLoadTGA("./data/missile_icon.tga", 0, X_TEX_TOP_IN_VRAM);
	base->resources.mgun_sound = xSoundLoadBufferWav("./data/mgun.wav");
	base->resources.laser_sound = xSoundLoadBufferWav("./data/laser.wav");
	if (base->resources.laser_sound != NULL) base->resources.laser_sound->def_loop = X_SOUND_LOOP;
	base->resources.tshell_sound = xSoundLoadBufferWav("./data/tshell.wav");
	base->resources.missile_launch_sound = xSoundLoadBufferWav("./data/missile_launch.wav");
	base->resources.missile_fly_sound = xSoundLoadBufferWav("./data/missile_fly.wav");
	if (base->resources.missile_fly_sound != NULL) base->resources.missile_fly_sound->def_loop = X_SOUND_LOOP;
	base->resources.explosion_sound = xSoundLoadBufferWav("./data/explosion.wav");
	base->resources.powerup_sound = xSoundLoadBufferWav("./data/powerup.wav");
	if (base->game.map->type == ENV_ARCTIC)
		base->resources.wind_sound = xSoundLoadBufferWav("./data/arctic.wav");
	else
		base->resources.wind_sound = xSoundLoadBufferWav("./data/desert.wav");
	if (base->resources.wind_sound != NULL)
	{
		base->resources.wind_sound->def_loop = X_SOUND_LOOP;
		base->resources.wind_sound->def_vol = 0.5f;
	}
	base->resources.loaded = 1;
}

void bg3_base_free_resources(bg3_base* base)
{
	if (base == NULL) return;
	if (!base->resources.loaded) return;
	//free all base resources
	xObjFree(base->resources.sphere_obj);
	xObjFree(base->resources.sky_obj);
	xObjFree(base->resources.tank_base);
	xObjFree(base->resources.tank_top);
	xObjFree(base->resources.tank_turret);
	xTexFree(base->resources.tank_tex);
	xTexFree(base->resources.smoke_tex);
	xTexFree(base->resources.dirt_particle_tex);
	xTexFree(base->resources.bulletmark_tex);
	//xTexFree(base->resources.lasermark_tex);
	//xTexFree(base->resources.bullet_tex);
	xTexFree(base->resources.crosshair_tex);
	xTexFree(base->resources.shadow_tex);
	xTexFree(base->resources.flash_tex);
	xTexFree(base->resources.explosion_tex);
	xTexFree(base->resources.laser_tex);
	xTexFree(base->resources.tshell_tex);
	xTexFree(base->resources.lasermark_tex);
	xTexFree(base->resources.muzzleflash_tex);
	xTexFree(base->resources.scorch_tex);
	xTexFree(base->resources.shield_tex);
	xTexFree(base->resources.shield_icon);
	xTexFree(base->resources.mgun_icon);
	xTexFree(base->resources.laser_icon);
	xTexFree(base->resources.tshell_icon);
	xTexFree(base->resources.missile_icon);
	xSoundFreeBuffer(base->resources.mgun_sound);
	xSoundFreeBuffer(base->resources.laser_sound);
	xSoundFreeBuffer(base->resources.tshell_sound);
	xSoundFreeBuffer(base->resources.missile_launch_sound);
	xSoundFreeBuffer(base->resources.missile_fly_sound);
	xSoundFreeBuffer(base->resources.explosion_sound);
	xSoundFreeBuffer(base->resources.powerup_sound);
	xSoundFreeBuffer(base->resources.wind_sound);
	memset(&base->resources, 0, sizeof(bg3_resources));
	base->resources.loaded = 0;
}

void bg3_base_init_effects(bg3_base* base)
{
	//initialize particle effects...
	if (base == NULL) return;
	if (base->effects.loaded) return;

	base->effects.dirt_ps = xParticleSystemConstruct(32);
	if (base->effects.dirt_ps != NULL)
	{
		xVec3Set(&base->effects.dirt_ps->pos_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.dirt_ps->vel_rand, 0.1f, 0.1f, 0.1f);
		xVec3Set(&base->effects.dirt_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.dirt_ps->colors[0], 255/255.0f, 236/255.0f, 170/255.0f, 1.0f);
		xCol4Set(&base->effects.dirt_ps->colors[1], 255/255.0f, 236/255.0f, 170/255.0f, 0.0f);
		base->effects.dirt_ps->num_cols = 2;
		base->effects.dirt_ps->sizes[0] = 0.8f;
		base->effects.dirt_ps->num_sizes = 1;
		base->effects.dirt_ps->size_rand = 0.0f;
		base->effects.dirt_ps->life = 0.1f;
		base->effects.dirt_ps->life_rand = 0.0f;
		base->effects.dirt_ps->rate = 0;
		base->effects.dirt_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.missile_ps = xParticleSystemConstruct(256);
	if (base->effects.missile_ps)
	{
		xVec3Set(&base->effects.missile_ps->pos_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.missile_ps->vel, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.missile_ps->vel_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.missile_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.missile_ps->colors[0], 255/255.0f, 255/255.0f, 255/255.0f, 1.0f);
		xCol4Set(&base->effects.missile_ps->colors[1], 255/255.0f, 255/255.0f, 255/255.0f, 0.0f);
		base->effects.missile_ps->num_cols = 2;
		base->effects.missile_ps->sizes[0] = 0.8f;
		base->effects.missile_ps->sizes[1] = 1.3f;
		base->effects.missile_ps->num_sizes = 2;
		base->effects.missile_ps->size_rand = 0.2f;
		base->effects.missile_ps->life = 0.5f;
		base->effects.missile_ps->life_rand = 0.0f;
		base->effects.missile_ps->rate = 0;
		base->effects.missile_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.expl_flash_ps = xParticleSystemConstruct(64);
	if (base->effects.expl_flash_ps)
	{
		xVec3Set(&base->effects.expl_flash_ps->pos_rand, 2.0f, 2.0f, 2.0f);
		xVec3Set(&base->effects.expl_flash_ps->vel, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.expl_flash_ps->vel_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.expl_flash_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.expl_flash_ps->colors[0], 255/255.0f, 255/255.0f, 255/255.0f, 1.0f);
		xCol4Set(&base->effects.expl_flash_ps->colors[1], 255/255.0f, 255/255.0f, 255/255.0f, 0.0f);
		base->effects.expl_flash_ps->num_cols = 2;
		base->effects.expl_flash_ps->sizes[0] = 0.0f;
		base->effects.expl_flash_ps->sizes[1] = 30.0f;
		base->effects.expl_flash_ps->num_sizes = 2;
		base->effects.expl_flash_ps->size_rand = 0.0f;
		base->effects.expl_flash_ps->life = 0.2f;
		base->effects.expl_flash_ps->life_rand = 0.0f;
		base->effects.expl_flash_ps->rate = 0;
		base->effects.expl_flash_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.expl_flames_ps = xParticleSystemConstruct(256);
	if (base->effects.expl_flames_ps)
	{
		xVec3Set(&base->effects.expl_flames_ps->pos_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.expl_flames_ps->vel, 0.0f, 0.0f, 4.0f);
		xVec3Set(&base->effects.expl_flames_ps->vel_rand, 5.0f, 5.0f, 5.0f);
		xVec3Set(&base->effects.expl_flames_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.expl_flames_ps->colors[0], 240/255.0f, 143/255.0f, 62/255.0f, 1.0f);
		xCol4Set(&base->effects.expl_flames_ps->colors[1], 240/255.0f, 143/255.0f, 62/255.0f, 0.0f);
		base->effects.expl_flames_ps->num_cols = 2;
		base->effects.expl_flames_ps->sizes[0] = 4.0f;
		base->effects.expl_flames_ps->sizes[1] = 5.2f;
		base->effects.expl_flames_ps->num_sizes = 2;
		base->effects.expl_flames_ps->size_rand = 0.5f;
		base->effects.expl_flames_ps->life = 0.6f;
		base->effects.expl_flames_ps->life_rand = 0.0f;
		base->effects.expl_flames_ps->rate = 0;
		base->effects.expl_flames_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.expl_debris_ps = xParticleSystemConstruct(256);
	if (base->effects.expl_debris_ps)
	{
		xVec3Set(&base->effects.expl_debris_ps->pos_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.expl_debris_ps->vel, 0.0f, 0.0f, 5.0f);
		xVec3Set(&base->effects.expl_debris_ps->vel_rand, 7.5f, 7.5f, 7.5f);
		xVec3Set(&base->effects.expl_debris_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.expl_debris_ps->colors[0], 255/255.0f, 255/255.0f, 255/255.0f, 1.0f);
		xCol4Set(&base->effects.expl_debris_ps->colors[1], 0/255.0f, 0/255.0f, 0/255.0f, 1.0f);
		base->effects.expl_debris_ps->num_cols = 2;
		base->effects.expl_debris_ps->sizes[0] = 3.0f;
		base->effects.expl_debris_ps->sizes[1] = 3.6f;
		base->effects.expl_debris_ps->num_sizes = 2;
		base->effects.expl_debris_ps->size_rand = 0.5f;
		base->effects.expl_debris_ps->life = 0.6f;
		base->effects.expl_debris_ps->life_rand = 0.0f;
		base->effects.expl_debris_ps->rate = 0;
		base->effects.expl_debris_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.expl_sparks_ps = xParticleSystemConstruct(256);
	if (base->effects.expl_sparks_ps)
	{
		xVec3Set(&base->effects.expl_sparks_ps->pos_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.expl_sparks_ps->vel, 0.0f, 0.0f, 10.0f);
		xVec3Set(&base->effects.expl_sparks_ps->vel_rand, 10.0f, 10.0f, 5.0f);
		xVec3Set(&base->effects.expl_sparks_ps->accel, 0.0f, 0.0f, -5.0f);
		xCol4Set(&base->effects.expl_sparks_ps->colors[0], 255/255.0f, 192/255.0f, 0/255.0f, 1.0f);
		xCol4Set(&base->effects.expl_sparks_ps->colors[1], 255/255.0f, 192/255.0f, 0/255.0f, 0.0f);
		base->effects.expl_sparks_ps->num_cols = 2;
		base->effects.expl_sparks_ps->sizes[0] = 0.75f;
		base->effects.expl_sparks_ps->num_sizes = 1;
		base->effects.expl_sparks_ps->size_rand = 0.1f;
		base->effects.expl_sparks_ps->life = 0.5f;
		base->effects.expl_sparks_ps->life_rand = 0.0f;
		base->effects.expl_sparks_ps->rate = 0;
		base->effects.expl_sparks_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.expl_smoke_ps = xParticleSystemConstruct(256);
	if (base->effects.expl_smoke_ps)
	{
		xVec3Set(&base->effects.expl_smoke_ps->pos_rand, 1.0f, 1.0f, 1.0f);
		xVec3Set(&base->effects.expl_smoke_ps->vel, 0.0f, 0.0f, 4.0f);
		xVec3Set(&base->effects.expl_smoke_ps->vel_rand, 1.0f, 1.0f, 2.0f);
		xVec3Set(&base->effects.expl_smoke_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.expl_smoke_ps->colors[0], 0.0f, 0.0f, 0.0f, 0.5f);
		xCol4Set(&base->effects.expl_smoke_ps->colors[1], 0.75f, 0.75f, 0.75f, 0.5f);
		xCol4Set(&base->effects.expl_smoke_ps->colors[2], 0.0f, 0.0f, 0.0f, 0.5f);
		base->effects.expl_smoke_ps->num_cols = 3;
		base->effects.expl_smoke_ps->sizes[0] = 2.0f;
		base->effects.expl_smoke_ps->sizes[1] = 8.0f;
		base->effects.expl_smoke_ps->num_sizes = 2;
		base->effects.expl_smoke_ps->size_rand = 1.0f;
		base->effects.expl_smoke_ps->life = 2.0f;
		base->effects.expl_smoke_ps->life_rand = 1.0f;
		base->effects.expl_smoke_ps->rate = 0;
		base->effects.expl_smoke_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.sparks_ps = xParticleSystemConstruct(32);
	if (base->effects.sparks_ps)
	{
		xVec3Set(&base->effects.sparks_ps->pos_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.sparks_ps->vel, 0.0f, 0.0f, 5.0f);
		xVec3Set(&base->effects.sparks_ps->vel_rand, 2.5f, 2.5f, 0.5f);
		xVec3Set(&base->effects.sparks_ps->accel, 0.0f, 0.0f, -5.0f);
		xCol4Set(&base->effects.sparks_ps->colors[0], 255/255.0f, 220/255.0f, 0/255.0f, 1.0f);
		xCol4Set(&base->effects.sparks_ps->colors[1], 255/255.0f, 220/255.0f, 0/255.0f, 0.0f);
		base->effects.sparks_ps->num_cols = 2;
		base->effects.sparks_ps->sizes[0] = 0.5f;
		base->effects.sparks_ps->num_sizes = 1;
		base->effects.sparks_ps->size_rand = 0.1f;
		base->effects.sparks_ps->life = 1.0f;
		base->effects.sparks_ps->life_rand = 0.0f;
		base->effects.sparks_ps->rate = 0;
		base->effects.sparks_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.smoke_ps = xParticleSystemConstruct(128);
	if (base->effects.smoke_ps)
	{
		xVec3Set(&base->effects.smoke_ps->pos_rand, 0.1f, 0.1f, 0.1f);
		xVec3Set(&base->effects.smoke_ps->vel, 0.0f, 0.0f, 2.0f);
		xVec3Set(&base->effects.smoke_ps->vel_rand, 0.1f, 0.1f, 1.5f);
		xVec3Set(&base->effects.smoke_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.smoke_ps->colors[0], 1.0f, 1.0f, 1.0f, 0.25f);
		xCol4Set(&base->effects.smoke_ps->colors[1], 0.0f, 0.0f, 0.0f, 0.25f);
		base->effects.smoke_ps->num_cols = 2;
		base->effects.smoke_ps->sizes[0] = 1.0f;
		base->effects.smoke_ps->sizes[1] = 3.0f;
		base->effects.smoke_ps->num_sizes = 2;
		base->effects.smoke_ps->size_rand = 0.5f;
		base->effects.smoke_ps->life = 1.0f;
		base->effects.smoke_ps->life_rand = 0.5f;
		base->effects.smoke_ps->friction = 2.0f;
		base->effects.smoke_ps->rate = 0;
		base->effects.smoke_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.lasermark_ps = xParticleSystemConstruct(64);
	if (base->effects.lasermark_ps)
	{
		xVec3Set(&base->effects.lasermark_ps->pos_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.lasermark_ps->vel, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.lasermark_ps->vel_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.lasermark_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.lasermark_ps->colors[0], 255/255.0f, 255/255.0f, 255/255.0f, 1.0f);
		xCol4Set(&base->effects.lasermark_ps->colors[1], 255/255.0f, 255/255.0f, 255/255.0f, 0.0f);
		base->effects.lasermark_ps->num_cols = 2;
		base->effects.lasermark_ps->sizes[0] = 1.0f;
		base->effects.lasermark_ps->sizes[1] = 0.75f;
		base->effects.lasermark_ps->num_sizes = 2;
		base->effects.lasermark_ps->size_rand = 0.25f;
		base->effects.lasermark_ps->life = 0.5f;
		base->effects.lasermark_ps->life_rand = 0.1f;
		base->effects.lasermark_ps->rate = 0;
		base->effects.lasermark_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.recharge_ps = xParticleSystemConstruct(32);
	if (base->effects.recharge_ps)
	{
		xVec3Set(&base->effects.recharge_ps->pos_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.recharge_ps->vel, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.recharge_ps->vel_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.recharge_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.recharge_ps->colors[0], 0/255.0f, 200/255.0f, 255/255.0f, 1.0f);
		xCol4Set(&base->effects.recharge_ps->colors[1], 0/255.0f, 200/255.0f, 255/255.0f, 0.0f);
		base->effects.recharge_ps->num_cols = 2;
		base->effects.recharge_ps->sizes[0] = 2.5f;
		base->effects.recharge_ps->sizes[1] = 1.5f;
		base->effects.recharge_ps->num_sizes = 2;
		base->effects.recharge_ps->size_rand = 0.25f;
		base->effects.recharge_ps->life = 0.25f;
		base->effects.recharge_ps->life_rand = 0.1f;
		base->effects.recharge_ps->rate = 0;
		base->effects.recharge_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.dust_ps = xParticleSystemConstruct(256);
	if (base->effects.dust_ps)
	{
		xVec3Set(&base->effects.dust_ps->pos_rand, 0.5f, 0.5f, 0.5f);
		xVec3Set(&base->effects.dust_ps->vel, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.dust_ps->vel_rand, 0.5f, 0.5f, 0.25f);
		xVec3Set(&base->effects.dust_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.dust_ps->colors[0], 255/255.0f, 236/255.0f, 170/255.0f, 1.0f);
		xCol4Set(&base->effects.dust_ps->colors[1], 255/255.0f, 236/255.0f, 170/255.0f, 0.0f);
		base->effects.dust_ps->num_cols = 2;
		base->effects.dust_ps->sizes[0] = 0.0f;
		base->effects.dust_ps->sizes[1] = 2.0f;
		base->effects.dust_ps->num_sizes = 2;
		base->effects.dust_ps->size_rand = 0.0f;
		base->effects.dust_ps->life = 0.5f;
		base->effects.dust_ps->life_rand = 0.0f;
		base->effects.dust_ps->rate = 0;
		base->effects.dust_ps->prim = X_PARTICLE_SPRITES;
	}

	int num = 0;
	switch (base->game.map->wind)
	{
	case WIND_NONE:
		break;
	case WIND_MEDIUM:
		num = 8;
		break;
	case WIND_FULL:
		num = 16;
		break;
	}
	base->effects.wind_ps = xParticleSystemConstruct(6*num);
	if (base->effects.wind_ps)
	{
		//xVec3Set(&base->effects.wind_ps->pos, 0.5f*map->hmp.tile_scale*(map->hmp.width-1), 0.5f*map->hmp.tile_scale*(map->hmp.width-1), 0.5f*map->hmp.z_scale);
		//xVec3Set(&base->effects.wind_ps->pos_rand, 0.5f*map->hmp.tile_scale*(map->hmp.width-1), 0.5f*map->hmp.tile_scale*(map->hmp.width-1), 0.5f*map->hmp.z_scale);
		xVec3Set(&base->effects.wind_ps->pos_rand, 80.0f, 80.0f, 40.0f);
		xVec3Set(&base->effects.wind_ps->vel, 40.0f, 40.0f, 0.0f);
		xVec3Set(&base->effects.wind_ps->vel_rand, 10.0f, 10.0f, 10.0f);
		xVec3Set(&base->effects.wind_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.wind_ps->colors[0], 0xd7/255.0f, 0xc9/255.0f, 0x7e/255.0f, 0.0f);
		xCol4Set(&base->effects.wind_ps->colors[1], 0xd7/255.0f, 0xc9/255.0f, 0x7e/255.0f, 0.2f);
		xCol4Set(&base->effects.wind_ps->colors[2], 0xd7/255.0f, 0xc9/255.0f, 0x7e/255.0f, 0.0f);
		base->effects.wind_ps->num_cols = 3;
		base->effects.wind_ps->sizes[0] = 60.0f;
		base->effects.wind_ps->num_sizes = 1;
		base->effects.wind_ps->size_rand = 10.0f;
		base->effects.wind_ps->life = 6.0f;
		base->effects.wind_ps->life_rand = 5.0f;
		base->effects.wind_ps->rate = num;
		base->effects.wind_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.laser_ps = xParticleSystemConstruct(2048);
	if (base->effects.laser_ps)
	{
		xVec3Set(&base->effects.laser_ps->pos_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.laser_ps->vel, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.laser_ps->vel_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.laser_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.laser_ps->colors[0], 255/255.0f, 220/255.0f, 0/255.0f, 1.0f);
		xCol4Set(&base->effects.laser_ps->colors[1], 255/255.0f, 220/255.0f, 0/255.0f, 0.0f);
		base->effects.laser_ps->num_cols = 2;
		base->effects.laser_ps->sizes[0] = 0.5f;
		base->effects.laser_ps->num_sizes = 1;
		base->effects.laser_ps->size_rand = 0.0f;
		base->effects.laser_ps->life = 0.05f;
		base->effects.laser_ps->life_rand = 0.0f;
		base->effects.laser_ps->rate = 0;
		base->effects.laser_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.muzzleflash_ps = xParticleSystemConstruct(64);
	if (base->effects.muzzleflash_ps)
	{
		xVec3Set(&base->effects.muzzleflash_ps->pos_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.muzzleflash_ps->vel, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.muzzleflash_ps->vel_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.muzzleflash_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.muzzleflash_ps->colors[0], 255/255.0f, 255/255.0f, 255/255.0f, 1.0f);
		xCol4Set(&base->effects.muzzleflash_ps->colors[1], 255/255.0f, 255/255.0f, 255/255.0f, 0.0f);
		base->effects.muzzleflash_ps->num_cols = 2;
		base->effects.muzzleflash_ps->sizes[0] = 0.5f;
		base->effects.muzzleflash_ps->num_sizes = 1;
		base->effects.muzzleflash_ps->size_rand = 0.1f;
		base->effects.muzzleflash_ps->life = 0.05f;
		base->effects.muzzleflash_ps->life_rand = 0.0f;
		base->effects.muzzleflash_ps->rate = 0;
		base->effects.muzzleflash_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.powerup_ps = xParticleSystemConstruct(1024);
	if (base->effects.powerup_ps)
	{
		xVec3Set(&base->effects.powerup_ps->pos_rand, 0.25f, 0.25f, 0.25f);
		xVec3Set(&base->effects.powerup_ps->vel, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.powerup_ps->vel_rand, 0.5f, 0.5f, 0.1f);
		xVec3Set(&base->effects.powerup_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.powerup_ps->colors[0], 255/255.0f, 200/255.0f, 0/255.0f, 1.0f);
		base->effects.powerup_ps->num_cols = 1;
		base->effects.powerup_ps->sizes[0] = 0.0f;
		base->effects.powerup_ps->sizes[1] = 0.5f;
		base->effects.powerup_ps->sizes[2] = 0.0f;
		base->effects.powerup_ps->num_sizes = 3;
		base->effects.powerup_ps->size_rand = 0.0f;
		base->effects.powerup_ps->life = 1.0f;
		base->effects.powerup_ps->life_rand = 0.1f;
		base->effects.powerup_ps->rate = 0;
		base->effects.powerup_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.gun_smoke_ps = xParticleSystemConstruct(128);
	if (base->effects.gun_smoke_ps)
	{
		xVec3Set(&base->effects.gun_smoke_ps->pos_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.gun_smoke_ps->vel, 0.0f, 0.0f, 0.5f);
		xVec3Set(&base->effects.gun_smoke_ps->vel_rand, 0.1f, 0.1f, 0.25f);
		xVec3Set(&base->effects.gun_smoke_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.gun_smoke_ps->colors[0], 1.0f, 1.0f, 1.0f, 0.25f);
		xCol4Set(&base->effects.gun_smoke_ps->colors[1], 0.0f, 0.0f, 0.0f, 0.25f);
		base->effects.gun_smoke_ps->num_cols = 2;
		base->effects.gun_smoke_ps->sizes[0] = 0.5f;
		base->effects.gun_smoke_ps->num_sizes = 1;
		base->effects.gun_smoke_ps->size_rand = 0.25f;
		base->effects.gun_smoke_ps->life = 0.75f;
		base->effects.gun_smoke_ps->life_rand = 0.25f;
		base->effects.gun_smoke_ps->friction = 2.0f;
		base->effects.gun_smoke_ps->rate = 0;
		base->effects.gun_smoke_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.powerup_pickup_ps = xParticleSystemConstruct(32);
	if (base->effects.powerup_pickup_ps)
	{
		xVec3Set(&base->effects.powerup_pickup_ps->pos_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.powerup_pickup_ps->vel, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.powerup_pickup_ps->vel_rand, 0.0f, 0.0f, 0.0f);
		xVec3Set(&base->effects.powerup_pickup_ps->accel, 0.0f, 0.0f, 0.0f);
		xCol4Set(&base->effects.powerup_pickup_ps->colors[0], 255/255.0f, 255/255.0f, 255/255.0f, 1.0f);
		xCol4Set(&base->effects.powerup_pickup_ps->colors[1], 255/255.0f, 255/255.0f, 255/255.0f, 0.0f);
		base->effects.powerup_pickup_ps->num_cols = 2;
		base->effects.powerup_pickup_ps->sizes[0] = 2.5f;
		base->effects.powerup_pickup_ps->sizes[1] = 1.5f;
		base->effects.powerup_pickup_ps->num_sizes = 2;
		base->effects.powerup_pickup_ps->size_rand = 0.25f;
		base->effects.powerup_pickup_ps->life = 0.25f;
		base->effects.powerup_pickup_ps->life_rand = 0.1f;
		base->effects.powerup_pickup_ps->rate = 0;
		base->effects.powerup_pickup_ps->prim = X_PARTICLE_SPRITES;
	}

	base->effects.shadow = bg3_create_shadow(GU_PSM_5650, 256, 256);
	base->effects.bullet_decals = bg3_create_decals(256, 0.5f);
	base->effects.scorch_decals = bg3_create_decals(256, 4.0f);

	base->effects.loaded = 1;
}

void bg3_base_free_effects(bg3_base* base)
{
	if (base == NULL) return;
	if (!base->effects.loaded) return;
	xParticleSystemDestroy(base->effects.dirt_ps);
	xParticleSystemDestroy(base->effects.missile_ps);
	xParticleSystemDestroy(base->effects.expl_flash_ps);
	xParticleSystemDestroy(base->effects.expl_flames_ps);
	xParticleSystemDestroy(base->effects.expl_debris_ps);
	xParticleSystemDestroy(base->effects.expl_sparks_ps);
	xParticleSystemDestroy(base->effects.expl_smoke_ps);
	xParticleSystemDestroy(base->effects.sparks_ps);
	xParticleSystemDestroy(base->effects.smoke_ps);
	xParticleSystemDestroy(base->effects.lasermark_ps);
	xParticleSystemDestroy(base->effects.recharge_ps);
	xParticleSystemDestroy(base->effects.dust_ps);
	xParticleSystemDestroy(base->effects.wind_ps);
	xParticleSystemDestroy(base->effects.laser_ps);
	xParticleSystemDestroy(base->effects.muzzleflash_ps);
	xParticleSystemDestroy(base->effects.gun_smoke_ps);
	bg3_free_shadow(base->effects.shadow);
	bg3_free_decals(base->effects.bullet_decals);
	bg3_free_decals(base->effects.scorch_decals);
	memset(&base->effects, 0, sizeof(bg3_effects));
	base->effects.loaded = 0;
}

void bg3_base_load_game(bg3_base* base, int map_id)
{
	//initalize players and objects...
	if (base == NULL) return;
	if (base->game.loaded) return;
	bg3_game* g = &base->game;

	g->map = bg3_load_map(map_id);
	if (g->map == NULL) return;
	bg3_map_setup_env(g->map);
	switch (g->map->type)
	{
	case ENV_DESERT:
		g->fog_color = 0x5484a2;
		break;
	case ENV_ARCTIC:
		g->fog_color = 0xdddddd;
		break;
	case ENV_TROPICAL:
		g->fog_color = 0xdddddd;
		break;
	}
	switch (g->map->fog)
	{
	case FOG_NONE:
		g->fog_near = 1000.0f;
		g->fog_far = 1000.0f;
		break;
	case FOG_MEDIUM:
		g->fog_near = 50.0f;
		g->fog_far = 400.0f;
		break;
	case FOG_THICK:
		g->fog_near = 20.0f;
		g->fog_far = 50.0f;
		break;
	}

	g->players = (bg3_player*)x_malloc(g->map->players*sizeof(bg3_player));
	if (g->players == NULL)
	{
		bg3_free_map(g->map);
		g->num_players = 0;
		return;
	}
	g->num_players = g->map->players;
	int i;
	for (i = 0; i < g->map->players; i++)
	{
		g->players[i].team = 0;
		g->players[i].score = 0;
		g->players[i].ai.path = astar_create_path(1024);
	}

	g->tshells = bg3_create_tshells(64);
	g->bullets = bg3_create_bullets(256);
	g->missiles = bg3_create_missiles(64);
	g->powerups = bg3_create_powerups(64);

	for (i = 0; i < g->map->num_powerups; i++)
	{
		ScePspFVector3 pos;
		pos.x = g->map->powerups[i].pos.x*g->map->hmp.tile_scale + 0.5f*g->map->hmp.tile_scale;
		pos.y = g->map->powerups[i].pos.y*g->map->hmp.tile_scale + 0.5f*g->map->hmp.tile_scale;
		pos.z = xHeightmapGetHeight(&g->map->hmp, 0, pos.x, pos.y) + base->hover_height;
		bg3_add_powerup(g->powerups, &pos, g->map->powerups[i].type, 1);
	}

	g->player = 0;
	g->ai_update_time = 0.0f;
	g->paused = 0;
	g->exit = 0;

	g->spawn_ammo_laser = 0;
	g->spawn_ammo_tshells = 0;
	g->spawn_ammo_missiles = 0;

	g->loaded = 1;
}

void bg3_base_free_game(bg3_base* base)
{
	if (base == NULL) return;
	if (!base->game.loaded) return;
	bg3_game* g = &base->game;

	int i;
	for (i = 0; i < g->num_players; i++)
	{
		if (g->players[i].ai.path)
		{
			astar_free_path(g->players[i].ai.path);
		}
	}
	x_free(g->players);
	g->players = NULL;
	bg3_free_map(g->map);
	g->map = NULL;
	bg3_free_tshells(g->tshells);
	g->tshells = NULL;
	bg3_free_bullets(g->bullets);
	g->bullets = NULL;
	bg3_free_missiles(g->missiles);
	g->missiles = NULL;
	bg3_free_powerups(g->powerups);
	g->powerups = NULL;

	g->loaded = 0;
}

static void calc_new_player_matrices(bg3_base* base, int player)
{
	if (base == NULL) return;
	if (player < 0 || player >= base->game.num_players) return;
	bg3_player* p = &base->game.players[player];

	p->base_mat.y.x = p->tank_dir.x;
	p->base_mat.y.y = p->tank_dir.y;
	p->base_mat.y.z = 0.0f;
	x_crossproduct((ScePspFVector3*)&p->base_mat.x, (ScePspFVector3*)&p->base_mat.y, (ScePspFVector3*)&p->base_mat.z);
	x_normalize((ScePspFVector3*)&p->base_mat.x, 1.0f);
	x_crossproduct((ScePspFVector3*)&p->base_mat.y, (ScePspFVector3*)&p->base_mat.z, (ScePspFVector3*)&p->base_mat.x);
	p->base_mat.w.x = p->pos.x;
	p->base_mat.w.y = p->pos.y;
	p->base_mat.w.z = p->pos.z + base->hover_height;
	p->top_mat = p->base_mat;
	p->top_mat.y.x = p->cam_dir.x;
	p->top_mat.y.y = p->cam_dir.y;
	p->top_mat.y.z = 0.0f;
	x_crossproduct((ScePspFVector3*)&p->top_mat.x, (ScePspFVector3*)&p->top_mat.y, (ScePspFVector3*)&p->top_mat.z);
	x_normalize((ScePspFVector3*)&p->top_mat.x, 1.0f);
	x_crossproduct((ScePspFVector3*)&p->top_mat.y, (ScePspFVector3*)&p->top_mat.z, (ScePspFVector3*)&p->top_mat.x);
}

void bg3_spawn_player(bg3_base* base, int player)
{
	if (base == NULL) return;
	if (base->game.players == NULL || player < 0 || player >= base->game.num_players) return;

	bg3_player* players = base->game.players;

	ScePspFVector2 pos;
	bg3_map_random_spawn(base->game.map, &pos);
	players[player].pos.x = pos.x + x_randf(-10.0f, 10.0f);
	players[player].pos.y = pos.y + x_randf(-10.0f, 10.0f);
	players[player].pos.z = xHeightmapGetHeight(&base->game.map->hmp, 0, players[player].pos.x, players[player].pos.y) + 20.0f;
	players[player].vel.x = 0.0f;
	players[player].vel.y = 0.0f;
	players[player].vel.z = 0.0f;
	players[player].acc.x = 0.0f;
	players[player].acc.y = 0.0f;
	players[player].cam_dir.x = 0.0f;
	players[player].cam_dir.y = 1.0f;
	players[player].cam_dir.z = 0.0f;
	players[player].tank_dir = players[player].cam_dir;
	players[player].cam_pos = players[player].pos;
	players[player].height = 0.0f;
	players[player].cam_pitch = 0.0f;
	players[player].shield_fade = 0.0f;
	gumLoadIdentity(&players[player].base_mat);
	gumLoadIdentity(&players[player].top_mat);
	players[player].turret_pitch = 0.0f;
	players[player].cam_look = players[player].cam_dir;
	players[player].cam_orig = players[player].cam_pos;
	players[player].e_pos = *(xVector3f*)&players[player].pos;
	players[player].e_pos.z += 0.5f;
	players[player].normal.x = 0.0f;
	players[player].normal.y = 0.0f;
	players[player].normal.z = 1.0f;

	players[player].laser_time = 0.0f;
	players[player].laser_snd_ref = -1;

	players[player].smoke_time = 0.0f;
	players[player].spark_time = 0.0f;
	players[player].powerup_time = 0.0f;

	players[player].firing = 0;

	players[player].laser_ammo = base->game.spawn_ammo_laser;
	players[player].tshell_ammo = base->game.spawn_ammo_tshells;
	players[player].missile_ammo = base->game.spawn_ammo_missiles;

	players[player].mgun_wait = 0.0f;
	players[player].tshell_wait = 0.0f;
	players[player].missile_wait = 0.0f;

	players[player].shield_wait = 0.0f;
	players[player].death_time = 0.0f;
	players[player].hp_shields = 1.0f;
	players[player].hp_armor = 1.0f;
	players[player].weapons = (1<<BG3_MACHINE_GUN);
	players[player].primary = BG3_MACHINE_GUN;

	players[player].ai.state = BG3_AI_NEUTRAL_WANDER;
	players[player].ai.path_idx = -1;
	players[player].ai.update = 0.0f;
	players[player].ai.rand = 0;
	players[player].ai.fire = 0.0f;

	calc_new_player_matrices(base, player);
}

int bg3_damage_player(bg3_base* base, int player, float shields_dmg, float armor_dmg)
{
	if (base == NULL) return 0;
	bg3_game* g = &base->game;
	if (g->players == NULL || player < 0 || player >= g->num_players) return 0;
	bg3_player* p = &g->players[player];
	if (p->hp_armor <= 0.0f) return 0;

	int killed = 0;

	if (p->hp_shields > 0.0f)
	{
		p->hp_shields -= shields_dmg;
		//p->hp_armor -= 0.5f*armor_dmg;
		if (p->hp_shields < 0.0f)
		{
			float dmg_not_done = -p->hp_shields;
			p->hp_shields = 0.0f;
			bg3_damage_player(base, player, 0.0f, (dmg_not_done/shields_dmg)*armor_dmg);
		}
		p->shield_fade = SHIELD_FADE_TIME;
	}
	else
	{
		/*
		p->hp_shields -= 0.5f*shields_dmg;
		if (p->hp_shields < 0.0f)
		p->hp_shields = 0.0f;
		*/
		p->hp_armor -= armor_dmg;
		if (p->hp_armor <= 0.0f)
		{
			//kill player
			killed = 1;
			p->death_time = 0.0f;
			bg3_create_explosion(base, &p->e_pos);
			//create powerups
			ScePspFVector3 pos = p->pos;
			pos.z = xHeightmapGetHeight(&g->map->hmp, 0, pos.x, pos.y) + base->hover_height;
			ScePspFVector3 powerup_pos;
			powerup_pos.x = pos.x + x_randf(-0.5f, 0.5f);
			powerup_pos.y = pos.y + x_randf(-0.5f, 0.5f);
			powerup_pos.z = pos.z;
			bg3_add_powerup(g->powerups, &powerup_pos, BG3_POWERUP_ARMOR, 0);
			if (p->weapons & (1 << BG3_LASER))
			{
				powerup_pos.x = pos.x + x_randf(-0.5f, 0.5f);
				powerup_pos.y = pos.y + x_randf(-0.5f, 0.5f);
				powerup_pos.z = pos.z;
				bg3_add_powerup(g->powerups, &powerup_pos, BG3_POWERUP_LASER, 0);
			}
			if (p->weapons & (1 << BG3_TANK_SHELL))
			{
				powerup_pos.x = pos.x + x_randf(-0.5f, 0.5f);
				powerup_pos.y = pos.y + x_randf(-0.5f, 0.5f);
				powerup_pos.z = pos.z;
				bg3_add_powerup(g->powerups, &powerup_pos, BG3_POWERUP_TSHELL, 0);
			}
			if (p->weapons & (1 << BG3_MISSILES))
			{
				powerup_pos.x = pos.x + x_randf(-0.5f, 0.5f);
				powerup_pos.y = pos.y + x_randf(-0.5f, 0.5f);
				powerup_pos.z = pos.z;
				bg3_add_powerup(g->powerups, &powerup_pos, BG3_POWERUP_MISSILE, 0);
			}
		}
	}
	p->shield_wait = SHIELD_RECHARGE_WAIT;

	if (p->ai.state < BG3_AI_NEUTRAL_STATES && p->ai.state != BG3_AI_NEUTRAL_SEEK_ACTIVE && p->ai.state != BG3_AI_NEUTRAL_SCAN)
	{
		bg3_ai_set_state(base, player, BG3_AI_NEUTRAL_SCAN);
	}
	return killed;
}

int bg3_damage_area(bg3_base* base, int player, ScePspFVector3* center, float inner_radius, float outer_radius, float shields_dmg, float armor_dmg)
{
	if (base == NULL || center == NULL) return 0;
	bg3_game* g = &base->game;
	if (g->players == NULL || player < 0 || player >= g->num_players) return 0;
	xVector3f dir;
	float dist_sq, dist;
	int killed = 0;
	int i;
	for (i = 0; i < g->num_players; i++)
	{
		xVec3Sub(&dir, &g->players[i].e_pos, (xVector3f*)center);
		dist_sq = xVec3SqLength(&dir);
		if (dist_sq < SQR(inner_radius))
		{
			killed += bg3_damage_player(base, i, shields_dmg, armor_dmg);
		}
		else if (dist_sq < SQR(outer_radius))
		{
			dist = x_sqrtf(dist_sq);
			float dmg = (outer_radius - dist)/(outer_radius - inner_radius);
			killed += bg3_damage_player(base, i, dmg*shields_dmg, dmg*armor_dmg);
		}
	}
	return killed;
}

void bg3_create_explosion(bg3_base* base, xVector3f* pos)
{
	if (base == NULL || pos == NULL) return;
	xParticleEmitter e;
	e.particle_pos = *pos;
	e.new_velocity = 0;
	xParticleSystemBurst(base->effects.expl_flash_ps, &e, 8);
	xParticleSystemBurst(base->effects.expl_flames_ps, &e, 32);
	xParticleSystemBurst(base->effects.expl_debris_ps, &e, 32);
	xParticleSystemBurst(base->effects.expl_sparks_ps, &e, 32);
	xParticleSystemBurst(base->effects.expl_smoke_ps, &e, 8);
	//play sound

	xSound3dSource src;
	src.pos = *(ScePspFVector3*)pos;
	src.vel.x = 0.0f;
	src.vel.y = 0.0f;
	src.vel.z = 0.0f;
	src.radius = SOUND_RADIUS;
	xSound3dPlay(base->resources.explosion_sound, &src, 0);
}

void bg3_ai_set_state(bg3_base* base, int player, int state)
{
	if (base == NULL) return;
	if (player < 0 || player >= base->game.num_players) return;
	bg3_player* p = &base->game.players[player];
	p->ai.state = state;
	p->ai.update = 0.0f;
	p->ai.fire = 0.0f;
	p->ai.rand = x_randi(1, 10);
}

bg3_tshells* bg3_create_tshells(int num)
{
	bg3_tshells* t = (bg3_tshells*)x_malloc(sizeof(bg3_tshells));
	if (t == NULL) return NULL;
	t->num = 0;
	t->max = num;
	t->stack = (int*)x_malloc(num*sizeof(int));
	t->tshells = (tshell*)x_malloc(num*sizeof(tshell));
	if (t->stack == NULL || t->tshells == NULL)
	{
		bg3_free_tshells(t);
		return NULL;
	}
	int i;
	for (i = 0; i < num; i++)
	{
		t->stack[i] = i;
	}
	return t;
}

void bg3_free_tshells(bg3_tshells* t)
{
	if (t != NULL)
	{
		if (t->stack != NULL)
		{
			x_free(t->stack);
		}
		if (t->tshells != NULL)
		{
			x_free(t->tshells);
		}
		x_free(t);
	}
}

void bg3_add_tshell(bg3_tshells* t, bg3_base* base, ScePspFVector3* p0, ScePspFVector3* p1)
{
	if (t == NULL || p0 == NULL || p1 == NULL) return;
	if (t->num >= t->max) return;
	tshell* s = &t->tshells[t->stack[t->num]];
	s->p0 = *p0;
	xVec3Sub((xVector3f*)&s->len, (xVector3f*)p1, (xVector3f*)p0);
	s->time = HUGE_VAL;
	t->num += 1;

	xSound3dSource src;
	src.pos = *p0;
	src.vel.x = 0.0f;
	src.vel.y = 0.0f;
	src.vel.z = 0.0f;
	src.radius = SOUND_RADIUS;
	xSound3dPlay(base->resources.tshell_sound, &src, 0);
}

static void remove_tshell(bg3_tshells* t, int idx)
{
	if (t->num <= 0) return;
	t->num -= 1;
	if (t->num == idx) return;
	u16 temp = t->stack[t->num];
	t->stack[t->num] = t->stack[idx];
	t->stack[idx] = temp;
}

void bg3_update_tshells(bg3_tshells* t, float dt)
{
	if (t == NULL) return;
	int i;
	for (i = 0; i < t->num; i++)
	{
		tshell* s = &t->tshells[t->stack[i]];
		if (s->time == HUGE_VAL)
		{
			s->time = 0.0f;
		}
		else
		{
			s->time += dt;
			if (s->time >= TSHELL_LIFE)
			{
				remove_tshell(t, i);
				i -= 1;
			}
		}
	}
}

void bg3_draw_tshells(bg3_tshells* t, ScePspFMatrix4* view)
{
	if (t == NULL || view == NULL) return;
	sceGuDepthMask(GU_TRUE);
	int i;
	for (i = 0; i < t->num; i++)
	{
		tshell* s = &t->tshells[t->stack[i]];
		float life = s->time/TSHELL_LIFE;
		float h = TSHELL_START_WIDTH + life*(TSHELL_END_WIDTH - TSHELL_START_WIDTH);
		u32 col = GU_COLOR(1.0f, 1.0f, 1.0f, 1.0f - life);
		bg3_draw_quad_billboard((xVector3f*)&view->w, (xVector3f*)&s->p0, (xVector3f*)&s->len, h, h, col, col);
	}
	sceGuDepthMask(GU_FALSE);
}

bg3_bullets* bg3_create_bullets(int num)
{
	bg3_bullets* b = (bg3_bullets*)x_malloc(sizeof(bg3_bullets));
	if (b == NULL) return NULL;
	b->num = 0;
	b->max = num;
	b->stack = (int*)x_malloc(num*sizeof(int));
	b->bullets = (bullet*)x_malloc(num*sizeof(bullet));
	if (b->stack == NULL || b->bullets == NULL)
	{
		bg3_free_bullets(b);
		return NULL;
	}
	int i;
	for (i = 0; i < num; i++)
	{
		b->stack[i] = i;
	}
	return b;
}

void bg3_free_bullets(bg3_bullets* b)
{
	if (b != NULL)
	{
		if (b->stack != NULL)
		{
			x_free(b->stack);
		}
		if (b->bullets != NULL)
		{
			x_free(b->bullets);
		}
		x_free(b);
	}
}

void bg3_add_bullet(bg3_bullets* b, bg3_base* base, int player, ScePspFVector3* pos, ScePspFVector3* vel)
{
	if (b == NULL || pos == NULL || vel == NULL) return;
	if (b->num >= b->max) return;
	bullet* s = &b->bullets[b->stack[b->num]];
	s->player = player;
	s->pos = *pos;
	s->vel = *vel;
	s->time = HUGE_VAL;
	b->num += 1;

	xSound3dSource src;
	src.pos = *pos;
	src.vel.x = 0.0f;
	src.vel.y = 0.0f;
	src.vel.z = 0.0f;
	src.radius = SOUND_RADIUS;
	xSound3dPlay(base->resources.mgun_sound, &src, 0);
}

static void remove_bullet(bg3_bullets* b, int idx)
{
	if (b->num <= 0) return;
	b->num -= 1;
	if (b->num == idx) return;
	u16 temp = b->stack[b->num];
	b->stack[b->num] = b->stack[idx];
	b->stack[idx] = temp;
}

void bg3_update_bullets(bg3_bullets* b, bg3_base* base, float dt)
{
	if (b == NULL || base == NULL) return;
	bg3_game* g = &base->game;
	bg3_player* players = g->players;
	xHeightmap* h = &g->map->hmp;
	bg3_decals* d = base->effects.bullet_decals;
	xParticleSystem* ps = base->effects.dirt_ps;
	int i, j;
	for (i = 0; i < b->num; i++)
	{
		bullet* s = &b->bullets[b->stack[i]];
		if (s->time == HUGE_VAL)
		{
			s->time = 0.0f;
		}
		else
		{
			//need to check for lowest t among enemies and terrain...
			float t;
			int collided = 0;
			for (j = 0; j < g->num_players; j++)
			{
				if (j != s->player && players[j].hp_armor > 0.0f)
				{
					t = bg3_ray_ellipsoid_collision(&players[j].e_mat, &players[j].e_pos, (xVector3f*)&s->pos, (xVector3f*)&s->vel);
					if (t >= 0.0f && t < dt)
					{
						collided = 1;
						xVector3f p, dir;
						xVec3Scale(&dir, (xVector3f*)&s->vel, t);
						xVec3Add(&p, (xVector3f*)&s->pos, &dir);
						//cause damage to enemy
						players[s->player].score += bg3_damage_player(base, j, MGUN_SHIELD_DPS*MGUN_WAIT, MGUN_ARMOR_DPS*MGUN_WAIT);
						remove_bullet(b, i);
						i -= 1;
						break;
					}
				}
			}
			if (!collided)
			{
				ScePspFVector3 new_pos;
				t = bg3_ray_heightmap_collision(h, &new_pos, &s->pos, &s->vel, dt, X_EPSILON);
				if (t < dt)
				{
					//create particles, decals
					bg3_add_decal(d, h, &new_pos);
					ScePspFVector3 n;
					xHeightmapGetNormal(h, &n, new_pos.x, new_pos.y);
					xParticleEmitter e;
					e.particle_pos = *(xVector3f*)&new_pos;
					xVec3Scale(&e.particle_vel, (xVector3f*)&n, 10.0f);
					e.new_velocity = 1;
					xParticleSystemBurst(ps, &e, 1);
					//play sound
					//...
					remove_bullet(b, i);
					i -= 1;
				}
				else
				{
					s->pos = new_pos;
					s->time += dt;
					if (s->time >= BULLET_LIFE)
					{
						remove_bullet(b, i);
						i -= 1;
					}
				}
			}
		}
	}
}

void bg3_draw_bullets(bg3_bullets* b, ScePspFMatrix4* view, float length, float h1, float h2, u32 c1, u32 c2)
{
	if (b == NULL || view == NULL) return;
	sceGuDepthMask(GU_TRUE);
	int i;
	for (i = 0; i < b->num; i++)
	{
		bullet* s = &b->bullets[b->stack[i]];
		xVector3f len;
		xVec3Normalize(&len, (xVector3f*)&s->vel);
		xVec3Scale(&len, &len, length);
		bg3_draw_quad_billboard((xVector3f*)&view->w, (xVector3f*)&s->pos, &len, h1, h2, c1, c2);
	}
	sceGuDepthMask(GU_FALSE);
}

bg3_missiles* bg3_create_missiles(int num)
{
	bg3_missiles* m = (bg3_missiles*)x_malloc(sizeof(bg3_missiles));
	if (m == NULL) return NULL;
	m->num = 0;
	m->max = num;
	m->stack = (int*)x_malloc(num*sizeof(int));
	m->missiles = (missile*)x_malloc(num*sizeof(missile));
	if (m->stack == NULL || m->missiles == NULL)
	{
		bg3_free_missiles(m);
		return NULL;
	}
	int i;
	for (i = 0; i < num; i++)
	{
		m->stack[i] = i;
	}
	return m;
}

void bg3_free_missiles(bg3_missiles* m)
{
	if (m != NULL)
	{
		if (m->stack != NULL)
		{
			x_free(m->stack);
		}
		if (m->missiles != NULL)
		{
			x_free(m->missiles);
		}
		x_free(m);
	}
}

void bg3_add_missile(bg3_missiles* m, bg3_base* base, int player, ScePspFVector3* pos, ScePspFVector3* dir, ScePspFVector3* target)
{
	if (m == NULL || pos == NULL || dir == NULL || target == NULL) return;
	if (m->num >= m->max) return;
	missile* s = &m->missiles[m->stack[m->num]];
	s->player = player;
	s->pos = *pos;
	s->dir = *dir;
	s->target = *target;
	s->time = HUGE_VAL;
	m->num += 1;

	s->snd_src.pos = *pos;
	s->snd_src.vel.x = 0.0f;
	s->snd_src.vel.y = 0.0f;
	s->snd_src.vel.z = 0.0f;
	s->snd_src.radius = SOUND_RADIUS;
	xSound3dPlay(base->resources.missile_launch_sound, &s->snd_src, 0);
	s->snd_ref = xSound3dPlay(base->resources.missile_fly_sound, &s->snd_src, 1);
}

static void remove_missile(bg3_missiles* m, int idx)
{
	if (m->num <= 0) return;
	m->num -= 1;
	if (m->num == idx) return;
	u16 temp = m->stack[m->num];
	m->stack[m->num] = m->stack[idx];
	m->stack[idx] = temp;
}

void bg3_update_missiles(bg3_missiles* m, bg3_base* base, float dt)
{
	if (m == NULL || base == NULL) return;
	bg3_game* g = &base->game;
	bg3_player* players = g->players;
	xHeightmap* h = &g->map->hmp;
	bg3_decals* d = base->effects.scorch_decals;
	xParticleSystem* ps = base->effects.missile_ps;
	float c = x_cosf(MISSILE_SEEK_ANGLE);
	int i, j;
	for (i = 0; i < m->num; i++)
	{
		missile* s = &m->missiles[m->stack[i]];
		if (s->time == HUGE_VAL)
		{
			s->time = 0.0f;
			s->smoke_time = 0.0f;
			xParticleEmitter e;
			e.particle_pos = *(xVector3f*)&s->pos;
			e.new_velocity = 0;
			xParticleSystemBurst(ps, &e, 1);
		}
		else
		{
			float t;
			int collided = 0;
			//see if there is a new target
			int enemy = -1;
			float min_test = SQR(MISSILE_SEEK_RADIUS);
			xVector3f dir;
			for (j = 0; j < g->num_players; j++)
			{
				if (j != s->player && players[j].hp_armor > 0.0f)
				{
					xVec3Sub(&dir, &players[j].e_pos, (xVector3f*)&s->pos);
					float dist_sq = xVec3SqLength(&dir);
					if (dist_sq < SQR(MISSILE_SEEK_RADIUS))
					{
						xVec3Normalize(&dir, &dir);
						float dot = xVec3Dot(&dir, (xVector3f*)&s->dir);
						if (dot >= c)
						{
							//float test = dist_sq*(c + (1.0f - dot));
							float test = dist_sq*(1.0f - dot)/(1.0f - c);
							if (test < min_test)
							{
								enemy = j;
								min_test = test;
							}
						}
					}
				}
			}
			if (enemy >= 0)
			{
				s->target = *(ScePspFVector3*)&players[enemy].e_pos;
			}

			//recompute dir
			xVec3Sub(&dir, (xVector3f*)&s->target, (xVector3f*)&s->pos);
			xVec3Normalize(&dir, &dir);
			if (s->time >= MISSILE_TARGET_TIME)
			{
				float ease = (s->time >= MISSILE_FOLLOW_TIME ? 5.0f : 20.0f);
				x_ease_to_target3(&s->dir, (ScePspFVector3*)&dir, ease, dt);
			}
			xVec3Normalize((xVector3f*)&s->dir, (xVector3f*)&s->dir);
			//find any collision
			for (j = 0; j < g->num_players; j++)
			{
				if (j != s->player && players[j].hp_armor > 0.0f)
				{
					//printf("checking collision with tank\n");
					t = bg3_ray_ellipsoid_collision(&players[j].e_mat, &players[j].e_pos, (xVector3f*)&s->pos, (xVector3f*)&s->dir);
					if (t >= 0.0f && t < MISSILE_VELOCITY*dt)
					{
						collided = 1;
						xVector3f p, dir;
						xVec3Scale(&dir, (xVector3f*)&s->dir, t);
						xVec3Add(&p, (xVector3f*)&s->pos, &dir);
						//bg3_create_ripple(j, (ScePspFVector3*)&p);
						//bg3_damage_player(j, MISSILE_SHIELD_DPS*MISSILE_WAIT, MISSILE_ARMOR_DPS*MISSILE_WAIT);
						players[s->player].score += bg3_damage_area(base, s->player, (ScePspFVector3*)&p, MISSILE_DMG_INNER_RADIUS, MISSILE_DMG_OUTER_RADIUS, 0.5f*MISSILE_SHIELD_DPS*MISSILE_WAIT, 0.5f*MISSILE_ARMOR_DPS*MISSILE_WAIT);
						bg3_create_explosion(base, &p);
						xSoundSetState(s->snd_ref, X_SOUND_STOP);
						remove_missile(m, i);
						i -= 1;
						break;
					}
				}
			}
			if (!collided)
			{
				ScePspFVector3 new_pos;
				//printf("checking collision with terrain\n");
				t = bg3_ray_heightmap_collision(h, &new_pos, &s->pos, &s->dir, MISSILE_VELOCITY*dt, X_EPSILON);
				if (t < MISSILE_VELOCITY*dt)
				{
					//create particles, decals
					players[s->player].score += bg3_damage_area(base, s->player, &new_pos, MISSILE_DMG_INNER_RADIUS, MISSILE_DMG_OUTER_RADIUS, 0.5f*MISSILE_SHIELD_DPS*MISSILE_WAIT, 0.5f*MISSILE_ARMOR_DPS*MISSILE_WAIT);
					bg3_create_explosion(base, (xVector3f*)&new_pos);
					//create scorchmark decal
					bg3_add_decal(d, h, &new_pos);
					xSoundSetState(s->snd_ref, X_SOUND_STOP);
					remove_missile(m, i);
					i -= 1;
				}
				else
				{
					s->pos = new_pos;
					s->smoke_time += dt;
					while (s->smoke_time >= MISSILE_SMOKE_WAIT)
					{
						s->smoke_time -= MISSILE_SMOKE_WAIT;
						xParticleEmitter e;
						e.particle_pos = *(xVector3f*)&s->pos;
						e.new_velocity = 0;
						xParticleSystemBurst(ps, &e, 1);
					}
					s->time += dt;
					if (s->time >= MISSILE_LIFE)
					{
						xSoundSetState(s->snd_ref, X_SOUND_STOP);
						remove_missile(m, i);
						i -= 1;
					}
				}
			}
		}
		s->snd_src.pos = s->pos;
		xVec3Scale((xVector3f*)&s->snd_src.vel, (xVector3f*)&s->dir, MISSILE_VELOCITY);
	}
}

void bg3_draw_missiles(bg3_missiles* m, ScePspFMatrix4* view, float width)
{
	if (m == NULL || view == NULL) return;
	xVector3f up_left = {-view->x.x + view->y.x, -view->x.y + view->y.y, -view->x.z + view->y.z};
	sceGuDepthMask(GU_TRUE);
	int i;
	for (i = 0; i < m->num; i++)
	{
		missile* s = &m->missiles[m->stack[i]];
		bg3_draw_sprite(&up_left, (xVector3f*)&s->pos, width);
	}
	sceGuDepthMask(GU_FALSE);
}

bg3_powerups* bg3_create_powerups(int num)
{
	bg3_powerups* p = (bg3_powerups*)x_malloc(sizeof(bg3_powerups));
	if (p == NULL) return NULL;
	p->num = 0;
	p->max = num;
	p->stack = (int*)x_malloc(num*sizeof(int));
	p->powerups = (powerup*)x_malloc(num*sizeof(powerup));
	if (p->stack == NULL || p->powerups == NULL)
	{
		bg3_free_powerups(p);
		return NULL;
	}
	int i;
	for (i = 0; i < num; i++)
	{
		p->stack[i] = i;
	}
	return p;
}

void bg3_free_powerups(bg3_powerups* p)
{
	if (p != NULL)
	{
		if (p->stack != NULL)
		{
			x_free(p->stack);
		}
		if (p->powerups != NULL)
		{
			x_free(p->powerups);
		}
		x_free(p);
	}
}

void bg3_add_powerup(bg3_powerups* p, ScePspFVector3* pos, int type, int respawns)
{
	if (p == NULL || pos == NULL) return;
	if (p->num >= p->max) return;
	powerup* s = &p->powerups[p->stack[p->num]];
	s->type = type;
	s->respawn = respawns;
	s->pos = *pos;
	s->age = 0.0f;
	s->effect_time = 0.0f;
	p->num += 1;
}

static void remove_powerup(bg3_powerups* p, int idx)
{
	if (p->num <= 0) return;
	p->num -= 1;
	if (p->num == idx) return;
	u16 temp = p->stack[p->num];
	p->stack[p->num] = p->stack[idx];
	p->stack[idx] = temp;
}

void bg3_update_powerups(bg3_powerups* p, bg3_base* base, float dt)
{
	if (p == NULL || base == NULL) return;
	bg3_game* g = &base->game;
	bg3_player* players = g->players;
	int i, j;
	for (i = 0; i < p->num; i++)
	{
		powerup* s = &p->powerups[p->stack[i]];
		int picked_up = 0;
		if (s->respawn != 2)
		{
			for (j = 0; j < g->num_players; j++)
			{
				if (players[j].hp_armor > 0.0f)
				{
					xVector3f dir;
					xVec3Sub(&dir, &players[j].e_pos, (xVector3f*)&s->pos);
					if (xVec3SqLength(&dir) <= SQR(TANK_RADIUS+POWERUP_RADIUS))
					{
						switch (s->type)
						{
						case BG3_POWERUP_ARMOR:
							players[j].hp_armor += ARMOR_POWERUP_AMT;
							if (players[j].hp_armor > 1.0f)
								players[j].hp_armor = 1.0f;
							break;
						case BG3_POWERUP_LASER:
							players[j].laser_ammo += LASER_POWERUP_AMT;
							if (players[j].laser_ammo > LASER_MAX_AMMO)
								players[j].laser_ammo = LASER_MAX_AMMO;
							break;
						case BG3_POWERUP_TSHELL:
							players[j].tshell_ammo += TSHELL_POWERUP_AMT;
							if (players[j].tshell_ammo > TSHELL_MAX_AMMO)
								players[j].tshell_ammo = TSHELL_MAX_AMMO;
							break;
						case BG3_POWERUP_MISSILE:
							players[j].missile_ammo += MISSILE_POWERUP_AMT;
							if (players[j].missile_ammo > MISSILE_MAX_AMMO)
								players[j].missile_ammo = MISSILE_MAX_AMMO;
							break;
						}
						//play sound, make effect
						if (j == g->player && players[j].powerup_time <= 0.0f)
						{
							xSoundPlay(base->resources.powerup_sound);
						}
						players[j].powerup_time = POWERUP_EFFECT_TIME;
						if (s->respawn)
						{
							s->respawn = 2;
							s->age = 0.0f;
						}
						else
						{
							picked_up = 1;
							remove_powerup(p, i);
							i -= 1;
						}
					}
				}
			}
		}

		if (!picked_up)
		{
			if (s->respawn != 2)
			{
				s->effect_time += dt;
				while (s->effect_time >= POWERUP_EFFECT_WAIT)
				{
					s->effect_time -= POWERUP_EFFECT_WAIT;
					//particle burst
					xParticleEmitter e;
					e.particle_pos = *(xVector3f*)&s->pos;
					e.particle_pos.z -= 0.5f;
					e.new_velocity = 0;
					xParticleSystemBurst(base->effects.powerup_ps, &e, 1);
				}
			}

			s->age += dt;
			if (s->age >= POWERUP_LIFE && !s->respawn)
			{
				remove_powerup(p, i);
				i -= 1;
			}

			if (s->age >= POWERUP_RESPAWN_TIME && s->respawn == 2)
			{
				s->respawn = 1;
				s->age = 0.0f;
			}
		}

	}
}

void bg3_draw_powerups(bg3_powerups* p, bg3_base* base, ScePspFMatrix4* view)
{
	if (p == NULL || base == NULL) return;
	xVector3f up_left = {-view->x.x + view->y.x, -view->x.y + view->y.y, -view->x.z + view->y.z};
	int i;
	for (i = 0; i < p->num; i++)
	{
		powerup* s = &p->powerups[p->stack[i]];
		if (s->respawn != 2)
		{
			switch (s->type)
			{
			case BG3_POWERUP_ARMOR:
				xTexSetImage(base->resources.shield_icon);
				break;
			case BG3_POWERUP_LASER:
				xTexSetImage(base->resources.laser_icon);
				break;
			case BG3_POWERUP_TSHELL:
				xTexSetImage(base->resources.tshell_icon);
				break;
			case BG3_POWERUP_MISSILE:
				xTexSetImage(base->resources.missile_icon);
				break;
			}
			bg3_draw_sprite(&up_left, (xVector3f*)&s->pos, 1.0f);
		}

	}
}
