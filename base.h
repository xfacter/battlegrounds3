#ifndef __GAME_H__
#define __GAME_H__

#include "xlib/xconfig.h"
#include "xlib/xparticle.h"
#include "xlib/xtexture.h"
#include "xlib/xtext.h"
#include "xlib/xobj.h"
#include "map.h"
#include "astar.h"
#include "bg3_util.h"
#include "values.h"

typedef struct tshell {
	ScePspFVector3 p0;
	ScePspFVector3 len;
	float time;
} tshell;

typedef struct bg3_tshells {
	int num;
	int max;
	int* stack;
	tshell* tshells;
} bg3_tshells;

typedef struct bullet {
	int player;
	ScePspFVector3 pos;
	ScePspFVector3 vel;
	float time;
} bullet;

typedef struct bg3_bullets {
	int num;
	int max;
	int* stack;
	bullet* bullets;
} bg3_bullets;

typedef struct missile {
	int player;
	ScePspFVector3 pos;
	ScePspFVector3 dir;
	ScePspFVector3 target;
	float time;
	float smoke_time;
} missile;

typedef struct bg3_missiles {
	int num;
	int max;
	int* stack;
	missile* missiles;
} bg3_missiles;

typedef struct powerup {
	int type;
	int respawn;
	ScePspFVector3 pos;
	float age;
	float effect_time;
} powerup;

typedef struct bg3_powerups {
	int num;
	int max;
	int* stack;
	powerup* powerups;
} bg3_powerups;

typedef struct bg3_ai {
	int state;
	astar_path* path;
	int path_idx;
	float update;
	int target;
	int rand;
	float fire;
} bg3_ai;

typedef struct bg3_player {
	int team;
	int score;

	ScePspFVector3 pos;
	ScePspFVector3 vel;
	ScePspFVector3 acc;
	ScePspFVector3 tank_dir;
	ScePspFVector3 cam_pos; //this keeps track of follow
	ScePspFVector3 cam_dir; //this keeps track of rotation on XY plane, 2D only, fix later...
	float height;
	float cam_pitch;
	float shield_fade;
	ScePspFMatrix4 base_mat;
	ScePspFMatrix4 top_mat;
	float turret_pitch;
	ScePspFVector3 cam_look; //these keep track of the actual camera orientation
	ScePspFVector3 cam_orig;
	ScePspFMatrix3 e_mat;
	xVector3f e_pos;
	ScePspFVector3 normal;

	ScePspFVector3 laser_start;
	ScePspFVector3 laser_len;
	//int laser_hit_last;
	float laser_time;

	float smoke_time;
	float spark_time;

	int firing;
	float laser_ammo;
	int tshell_ammo;
	int missile_ammo;
	float mgun_wait;
	float tshell_wait;
	//float gauss_wait;
	float missile_wait;
	float shield_wait;
	float death_time;
	float hp_shields;
	float hp_armor;
	//float boost;
	//float heat;
	int weapons;
	int primary;
	//int secondary;

	bg3_ai ai;
} bg3_player;

typedef struct bg3_resources {
	int loaded;
	xObj* sphere_obj;
	xObj* sky_obj;
	xObj* tank_base;
	xObj* tank_top;
	xObj* tank_turret;
	xTexture* tank_tex;
	xTexture* smoke_tex;
	xTexture* dirt_particle_tex;
	xTexture* bulletmark_tex;
	//xTexture* lasermark_tex;
	//xTexture* bullet_tex;
	xTexture* reticle_tex;
	xTexture* shadow_tex;
	xTexture* flash_tex;
	xTexture* explosion_tex;
	xTexture* laser_tex;
	xTexture* tshell_tex;
	xTexture* scramble_tex;
	xTexture* hp_hud_tex;
	xTexture* hp_armor_tex;
	xTexture* hp_shields_tex;
	xTexture* muzzleflash_tex;
	xTexture* scorch_tex;
	xTexture* shield_tex;
	xTexture* health_tex;
} bg3_resources;

typedef struct bg3_effects {
	int loaded;
	xParticleSystem* dirt_ps;
	xParticleSystem* missile_ps;
	xParticleSystem* expl_flash_ps;
	xParticleSystem* expl_flames_ps;
	xParticleSystem* expl_debris_ps;
	xParticleSystem* expl_sparks_ps;
	xParticleSystem* expl_smoke_ps;
	xParticleSystem* sparks_ps;
	xParticleSystem* smoke_ps;
	xParticleSystem* scramble_ps;
	xParticleSystem* recharge_ps;
	xParticleSystem* dust_ps;
	xParticleSystem* wind_ps;
	xParticleSystem* laser_ps;
	xParticleSystem* muzzleflash_ps;
	xParticleSystem* health_ps;
	xParticleSystem* powerup_ps;
	xParticleSystem* gun_smoke_ps;
	bg3_shadow* shadow;
	bg3_decals* bullet_decals;
	bg3_decals* scorch_decals;
} bg3_effects;

typedef struct bg3_game {
	int loaded;
	int player;
	int num_players;
	bg3_player* players;
	bg3_map* map;
	bg3_tshells* tshells;
	bg3_bullets* bullets;
	bg3_missiles* missiles;
	bg3_powerups* powerups;
	float ai_update_time;
	float fog_near;
	float fog_far;
	u32 fog_color;
	int paused;
	int exit;
	float fade;
} bg3_game;

typedef struct bg3_base {
	xTexture* logo_tex;
	bg3_resources resources;
	bg3_effects effects;
	bg3_game game;
	int state;
	int started;

	float hover_height;
	ScePspFVector3 mgun_offset;
	ScePspFVector3 laser_offset;
	ScePspFVector3 tshell_offset;
	ScePspFVector3 missile_offset;
	ScePspFVector3 hit_ellipsoid_pos;
	ScePspFVector3 hit_ellipsoid_radii;

	int inverted;
	float deadzone;
} bg3_base;

bg3_base* bg3_create_base();

void bg3_destroy_base(bg3_base* base);

void bg3_base_load_resources(bg3_base* base);

void bg3_base_free_resources(bg3_base* base);

void bg3_base_init_effects(bg3_base* base);

void bg3_base_free_effects(bg3_base* base);

void bg3_base_load_game(bg3_base* base, int map_id);

void bg3_base_free_game(bg3_base* base);

void bg3_spawn_player(bg3_base* base, int player);

int bg3_damage_player(bg3_base* base, int player, float shields_dmg, float armor_dmg);

int bg3_damage_area(bg3_base* base, int player, ScePspFVector3* center, float inner_radius, float outer_radius, float shields_dmg, float armor_dmg);

void bg3_create_explosion(bg3_base* base, xVector3f* pos);

void bg3_ai_set_state(bg3_base* base, int player, int state);

bg3_tshells* bg3_create_tshells(int num);

void bg3_free_tshells(bg3_tshells* t);

void bg3_add_tshell(bg3_tshells* t, ScePspFVector3* p0, ScePspFVector3* p1);

void bg3_update_tshells(bg3_tshells* t, float dt);

void bg3_draw_tshells(bg3_tshells* t, ScePspFMatrix4* view);

bg3_bullets* bg3_create_bullets(int num);

void bg3_free_bullets(bg3_bullets* b);

void bg3_add_bullet(bg3_bullets* b, int player, ScePspFVector3* pos, ScePspFVector3* vel);

void bg3_update_bullets(bg3_bullets* b, bg3_base* base, float dt);

void bg3_draw_bullets(bg3_bullets* b, ScePspFMatrix4* view, float length, float h1, float h2, u32 c1, u32 c2);

bg3_missiles* bg3_create_missiles(int num);

void bg3_free_missiles(bg3_missiles* m);

void bg3_add_missile(bg3_missiles* m, int player, ScePspFVector3* pos, ScePspFVector3* dir, ScePspFVector3* target);

void bg3_update_missiles(bg3_missiles* m, bg3_base* base, float dt);

void bg3_draw_missiles(bg3_missiles* m, ScePspFMatrix4* view, float width);

bg3_powerups* bg3_create_powerups(int num);

void bg3_free_powerups(bg3_powerups* p);

void bg3_add_powerup(bg3_powerups* p, ScePspFVector3* pos, int type, int respawns);

void bg3_update_powerups(bg3_powerups* p, bg3_base* base, float dt);

void bg3_draw_powerups(bg3_powerups* p, bg3_base* base);

#endif