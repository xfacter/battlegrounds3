/**
 * This file belongs to the 'Battlegrounds 3' game project.
 * Copyright 2009 xfacter
 * Copyright 2016 wickles
 * This work is licensed under the GPLv3
 * subject to all terms as reproduced in the included LICENSE file.
 */

#include <pspgu.h>
#include <pspgum.h>
#include "xlab/xmath.h"
#include "xlab/xlab.h"
#include "xlab/xtime.h"
#include "xlab/xctrl.h"
#include "xlab/xgraphics.h"
#include "xlab/xsound.h"

#include "game.h"

#define LOD_DELAY 0.1f
//#define ENABLE_MOTIONBLUR
#define MOTION_BLUR_STEP (1.0f/60)
//#define DRAW_ELLIPSOIDS
#define DRAW_TANKS
#define DRAW_HEIGHTMAP
#define AI_ENABLED
#define ENABLE_CALCULATIONS
#define ENABLE_SOUND
#define LOD_DIST 10.0f
#define TANK_TURN_X 2.0f
#define TANK_TURN_Y 1.0f
#define MAX_ANGLE DEG_TO_RAD(60.0f)
#define FORCE_CONST 20.0f
//#define FORCE_CONST FORCE_CONST
#define AI_SIGHT_RANGE 40.0f
#define AI_ATTACK_RANGE 20.0f
#define AI_VIEW_ANGLE 90.0f
#define AI_VISION_MAX 50.0f
#define AI_FWD_DIST 2.5f
#define AI_TURN_DIST 10.0f
#define AI_QUEUE_TIME 1.0f
#define AI_UPDATE_TIME 0.5f
#define AI_ATTACK_UPDATE 2.0f
#define AI_PATH_TIMEOUT 5.0f
#define AI_SCAN_TIMEOUT 5.0f
#define AI_FLEE_TIMEOUT 5.0f
#define AI_HP_FLEE 0.5f
#define AI_ACCURACY_DEG 0.5f
#define AI_FIRE_TIME 1.0f
#define AI_FIRE_DELAY 4.0f
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define CAM_RAY_DIST 100.0f
#define RAND_ANGLE DEG_TO_RAD(1.0f)
#define TANK_HEIGHT 0.5f
#define CAM_DIST 2.5f
#define CAM_BACKOFF 0.5f
#define CAM_OFFSET 0.0f
#define CAM_HEIGHT 2.0f
#define CAM_DROPOFF DEG_TO_RAD(-15.0f)
#define TURRET_LOW_ANGLE DEG_TO_RAD(0.0f)
#define SHADOW_FOV 50.0f
#define SHADOW_DIST 3.0f
#define ROCKETS_COLOR 0xffffffff
#define TURRET2_COLOR 0xffdfdfdf
#define LASER_MARK_STEP 0.025f
#define LASER_DISTANCE_FIX 0.5f
#define LASER_MAX_DIST 50.0f
#define LASER_RADIUS 0.1f
#define LASER_STEP 0.25f
#define LASER_PHASE 50.0f
#define LASER_TRIG_STEP 2.5f
#define DUST_WAIT 0.05f
#define DUST_HEIGHT 2.0f
#define GAME_FADE_TIME 1.0f

typedef struct bg3_menu {
	int screen;
	int selection;
} bg3_menu;

int inverted;
float deadzone;
int control_style;

void game_save_settings(bg3_base* base)
{
	base->inverted = inverted;
	base->deadzone = deadzone;
	base->control_style = control_style;

	FILE* file = fopen("./config.ini", "w");
	if (file == NULL) return;
	fprintf(file, "[config]\r\ninverted = %s\r\ndeadzone = %.2f\r\nstyle = %s", (inverted == 0 ? "false" : "true"), deadzone, (control_style == 0 ? "ANALOG_LOOK" : "ANALOG_MOVE"));
	fclose(file);
}

/* menu
	- resume
	- options
		inverted: yes/no
		deadzone: 0.0 - 1.0
		controls: analog look / analog move
		Apply
	- quit
		sure? yes/no
*/

enum menu_states {
	MENU_MAIN,
	MENU_OPTIONS,
	MENU_EXIT
};

void bg3_game_menu_input(bg3_base* base, bg3_menu* menu)
{
	bg3_game* g = &base->game;
	if (xCtrlTap(PSP_CTRL_START))
	{
		goto unpause;
	}
	switch (menu->screen)
	{
	case MENU_MAIN:
		if (xCtrlTap(PSP_CTRL_CIRCLE))
		{
			goto unpause;
		}
		if (xCtrlTap(PSP_CTRL_UP))
		{
			if (menu->selection > 0)
			{
				menu->selection -= 1;
			}
		}
		if (xCtrlTap(PSP_CTRL_DOWN))
		{
			if (menu->selection < 2)
			{
				menu->selection += 1;
			}
		}
		switch (menu->selection)
		{
		case 0:
			if (xCtrlTap(PSP_CTRL_CROSS))
			{
				goto unpause;
			}
			break;
		case 1:
			if (xCtrlTap(PSP_CTRL_CROSS))
			{
				menu->screen = MENU_OPTIONS;
				menu->selection = 0;
				inverted = base->inverted;
				deadzone = base->deadzone;
				control_style = base->control_style;
			}
			break;
		case 2:
			if (xCtrlTap(PSP_CTRL_CROSS))
			{
				menu->screen = MENU_EXIT;
				menu->selection = 0;
			}
			break;
		}
		break;
	case MENU_OPTIONS:
		if (xCtrlTap(PSP_CTRL_CIRCLE))
		{
			menu->screen = MENU_MAIN;
			menu->selection = 1;
		}
		if (xCtrlTap(PSP_CTRL_UP))
		{
			if (menu->selection > 0)
			{
				menu->selection -= 1;
			}
		}
		if (xCtrlTap(PSP_CTRL_DOWN))
		{
			if (menu->selection < 3)
			{
				menu->selection += 1;
			}
		}
		switch (menu->selection)
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
				game_save_settings(base);
				menu->screen = MENU_MAIN;
				menu->selection = 0;
			}
			break;
		}
		break;
	case MENU_EXIT:
		if (xCtrlTap(PSP_CTRL_CIRCLE))
		{
			menu->screen = MENU_MAIN;
			menu->selection = 2;
		}
		if (xCtrlTap(PSP_CTRL_LEFT) || xCtrlTap(PSP_CTRL_RIGHT))
		{
			menu->selection = !menu->selection;
		}
		switch (menu->selection)
		{
		case 0:
			if (xCtrlTap(PSP_CTRL_CROSS))
			{
				menu->screen = MENU_MAIN;
				menu->selection = 2;
			}
			break;
		case 1:
			if (xCtrlTap(PSP_CTRL_CROSS))
			{
				base->transition = BG3_FADE_OUT;
				base->fade = 0.0f;
			}
			break;
		}
		break;
	}
	return;
unpause:
	g->paused = 0;
	xSoundSetStateAll(X_SOUND_PLAY, 0);
}

float dust_time = 0.0f;
float lod_time = LOD_DELAY;

void bg3_game_reset_players_frame(bg3_base* base)
{
	if (base == NULL) return;
	bg3_game* g = &base->game;
	bg3_player* players = g->players;
	int i;
	for (i = 0; i < g->num_players; i++)
	{
		players[i].acc.x = 0.0f;
		players[i].acc.y = 0.0f;
		players[i].acc.z = 0.0f;
		players[i].firing = 0;
	}
}

void bg3_ai_find_path(bg3_base* base, int player, int x, int y, int active);

void bg3_game_player_input(bg3_base* base, bg3_menu* menu, float dt)
{
	if (base == NULL) return;
	bg3_game* g = &base->game;
	bg3_player* p = &g->players[g->player];
	if (g->game_over)
	{
		if (base->transition != BG3_FADE_OUT)
		{
			if (xCtrlTap(PSP_CTRL_CROSS))
			{
				base->transition = BG3_FADE_OUT;
				base->fade = 0.0f;
			}
		}

	}
	else
	{
		if (p->hp_armor > 0.0f)
		{
			if (base->control_style == 0)
			{
				if (xCtrlAnalogAlive(base->deadzone))
				{
					x_rotatez(&p->cam_dir, -TANK_TURN_X*xCtrlAnalogX()*dt);
					//p->cam_dir.z += 5.0f*xCtrlAnalogY()*dt;
					p->cam_pitch += (base->inverted ? -1 : 1) * TANK_TURN_Y * -xCtrlAnalogY() * dt;
				}
				if (xCtrlPress(PSP_CTRL_TRIANGLE)) //fwd
				{
					p->acc.x += FORCE_CONST*p->tank_dir.x;
					p->acc.y += FORCE_CONST*p->tank_dir.y;
				}
				if (xCtrlPress(PSP_CTRL_CROSS)) //back
				{
					p->acc.x -= FORCE_CONST*p->tank_dir.x;
					p->acc.y -= FORCE_CONST*p->tank_dir.y;
				}
				if (xCtrlPress(PSP_CTRL_CIRCLE)) //right
				{
					p->acc.x += FORCE_CONST*p->tank_dir.y;
					p->acc.y += FORCE_CONST*-p->tank_dir.x;
				}
				if (xCtrlPress(PSP_CTRL_SQUARE)) //left
				{
					p->acc.x -= FORCE_CONST*p->tank_dir.y;
					p->acc.y -= FORCE_CONST*-p->tank_dir.x;
				}
			}
			else
			{
				if (xCtrlAnalogAlive(base->deadzone))
				{
					p->acc.x += xCtrlAnalogX()*FORCE_CONST*p->tank_dir.y;
					p->acc.y += xCtrlAnalogX()*FORCE_CONST*-p->tank_dir.x;

					p->acc.x += xCtrlAnalogY()*FORCE_CONST*p->tank_dir.x;
					p->acc.y += xCtrlAnalogY()*FORCE_CONST*p->tank_dir.y;
				}
				if (xCtrlPress(PSP_CTRL_TRIANGLE)) //fwd
				{
					p->cam_pitch += (base->inverted ? -1 : 1) * -TANK_TURN_Y * dt;
				}
				if (xCtrlPress(PSP_CTRL_CROSS)) //back
				{
					p->cam_pitch += (base->inverted ? -1 : 1) * TANK_TURN_Y * dt;
				}
				if (xCtrlPress(PSP_CTRL_CIRCLE)) //right
				{
					x_rotatez(&p->cam_dir, -TANK_TURN_X*dt);
				}
				if (xCtrlPress(PSP_CTRL_SQUARE)) //left
				{
					x_rotatez(&p->cam_dir, TANK_TURN_X*dt);
				}
			}
			if (xCtrlTap(PSP_CTRL_RIGHT) || xCtrlTap(PSP_CTRL_LTRIGGER))
			{
				do {
					p->primary += 1;
					if (p->primary >= BG3_NUM_PRIMARY)
						p->primary = 0;

				} while (!(p->weapons & (1<<p->primary)));
			}
			if (xCtrlTap(PSP_CTRL_LEFT))
			{
				do {
					p->primary -= 1;
					if (p->primary < 0)
						p->primary = BG3_NUM_PRIMARY;

				} while (!(p->weapons & (1<<p->primary)));
			}
			if (xCtrlPress(PSP_CTRL_RTRIGGER))
			{
				p->firing = 1;
			}
		}
		if (xCtrlTap(PSP_CTRL_START))
		{
			menu->screen = MENU_MAIN;
			menu->selection = 0;
			g->paused = 1;
			xSoundSetStateAll(X_SOUND_PAUSE, 0);
		}
	}

#ifdef X_DEBUG
	if (xCtrlTap(PSP_CTRL_UP))
	{
		g->player += 1;
		if (g->player >= g->num_players) g->player = 0;
	}
	if (xCtrlTap(PSP_CTRL_DOWN))
	{
		g->player -= 1;
		if (g->player < 0) g->player = g->num_players-1;
	}
#endif
}

/*
void bg3_ai_find_path(bg3_base* base, int player, ScePspFVector2* target, int active)
{
	//
}

void bg3_ai_get_path(bg3_base* base, int player, ScePspFVector2* out)
{
	//
}
*/

void bg3_ai_find_path(bg3_base* base, int player, int x, int y, int active)
{
	if (base == NULL) return;
	if (player < 0 || player >= base->game.num_players) return;
	//printf("looking for path...\n");
	bg3_game* g = &base->game;
	bg3_map* map = g->map;
	bg3_player* p = &g->players[player];
	int goal = y*map->width + x;
	x = (int)(p->pos.x/map->hmp.tile_scale);
	y = (int)(p->pos.y/map->hmp.tile_scale);
	int start = y*map->width + x;
	//start = bg3_map_to_astar(map, start);
	astar_find_path(p->ai.path, map->astar_map, start, goal);
	if (p->ai.path->length > 0)
	{
		p->ai.path_idx = 0;
		//printf("path found!\n");
		bg3_ai_set_state(base, player, (active ? BG3_AI_NEUTRAL_SEEK_ACTIVE : BG3_AI_NEUTRAL_SEEK_PASSIVE));
	}
}

void bg3_game_process_ai(bg3_base* base, float dt)
{
	if (base == NULL) return;
	bg3_game* g = &base->game;
	bg3_map* map = g->map;
	bg3_player* players = g->players;
	float view_cos = x_cosf(DEG_TO_RAD(0.5f*AI_VIEW_ANGLE));
	int update = 0;
	int refresh = 0;
	g->ai_update_time += dt;
	while (g->ai_update_time >= AI_UPDATE_TIME)
	{
		update += 1;
		refresh = 1;
		g->ai_update_time -= AI_UPDATE_TIME;
	}
	int i, j;
	for (i = 0; i < g->num_players; i++)
	{
		if (i != g->player && players[i].hp_armor > 0.0f)
		{
			players[i].ai.update += dt;
			if (players[i].ai.state < BG3_AI_NEUTRAL_STATES)
			{
				int state_changed = 0;
				//neutral
				if (refresh && players[i].ai.state != BG3_AI_NEUTRAL_SEEK_ACTIVE)
				{
					for (j = 0; j < g->num_players; j++)
					{
						if (j != i && players[j].hp_armor > 0.0f)
						{
							xVector3f dir;
							xVec3Sub(&dir, &players[j].e_pos, (xVector3f*)&players[i].cam_orig);
							float dist_sq = xVec3SqLength(&dir);
							if (dist_sq <= SQR(AI_SIGHT_RANGE))
							{
								//within range
								float dist = x_sqrtf(dist_sq);
								if (xVec3Dot(&dir, (xVector3f*)&players[i].cam_dir)/dist >= view_cos)
								{
									//within view angle
									if (bg3_ray_heightmap_collision(&map->hmp, 0, &players[i].cam_orig, (ScePspFVector3*)&dir, 1.0f, X_EPSILON) >= 1.0f)
									{
										if (dist <= AI_ATTACK_RANGE)
										{
											//enemy visible, set hostile state
											bg3_ai_set_state(base, i, BG3_AI_HOSTILE_ATTACK);
											players[i].ai.target = j;
											state_changed = 1;
										}
										else
										{
											int x = (int)(players[j].pos.x/map->hmp.tile_scale);
											int y = (int)(players[j].pos.y/map->hmp.tile_scale);
											bg3_ai_find_path(base, i, x, y, 0);
											update -= 1;
											state_changed = 1;
										}
										break;
									}
								}
							}
						}
					}
					if (!state_changed)
					{
						for (j = 0; j < g->powerups->num; j++)
						{
							powerup* p = &g->powerups->powerups[g->powerups->stack[j]];
							if (p->respawn != 2)
							{
								//not waiting to respawn
								xVector3f dir;
								xVec3Sub(&dir, (xVector3f*)&p->pos, (xVector3f*)&players[i].cam_orig);
								float dist_sq = xVec3SqLength(&dir);
								if (dist_sq <= SQR(AI_SIGHT_RANGE))
								{
									//within range
									float dist = x_sqrtf(dist_sq);
									if (xVec3Dot(&dir, (xVector3f*)&players[i].cam_dir)/dist >= view_cos)
									{
										//within view angle
										if (bg3_ray_heightmap_collision(&map->hmp, 0, &players[i].cam_orig, (ScePspFVector3*)&dir, 1.0f, X_EPSILON) >= 1.0f)
										{
											//powerup is in the ai's view, find a path to it
											int x = (int)(p->pos.x/map->hmp.tile_scale);
											int y = (int)(p->pos.y/map->hmp.tile_scale);
											bg3_ai_find_path(base, i, x, y, 0);
											update -= 1;
											state_changed = 1;
											break;
										}
									}
								}
							}
						}
					}
				}
				if (!state_changed)
				{
					if (players[i].ai.state == BG3_AI_NEUTRAL_WANDER)
					{
						//vision-based movement
						xVector3f fwd, left, right;
						xVec3Normalize(&fwd, (xVector3f*)&players[i].tank_dir);
						fwd.z = x_tanf(X_PI/12);
						left = fwd;
						x_rotatez((ScePspFVector3*)&left, X_PI/6);
						right = fwd;
						x_rotatez((ScePspFVector3*)&right, -X_PI/6);

						float t_fwd = bg3_ray_heightmap_collision(&map->hmp, 0, &players[i].cam_orig, (ScePspFVector3*)&fwd, AI_VISION_MAX, X_EPSILON);
						float t_left = bg3_ray_heightmap_collision(&map->hmp, 0, &players[i].cam_orig, (ScePspFVector3*)&left, AI_VISION_MAX, X_EPSILON);
						float t_right = bg3_ray_heightmap_collision(&map->hmp, 0, &players[i].cam_orig, (ScePspFVector3*)&right, AI_VISION_MAX, X_EPSILON);
						if (t_right <= AI_TURN_DIST && t_left > t_right)
						{
							//turn left
							x_rotatez(&players[i].cam_dir, TANK_TURN_X*dt);
						}
						if (t_left <= AI_TURN_DIST && t_right > t_left)
						{
							//turn right
							x_rotatez(&players[i].cam_dir, -TANK_TURN_X*dt);
						}
						if (t_fwd >= AI_FWD_DIST)
						{
							//move fwd
							players[i].acc.x += FORCE_CONST*players[i].tank_dir.x;
							players[i].acc.y += FORCE_CONST*players[i].tank_dir.y;
						}
						if (update > 0)
						{
							//try to find a path...
							ScePspIVector2 goal;
							bg3_map_random_poi(map, &goal);
							bg3_ai_find_path(base, i, goal.x, goal.y, 0);
							update -= 1;
						}
					}
					else if (players[i].ai.state == BG3_AI_NEUTRAL_SCAN)
					{
						//scan surroundings
						if (players[i].ai.rand <= 5)
						{
							//rotate left
							x_rotatez(&players[i].cam_dir, TANK_TURN_X*dt);
						}
						else
						{
							//rotate right
							x_rotatez(&players[i].cam_dir, -TANK_TURN_X*dt);
						}
						if (players[i].ai.rand == 2)
						{
							//move fwd
							players[i].acc.x += FORCE_CONST*players[i].tank_dir.x;
							players[i].acc.y += FORCE_CONST*players[i].tank_dir.y;
						}
						else if (players[i].ai.rand == 4)
						{
							//move back
							players[i].acc.x -= FORCE_CONST*players[i].tank_dir.x;
							players[i].acc.y -= FORCE_CONST*players[i].tank_dir.y;
						}
						else if (players[i].ai.rand == 6)
						{
							//move right
							players[i].acc.x += FORCE_CONST*players[i].tank_dir.y;
							players[i].acc.y += FORCE_CONST*-players[i].tank_dir.x;
						}
						else if (players[i].ai.rand == 8)
						{
							//move left
							players[i].acc.x -= FORCE_CONST*players[i].tank_dir.y;
							players[i].acc.y -= FORCE_CONST*-players[i].tank_dir.x;
						}
						if (players[i].ai.update >= AI_SCAN_TIMEOUT)
						{
							//didnt find anything.. go home!
							bg3_ai_set_state(base, i, BG3_AI_NEUTRAL_WANDER);
						}
					}
					else
					{
						//follow path
						if (players[i].ai.path_idx >= 0)
						{
							int current = (int)(players[i].pos.y/map->hmp.tile_scale)*map->width + (int)(players[i].pos.x/map->hmp.tile_scale);
							int target = astar_get_path(players[i].ai.path, players[i].ai.path_idx);
							/*
							target = bg3_astar_to_map(map, target);
							if (bg3_map_to_astar(map, current) == bg3_map_to_astar(map, target))
							*/
							//if player is close enough to target, update path
							if (current == target)
							{
								//printf("path_idx: %i\n", players[i].ai.path_idx);
								//advance to next position in path
								players[i].ai.path_idx += 1;
								players[i].ai.update = 0.0f;
								if (players[i].ai.path_idx >= players[i].ai.path->length)
								{
									bg3_ai_set_state(base, i, BG3_AI_NEUTRAL_WANDER);
								}
							}
							else
							{
								if (players[i].ai.update >= AI_PATH_TIMEOUT)
								{
									//taking too long to reach target, dump the path
									bg3_ai_set_state(base, i, BG3_AI_NEUTRAL_WANDER);
								}
								else
								{
									//move towards target
									float x = (target % map->width)*map->hmp.tile_scale + 0.5f*map->hmp.tile_scale;
									float y = (target / map->width)*map->hmp.tile_scale + 0.5f*map->hmp.tile_scale;
									//printf("path: x: %f, y: %f\n", x, y);
									xVector3f fwd = {x - players[i].pos.x, y - players[i].pos.y, 0.0f};
									xVec3Normalize(&fwd, &fwd);
									//rotate towards target
									float theta = x_acosf(xVec3Dot((xVector3f*)&players[i].cam_dir, &fwd));
									xVector3f left = {-players[i].cam_dir.y, players[i].cam_dir.x, 0.0f};
									if (xVec3Dot(&fwd, &left) < 0.0f)
									{
										//turn right
										if (theta >= DEG_TO_RAD(5.0f))
										{
											x_rotatez(&players[i].cam_dir, -TANK_TURN_X*dt);
											if (theta < DEG_TO_RAD(60.0f))
											{
												//strafe right
												players[i].acc.x += FORCE_CONST*players[i].tank_dir.y;
												players[i].acc.y += FORCE_CONST*-players[i].tank_dir.x;
											}
										}
									}
									else
									{
										//turn left
										if (theta >= DEG_TO_RAD(5.0f))
										{
											x_rotatez(&players[i].cam_dir, TANK_TURN_X*dt);
											if (theta < DEG_TO_RAD(60.0f))
											{
												//strafe left
												players[i].acc.x -= FORCE_CONST*players[i].tank_dir.y;
												players[i].acc.y -= FORCE_CONST*-players[i].tank_dir.x;
											}

										}
									}
									if (theta < DEG_TO_RAD(5.0f))
									{
										//move fwd
										players[i].acc.x += FORCE_CONST*players[i].tank_dir.x;
										players[i].acc.y += FORCE_CONST*players[i].tank_dir.y;
									}
								}
							}
						}
						else
						{
							bg3_ai_set_state(base, i, BG3_AI_NEUTRAL_WANDER);
						}
					}
				}
				if (players[i].cam_pitch > DEG_TO_RAD(AI_ACCURACY_DEG))
				{
					//back to level
					players[i].cam_pitch -= TANK_TURN_Y*dt;
				}
				else if (players[i].cam_pitch < DEG_TO_RAD(-AI_ACCURACY_DEG))
				{
					//back to level
					players[i].cam_pitch += TANK_TURN_Y*dt;
				}
			}
			else if (players[i].ai.state < BG3_AI_HOSTILE_STATES)
			{
				//hostile
				int target = players[i].ai.target;
				if (players[target].hp_armor <= 0.0f)
				{
					//enemy is dead
					bg3_ai_set_state(base, i, BG3_AI_NEUTRAL_WANDER);
				}
				else
				{
					if (players[i].ai.state == BG3_AI_HOSTILE_FLEE)
					{
						int danger = 0;
						xVector3f dir;
						xVec3Sub(&dir, &players[target].e_pos, (xVector3f*)&players[i].cam_orig);
						dir.z = 0.0f;
						float dist_sq = xVec3SqLength(&dir);
						if (dist_sq <= SQR(AI_SIGHT_RANGE))
						{
							//within range
							if (bg3_ray_heightmap_collision(&map->hmp, 0, &players[i].cam_orig, (ScePspFVector3*)&dir, 1.0f, X_EPSILON) >= 1.0f)
							{
								float dist = x_sqrtf(dist_sq);
								if (xVec3Dot(&dir, (xVector3f*)&players[i].cam_dir)/dist >= 0.0f)
								{
									//within view angle
									//rotate away from enemy
									xVec3Scale(&dir, &dir, 1.0f/dist);
									float theta = x_acosf(xVec3Dot((xVector3f*)&players[i].cam_dir, &dir));
									xVector3f left = {-players[i].cam_dir.y, players[i].cam_dir.x, 0.0f};
									//turning away, opposite of the normal test
									if (xVec3Dot(&dir, &left) > 0.0f)
									{
										//turn right (away)
										if (theta < DEG_TO_RAD(180.0f-AI_ACCURACY_DEG))
										{
											x_rotatez(&players[i].cam_dir, -TANK_TURN_X*dt);
										}
									}
									else
									{
										//turn left (away)
										if (theta < DEG_TO_RAD(180.0f-AI_ACCURACY_DEG))
										{
											x_rotatez(&players[i].cam_dir, TANK_TURN_X*dt);
										}
									}
									//if pointing away from enemy, move fwd
									if (theta >= DEG_TO_RAD(90.0f))
									{
										//move fwd
										players[i].acc.x += FORCE_CONST*players[i].tank_dir.x;
										players[i].acc.y += FORCE_CONST*players[i].tank_dir.y;
									}
									danger = 1;
								}
							}
						}
						if (!danger || players[i].ai.update >= AI_FLEE_TIMEOUT)
						{
							bg3_ai_set_state(base, i, BG3_AI_NEUTRAL_WANDER);
						}
					}
					else
					{
						if (players[i].hp_shields <= AI_HP_FLEE && players[i].hp_armor <= AI_HP_FLEE)
						{
							bg3_ai_set_state(base, i, BG3_AI_HOSTILE_FLEE);
						}
						else
						{
							int visible = 0;
							xVector3f dir;
							xVec3Sub(&dir, &players[target].e_pos, (xVector3f*)&players[i].cam_orig);
							float dist_sq = xVec3SqLength(&dir);
							if (dist_sq <= SQR(AI_SIGHT_RANGE))
							{
								//within range
								float dist = x_sqrtf(dist_sq);
								if (xVec3Dot(&dir, (xVector3f*)&players[i].cam_dir)/dist >= view_cos)
								{
									//within view angle
									if (bg3_ray_heightmap_collision(&map->hmp, 0, &players[i].cam_orig, (ScePspFVector3*)&dir, 1.0f, X_EPSILON) >= 1.0f)
									{
										//enemy visible, attack
										if (x_modf(players[i].ai.update, 4.0f) <= 2.0f)
										{
											players[i].ai.fire += dt;
											if (players[i].ai.fire < AI_FIRE_TIME)
											{
												players[i].firing = 1;
											}
											if (players[i].ai.fire >= AI_FIRE_DELAY)
											{
												players[i].ai.fire = 0.0f;
											}
											if (players[target].hp_shields > 0.0f)
											{
												if (players[i].weapons & (1<<BG3_LASER))
													players[i].primary = BG3_LASER;
												else
													players[i].primary = BG3_MACHINE_GUN;
											}
											else
											{
												if (players[i].weapons & (1<<BG3_MISSILES))
													players[i].primary = BG3_MISSILES;
												else if (players[i].weapons & (1<<BG3_TANK_SHELL))
													players[i].primary = BG3_TANK_SHELL;
												else
													players[i].primary = BG3_MACHINE_GUN;
											}
										}
										visible = 1;
										xVector3f fwd = dir;
										fwd.z = 0.0f;
										xVec3Normalize(&fwd, &fwd);
										//rotate towards target
										float theta = x_acosf(xVec3Dot((xVector3f*)&players[i].cam_dir, &fwd));
										xVector3f left = {-players[i].cam_dir.y, players[i].cam_dir.x, 0.0f};
										if (xVec3Dot(&fwd, &left) < 0.0f)
										{
											//turn right
											if (theta > DEG_TO_RAD(AI_ACCURACY_DEG))
											{
												x_rotatez(&players[i].cam_dir, -TANK_TURN_X*dt);
											}
										}
										else
										{
											//turn left
											if (theta > DEG_TO_RAD(AI_ACCURACY_DEG))
											{
												x_rotatez(&players[i].cam_dir, TANK_TURN_X*dt);
											}
										}

										xVec3Scale(&dir, &dir, 1.0f/dist);
										float phi = x_asinf(dir.z) - -players[i].cam_pitch;
										if (phi > DEG_TO_RAD(AI_ACCURACY_DEG))
										{
											//rotate up
											players[i].cam_pitch += -TANK_TURN_Y*dt;
										}
										else if (phi < DEG_TO_RAD(-AI_ACCURACY_DEG))
										{
											//rotate down
											players[i].cam_pitch -= -TANK_TURN_Y*dt;
										}

										if (players[i].ai.rand == 2)
										{
											//move fwd
											players[i].acc.x += FORCE_CONST*players[i].tank_dir.x;
											players[i].acc.y += FORCE_CONST*players[i].tank_dir.y;
										}
										else if (players[i].ai.rand == 4)
										{
											//move back
											players[i].acc.x -= FORCE_CONST*players[i].tank_dir.x;
											players[i].acc.y -= FORCE_CONST*players[i].tank_dir.y;
										}
										else if (players[i].ai.rand == 6)
										{
											//move right
											players[i].acc.x += FORCE_CONST*players[i].tank_dir.y;
											players[i].acc.y += FORCE_CONST*-players[i].tank_dir.x;
										}
										else if (players[i].ai.rand == 8)
										{
											//move left
											players[i].acc.x -= FORCE_CONST*players[i].tank_dir.y;
											players[i].acc.y -= FORCE_CONST*-players[i].tank_dir.x;
										}
									}
								}
							}

							if (players[i].ai.update >= AI_ATTACK_UPDATE)
							{
								players[i].ai.update = 0.0f;
								players[i].ai.rand = x_randi(1, 10);
							}

							if (!visible)
							{
								//find a path to the targets last known position
								int x = (int)(players[target].pos.x/map->hmp.tile_scale);
								int y = (int)(players[target].pos.y/map->hmp.tile_scale);
								bg3_ai_find_path(base, i, x, y, 0);
								update -= 1;
							}
						}

					}
				}
			}
			else
			{
				//unknown state, set to wander
				bg3_ai_set_state(base, i, BG3_AI_NEUTRAL_WANDER);
			}
		}
	}
}

void bg3_game_update_players(bg3_base* base, float dt)
{
	if (base == NULL) return;
	bg3_game* g = &base->game;
	bg3_map* map = g->map;
	bg3_player* players = g->players;
	bg3_resources* resources = &base->resources;
	bg3_effects* effects = &base->effects;
	int i, j;
	for (i = 0; i < g->num_players; i++)
	{
		//if player is alive...
		if (players[i].hp_armor > 0.0f)
		{
			if (players[i].hp_shields < 1.0f)
			{
				if (players[i].shield_wait > 0.0f)
				{
					//shield is waiting to recharge
					players[i].shield_wait -= dt;
				}
				else
				{
					//shield is recharging
					players[i].hp_shields += SHIELD_RECHARGE_PER_SEC*dt;
					if (players[i].hp_shields > 1.0f)
						players[i].hp_shields = 1.0f;
					xParticleEmitter e;
					e.particle_pos = players[i].e_pos;
					e.new_velocity = 0;
					xParticleSystemBurst(effects->recharge_ps, &e, 1);
				}
			}

			if (players[i].hp_armor <= DAMAGE_POINT1)
			{
				xParticleEmitter e;
				e.particle_pos = players[i].e_pos;
				e.particle_vel = *(xVector3f*)&players[i].vel;
				e.particle_vel.z = 2.0f;
				e.new_velocity = 1;

				players[i].smoke_time += dt;
				while (players[i].smoke_time >= DMG_SMOKE_WAIT)
				{
					players[i].smoke_time -= DMG_SMOKE_WAIT;
					xParticleSystemBurst(effects->smoke_ps, &e, 1);
				}
				players[i].spark_time += dt;
				while (players[i].spark_time >= DMG_SPARKS_WAIT)
				{
					players[i].spark_time -= DMG_SPARKS_WAIT;
					xParticleSystemBurst(effects->sparks_ps, &e, 5);
				}
			}

			if (players[i].powerup_time > 0.0f)
			{
				players[i].powerup_time -= dt;
				xParticleEmitter e;
				e.particle_pos = players[i].e_pos;
				e.new_velocity = 0;
				xParticleSystemBurst(effects->powerup_pickup_ps, &e, 1);
			}

			ScePspFVector3 normal;
			for (j = 0; j < 4; j++)
			{
				if (j == 0)
					xHeightmapGetNormal(&map->hmp, &normal, players[i].pos.x + base->hit_ellipsoid_radii.x*players[i].base_mat.x.x + base->hit_ellipsoid_radii.y*players[i].base_mat.y.x,
					players[i].pos.y + base->hit_ellipsoid_radii.x*players[i].base_mat.x.y + base->hit_ellipsoid_radii.y*players[i].base_mat.y.y);
				else if (j == 1)
					xHeightmapGetNormal(&map->hmp, &normal, players[i].pos.x + base->hit_ellipsoid_radii.x*players[i].base_mat.x.x - base->hit_ellipsoid_radii.y*players[i].base_mat.y.x,
					players[i].pos.y + base->hit_ellipsoid_radii.x*players[i].base_mat.x.y - base->hit_ellipsoid_radii.y*players[i].base_mat.y.y);
				else if (j == 2)
					xHeightmapGetNormal(&map->hmp, &normal, players[i].pos.x - base->hit_ellipsoid_radii.x*players[i].base_mat.x.x + base->hit_ellipsoid_radii.y*players[i].base_mat.y.x,
					players[i].pos.y - base->hit_ellipsoid_radii.x*players[i].base_mat.x.y + base->hit_ellipsoid_radii.y*players[i].base_mat.y.y);
				else //if (j == 3)
					xHeightmapGetNormal(&map->hmp, &normal, players[i].pos.x - base->hit_ellipsoid_radii.x*players[i].base_mat.x.x - base->hit_ellipsoid_radii.y*players[i].base_mat.y.x,
					players[i].pos.y - base->hit_ellipsoid_radii.x*players[i].base_mat.x.y - base->hit_ellipsoid_radii.y*players[i].base_mat.y.y);
				if (normal.z < 0.6f)
				{
					players[i].acc.x += 50.0f*normal.x;
					players[i].acc.y += 50.0f*normal.y;
					break;
				}
			}

			//friction
			/*
			if (x_absf(players[i].vel.x) >= 0.1f || x_absf(players[i].vel.y) >= 0.1f)
			{
			ScePspFVector3 inv_vel = {players[i].vel.x, players[i].vel.y, 0.0f};
			x_normalize(&inv_vel, -1.0f);
			players[i].acc.x += 10.0f*inv_vel.x;
			players[i].acc.y += 10.0f*inv_vel.y;
			}
			*/
			//resistive force, limits velocity
			players[i].acc.x -= 3.0f*players[i].vel.x;//*x_absf(players[i].vel.x);
			players[i].acc.y -= 3.0f*players[i].vel.y;//*x_absf(players[i].vel.y);
			//players[i].acc.z -= 9.8f;
			//integration
			players[i].pos.x += players[i].vel.x*dt + 0.5f*players[i].acc.x*dt*dt;
			players[i].pos.y += players[i].vel.y*dt + 0.5f*players[i].acc.y*dt*dt;
			players[i].vel.x += players[i].acc.x*dt;
			players[i].vel.y += players[i].acc.y*dt;

			if (players[i].pos.x < 0.0f) players[i].pos.x = 0.0f;
			if (players[i].pos.x >= map->hmp.tile_scale*(map->hmp.width-1)) players[i].pos.x = map->hmp.tile_scale*(map->hmp.width-1) - 0.0001f;
			if (players[i].pos.y < 0.0f) players[i].pos.y = 0.0f;
			if (players[i].pos.y >= map->hmp.tile_scale*(map->hmp.height-1)) players[i].pos.y = map->hmp.tile_scale*(map->hmp.height-1) - 0.0001f;

			float new_height = xHeightmapGetHeight(&map->hmp, &normal, players[i].pos.x, players[i].pos.y);
			if (new_height >= players[i].pos.z)
			{
				players[i].pos.z = new_height;
				//players[i].vel.z = 0.0f;
			}
			else
			{
				players[i].pos.z = x_ease_to_target(players[i].pos.z, new_height, 2.0f, dt);
				/*
				players[i].acc.z -= 0.1f/(new_height-players[i].pos.z);
				players[i].pos.z += players[i].vel.z*dt + 0.5f*players[i].acc.z*dt*dt;
				players[i].vel.z += players[i].acc.z*dt;
				*/
			}
			players[i].height = x_absf(new_height - players[i].pos.z);

			for (j = 0; j < g->num_players; j++)
			{
				if (j != i && players[j].hp_armor > 0.0f)
				{
					//if (x_absf(players[i].pos.z - players[j].pos.z) < 2*TANK_HEIGHT)
					xVector3f dir;
					xVec3Sub(&dir, (xVector3f*)&players[i].pos, (xVector3f*)&players[j].pos);
					//dir.z = 0.0f;
					if (xVec3SqLength(&dir) < SQR(2*TANK_RADIUS))
					{
						xVec3Normalize(&dir, &dir);
						xVec3Scale(&dir, &dir, 2*TANK_RADIUS);
						xVec3Add((xVector3f*)&players[i].pos, (xVector3f*)&players[j].pos, &dir);
					}
				}
			}

			xHeightmapGetNormal(&map->hmp, &normal, players[i].pos.x, players[i].pos.y);

			//if normal is not too slanted, ease up vector to it
			if (normal.z > 0.5f)// && players[i].vel.z > -1.0f)
			{
				x_ease_to_target3((ScePspFVector3*)&players[i].base_mat.z, &normal, 5.0f, dt);
				x_normalize((ScePspFVector3*)&players[i].base_mat.z, 1.0f);
			}
			x_ease_to_target3(&players[i].tank_dir, &players[i].cam_dir, 2.5f, dt);
			x_normalize(&players[i].tank_dir, 1.0f);
			x_ease_to_target3(&players[i].cam_pos, &players[i].pos, 10.0f, dt);

			if (players[i].cam_pitch >= MAX_ANGLE)
				players[i].cam_pitch = MAX_ANGLE;
			else if (players[i].cam_pitch <= -MAX_ANGLE)
				players[i].cam_pitch = -MAX_ANGLE;

			{
				float s, c, s1, c1;
				x_sincos(players[i].cam_pitch, &s, &c);
				x_sincos(CAM_DROPOFF, &s1, &c1);
				ScePspFVector3 center = {players[i].cam_pos.x - CAM_OFFSET*players[i].cam_dir.x, players[i].cam_pos.y - CAM_OFFSET*players[i].cam_dir.y, players[i].cam_pos.z + CAM_HEIGHT};
				ScePspFVector3 dir;
				if (players[i].cam_pitch < CAM_DROPOFF)
				{
					dir.x = -players[i].cam_dir.x*c1;
					dir.y = -players[i].cam_dir.y*c1;
					dir.z = s1;
					players[i].cam_look.x = players[i].cam_dir.x*c;
					players[i].cam_look.y = players[i].cam_dir.y*c;
					players[i].cam_look.z = -s;
				}
				else
				{
					dir.x = -players[i].cam_dir.x*c;
					dir.y = -players[i].cam_dir.y*c;
					dir.z = s;
					players[i].cam_look.x = -dir.x;
					players[i].cam_look.y = -dir.y;
					players[i].cam_look.z = -dir.z;
				}
				bg3_ray_heightmap_collision(&map->hmp, &players[i].cam_orig, &center, &dir, CAM_DIST, CAM_BACKOFF);
			}
			players[i].base_mat.y.x = players[i].tank_dir.x;
			players[i].base_mat.y.y = players[i].tank_dir.y;
			players[i].base_mat.y.z = 0.0f;
			x_crossproduct((ScePspFVector3*)&players[i].base_mat.x, (ScePspFVector3*)&players[i].base_mat.y, (ScePspFVector3*)&players[i].base_mat.z);
			x_normalize((ScePspFVector3*)&players[i].base_mat.x, 1.0f);
			x_crossproduct((ScePspFVector3*)&players[i].base_mat.y, (ScePspFVector3*)&players[i].base_mat.z, (ScePspFVector3*)&players[i].base_mat.x);
			players[i].base_mat.w.x = players[i].pos.x;
			players[i].base_mat.w.y = players[i].pos.y;
			players[i].base_mat.w.z = players[i].pos.z + base->hover_height;
			players[i].top_mat = players[i].base_mat;
			players[i].top_mat.y.x = players[i].cam_dir.x;
			players[i].top_mat.y.y = players[i].cam_dir.y;
			players[i].top_mat.y.z = 0.0f;
			x_crossproduct((ScePspFVector3*)&players[i].top_mat.x, (ScePspFVector3*)&players[i].top_mat.y, (ScePspFVector3*)&players[i].top_mat.z);
			x_normalize((ScePspFVector3*)&players[i].top_mat.x, 1.0f);
			x_crossproduct((ScePspFVector3*)&players[i].top_mat.y, (ScePspFVector3*)&players[i].top_mat.z, (ScePspFVector3*)&players[i].top_mat.x);
			/*
			players[i].turret = players[i].top;
			ScePspFVector3 trans = {0.0f, 0.0f, turretheight};
			gumTranslate(&players[i].turret, &trans);
			*/

	#ifdef ENABLE_CALCULATIONS
			float look_angle = x_asinf(players[i].base_mat.y.z);
			/*
			if (-players[i].cam_pitch > look_angle + TURRET_LOW_ANGLE)
			{
			gumRotateX(&players[i].turret, -players[i].cam_pitch - look_angle);
			}
			else
			{
			gumRotateX(&players[i].turret, TURRET_LOW_ANGLE);
			}
			*(ScePspFVector3*)&players[i].turret.y = players[i].cam_look;
			x_crossproduct((ScePspFVector3*)&players[i].turret.x, (ScePspFVector3*)&players[i].turret.y, (ScePspFVector3*)&players[i].turret.z);
			x_normalize((ScePspFVector3*)&players[i].turret.x, 1.0f);
			x_crossproduct((ScePspFVector3*)&players[i].turret.y, (ScePspFVector3*)&players[i].turret.z, (ScePspFVector3*)&players[i].turret.x);
			*/
			if (-players[i].cam_pitch > look_angle)
			{
				//gumRotateX(&players[i].turret, -players[i].cam_pitch - look_angle);
				players[i].turret_pitch = -players[i].cam_pitch - look_angle;
			}
			else
			{
				players[i].turret_pitch = 0.0f;
			}
			//gumRotateY(&players[i].turret, players[i].turret_rot);

			/*
			xVec3Set(&players[i].e_pos,
			players[i].base_mat.w.x + e_pos.x*players[i].base_mat.x.x + e_pos.y*players[i].base_mat.y.x + e_pos.z*players[i].base_mat.z.x,
			players[i].base_mat.w.y + e_pos.x*players[i].base_mat.x.y + e_pos.y*players[i].base_mat.y.y + e_pos.z*players[i].base_mat.z.y,
			players[i].base_mat.w.z + e_pos.x*players[i].base_mat.x.z + e_pos.y*players[i].base_mat.y.z + e_pos.z*players[i].base_mat.z.z);
			*/
			ScePspFMatrix4 mat = players[i].base_mat;
			gumTranslate(&mat, &base->hit_ellipsoid_pos);
			players[i].e_pos = *(xVector3f*)&mat.w;
			xVector3f radii = *(xVector3f*)&base->hit_ellipsoid_radii;
			if (players[i].hp_shields > 0.0f)
				xVec3Scale(&radii, &radii, SHIELD_SCALE);
			bg3_get_ellipsoid_inverse_matrix(&players[i].e_mat, &players[i].base_mat, &radii);

			if (players[i].shield_fade > 0.0f)
				players[i].shield_fade -= dt;

			/*
			dirt_ps->pos = *(xVector3f*)&players[i].pos;
			xVec3Set(&dirt_ps->pos_rand, 0.3f, 0.5f, 0.1f);
			xVec3Set(&dirt_ps->vel, 0.0f, 0.0f, 0.0f);
			xVec3Set(&dirt_ps->vel_rand, 1.0f, 1.0f, 0.0f);
			dirt_ps->life = 0.5f;
			xParticleSystemBurst(dirt_ps, 2);
			*/

			if (players[i].height <= DUST_HEIGHT)
			{
				float t = dust_time;
				while (t > 0.0f)
				{
					xParticleEmitter e;
					e.particle_pos = *(xVector3f*)&players[i].pos;
					e.new_velocity = 0;
					xParticleSystemBurst(effects->dust_ps, &e, 1);
					t -= DUST_WAIT;
				}
			}
	#endif

			if (players[i].mgun_wait > 0.0f)
				players[i].mgun_wait -= dt;
			if (players[i].tshell_wait > 0.0f)
				players[i].tshell_wait -= dt;
			/*
			if (players[i].gauss_wait > 0.0f)
				players[i].gauss_wait -= dt;
			*/
			if (players[i].missile_wait > 0.0f)
				players[i].missile_wait -= dt;

			if (players[i].laser_ammo > 0.0f)
				players[i].weapons |= 1 << BG3_LASER;
			else
				players[i].weapons &= ~(1 << BG3_LASER);
			if (players[i].tshell_ammo > 0)
				players[i].weapons |= 1 << BG3_TANK_SHELL;
			else
				players[i].weapons &= ~(1 << BG3_TANK_SHELL);
			if (players[i].missile_ammo > 0)
				players[i].weapons |= 1 << BG3_MISSILES;
			else
				players[i].weapons &= ~(1 << BG3_MISSILES);

			if (players[i].primary != BG3_MACHINE_GUN && !(players[i].weapons & (1 << players[i].primary)))
				players[i].primary = BG3_MACHINE_GUN;

			if (players[i].firing)
			{
				switch (players[i].primary)
				{
				case BG3_MACHINE_GUN:
					if (players[i].mgun_wait <= 0.0f)
					{
						players[i].mgun_wait = MGUN_WAIT;
						ScePspFVector3 shootat;
						bg3_ray_heightmap_collision(&map->hmp, &shootat, &players[i].cam_orig, &players[i].cam_look, CAM_RAY_DIST, 0.0f);
						ScePspFMatrix4 mat = players[i].top_mat;
						gumTranslate(&mat, &resources->tank_turret->pos);
						gumRotateX(&mat, players[i].turret_pitch);
						gumTranslate(&mat, &base->mgun_offset);
						xVector3f vel;
						xVec3Sub(&vel, (xVector3f*)&shootat, (xVector3f*)&mat.w);
						xVec3Normalize(&vel, &vel);
						xVec3Scale(&vel, &vel, 100.0f);

						ScePspFVector3 rand_dir = *(ScePspFVector3*)&vel;
						x_rotatez(&rand_dir, x_randf(-RAND_ANGLE, RAND_ANGLE));
						x_rotatex(&rand_dir, x_randf(-RAND_ANGLE, RAND_ANGLE));
						bg3_add_bullet(g->bullets, base, i, (ScePspFVector3*)&mat.w, &rand_dir);
						g->scores[i].shots_fired += 1.0f;
						xParticleEmitter e;
						e.particle_pos = *(xVector3f*)&mat.w;
						e.new_velocity = 0;
						xParticleSystemBurst(effects->muzzleflash_ps, &e, 1);
						xParticleSystemBurst(effects->gun_smoke_ps, &e, 1);
						//play sound
						//xWavPlay(bullet_wav);
					}
					break;
				case BG3_LASER:
					{
						ScePspFVector3 shootat, dir, hit;
						float t_cam = bg3_ray_heightmap_collision(&map->hmp, &shootat, &players[i].cam_orig, &players[i].cam_look, CAM_RAY_DIST, 0.0f);
						ScePspFMatrix4 mat = players[i].top_mat;
						//gumRotateX(&mat, players[i].turret_pitch);
						gumTranslate(&mat, &base->laser_offset);
						dir.x = shootat.x - mat.w.x;
						dir.y = shootat.y - mat.w.y;
						dir.z = shootat.z - mat.w.z;
						float t0 = bg3_ray_heightmap_collision(&map->hmp, &hit, (ScePspFVector3*)&mat.w, &dir, 1.0f, X_EPSILON);
						int enemy = -1;
						for (j = 0; j < g->num_players; j++)
						{
							if (j != i && players[j].hp_armor > 0.0f)
							{
								float t1 = bg3_ray_ellipsoid_collision(&players[j].e_mat, &players[j].e_pos, (xVector3f*)&mat.w, (xVector3f*)&dir);
								if (t1 >= 0.0f && t1 < t0)
								{
									t0 = t1;
									enemy = j;
								}
							}
						}
						dir.x *= t0;
						dir.y *= t0;
						dir.z *= t0;
						//printf("enemy: %i, t0: %f\n", enemy, t0);
						if (players[i].laser_ammo > 0.0f)
						{
							g->scores[i].shots_fired += dt;
							if (enemy >= 0)
							{
								//hit enemy
								hit.x = mat.w.x + dir.x;
								hit.y = mat.w.y + dir.y;
								hit.z = mat.w.z + dir.z;
								//players[enemy].shield_fade = SHIELD_FADE_TIME;
								bg3_damage_player(base, i, enemy, LASER_SHIELD_DPS*dt, LASER_ARMOR_DPS*dt);
								g->scores[i].shots_hit += dt;
								//players[i].laser_hit_last = 0;
								xParticleEmitter e;
								e.particle_pos = *(xVector3f*)&hit;
								e.new_velocity = 0;
								xParticleSystemBurst(effects->lasermark_ps, &e, 1);
							}
							else if (t_cam < CAM_RAY_DIST || t0 < 1.0f)
							{
								//hit ground
								xParticleEmitter e;
								e.particle_pos = *(xVector3f*)&hit;
								e.new_velocity = 0;
								xParticleSystemBurst(effects->lasermark_ps, &e, 1);
							}
							else
							{
								//players[i].laser_hit_last = 0;
							}

							players[i].laser_start = *(ScePspFVector3*)&mat.w;
							players[i].laser_len = dir;
							players[i].laser_time += dt;

							float len = xVec3Length((xVector3f*)&dir);
							gumLoadIdentity(&mat);
							mat.w.x = players[i].laser_start.x;
							mat.w.y = players[i].laser_start.y;
							mat.w.z = players[i].laser_start.z;
							xVec3Normalize((xVector3f*)&mat.y, (xVector3f*)&dir);
							mat.z.x = 0.0f;
							mat.z.y = 0.0f;
							mat.z.z = 1.0f;
							xVec3Cross((xVector3f*)&mat.x, (xVector3f*)&mat.y, (xVector3f*)&mat.z);
							xVec3Cross((xVector3f*)&mat.z, (xVector3f*)&mat.x, (xVector3f*)&mat.y);
							ScePspFVector3 mv = {0.0f, LASER_STEP, 0.0f};
							ScePspFVector3 radial = {0.0f, 0.0f, LASER_RADIUS};
							float t = 0.0f;
							while (t < len)
							{
								ScePspFMatrix4 mat2 = mat;
								gumRotateY(&mat2, LASER_PHASE*players[i].laser_time - LASER_TRIG_STEP*t);
								gumTranslate(&mat2, &radial);
								xParticleEmitter e;
								e.particle_pos = *(xVector3f*)&mat2.w;
								e.new_velocity = 0;
								xParticleSystemBurst(effects->laser_ps, &e, 1);
								gumTranslate(&mat, &mv);
								t += LASER_STEP;
							}
							players[i].laser_ammo -= dt;
							if (players[i].laser_ammo < 0.0f)
								players[i].laser_ammo = 0.0f;

							//play sound
							players[i].laser_snd_src.pos = players[i].laser_start;
							players[i].laser_snd_src.vel = players[i].vel;
							players[i].laser_snd_src.radius = SOUND_RADIUS;
							if (players[i].laser_snd_ref < 0)
							{
								players[i].laser_snd_ref = xSound3dPlay(resources->laser_sound, &players[i].laser_snd_src, 1);
							}
						}
					}
					break;
				case BG3_TANK_SHELL:
					if (players[i].tshell_wait <= 0.0f)
					{
						players[i].tshell_wait = TSHELL_WAIT;
						ScePspFVector3 shootat, dir, hit;
						bg3_ray_heightmap_collision(&map->hmp, &shootat, &players[i].cam_orig, &players[i].cam_look, CAM_RAY_DIST, 0.0f);
						ScePspFMatrix4 mat = players[i].top_mat;
						gumTranslate(&mat, &resources->tank_turret->pos);
						gumRotateX(&mat, players[i].turret_pitch);
						gumTranslate(&mat, &base->tshell_offset);
						dir.x = shootat.x - mat.w.x;
						dir.y = shootat.y - mat.w.y;
						dir.z = shootat.z - mat.w.z;
						float t0 = bg3_ray_heightmap_collision(&map->hmp, &hit, (ScePspFVector3*)&mat.w, &dir, 1.0f, X_EPSILON);
						int enemy = -1;
						for (j = 0; j < g->num_players; j++)
						{
							if (j != i && players[j].hp_armor > 0.0f)
							{
								float t1 = bg3_ray_ellipsoid_collision(&players[j].e_mat, &players[j].e_pos, (xVector3f*)&mat.w, (xVector3f*)&dir);
								if (t1 >= 0.0f && t1 < t0)
								{
									t0 = t1;
									enemy = j;
								}
							}
						}
						if (players[i].tshell_ammo > 0)
						{
							if (enemy >= 0)
							{
								hit.x = mat.w.x + t0*dir.x;
								hit.y = mat.w.y + t0*dir.y;
								hit.z = mat.w.z + t0*dir.z;
								//cause damage to enemy
								//players[enemy].shield_fade = SHIELD_FADE_TIME;
								//bg3_damage_player(enemy, TSHELL_SHIELD_DPS*TSHELL_WAIT, TSHELL_ARMOR_DPS*TSHELL_WAIT);
							}
							else if (t0 < 1.0f)
							{
								//hit ground
								bg3_add_decal(effects->scorch_decals, &map->hmp, &hit);
							}
							//add shell, damage enemies
							bg3_add_tshell(g->tshells, base, (ScePspFVector3*)&mat.w, &hit);
							g->scores[i].shots_fired += 1.0f;
							if (bg3_damage_area(base, i, &hit, TSHELL_DMG_INNER_RADIUS, TSHELL_DMG_OUTER_RADIUS, TSHELL_SHIELD_DPS*TSHELL_WAIT, TSHELL_ARMOR_DPS*TSHELL_WAIT) > 0)
							{
								g->scores[i].shots_hit += 1.0f;
							}
							bg3_create_explosion(base, (xVector3f*)&hit);

							xParticleEmitter e;
							e.particle_pos = *(xVector3f*)&mat.w;
							e.new_velocity = 0;
							xParticleSystemBurst(effects->muzzleflash_ps, &e, 1);
							xParticleSystemBurst(effects->gun_smoke_ps, &e, 1);
							//play sound
							players[i].tshell_ammo -= 1;
							if (players[i].tshell_ammo < 0)
								players[i].tshell_ammo = 0;
						}

					}
					break;
				/*
				case BG3_GAUSS:
					if (players[i].gauss_wait <= 0.0f)
					{
					//get collision(s)
					//bg3_add_shell(...);
					players[i].gauss_wait = GAUSS_WAIT;
					}
					break;
					*/
				case BG3_MISSILES:
					if (players[i].missile_wait <= 0.0f)
					{
						players[i].missile_wait = MISSILE_WAIT;
						//add 2 missiles
						if (players[i].missile_ammo > 0)
						{
							ScePspFVector3 shootat;
							bg3_ray_heightmap_collision(&map->hmp, &shootat, &players[i].cam_orig, &players[i].cam_look, CAM_RAY_DIST, 0.0f);
							ScePspFVector3 dir = *(ScePspFVector3*)&players[i].top_mat.z;
							ScePspFMatrix4 mat = players[i].top_mat;
							gumTranslate(&mat, &base->missile_offset);
							bg3_add_missile(g->missiles, base, i, (ScePspFVector3*)&mat.w, &dir, &shootat);
							g->scores[i].shots_fired += 1.0f;
							//play sound
							players[i].missile_ammo -= 1;
							if (players[i].missile_ammo < 0)
								players[i].missile_ammo = 0;

							if (players[i].missile_ammo > 0)
							{
								ScePspFVector3 move = {-2*base->missile_offset.x, 0.0f, 0.0f};
								gumTranslate(&mat, &move);
								bg3_add_missile(g->missiles, base, i, (ScePspFVector3*)&mat.w, &dir, &shootat);
								g->scores[i].shots_fired += 1.0f;
								//play sound
								players[i].missile_ammo -= 1;
								if (players[i].missile_ammo < 0)
									players[i].missile_ammo = 0;
							}

						}

					}
					break;
				}
			}
		}
		else
		{
			//player is dead
			players[i].death_time += dt;
			if (players[i].death_time >= (float)bg3_game_spawns[g->values.spawn_time_id])
			{
				//respawn
				bg3_spawn_player(base, i);
			}
		}

		if (!players[i].firing || players[i].primary != BG3_LASER)
		{
			//players[i].laser_hit_last = 0;
			players[i].laser_time = 0.0f;
			if (players[i].laser_snd_ref >= 0)
			{
				xSoundSetState(players[i].laser_snd_ref, X_SOUND_STOP);
				players[i].laser_snd_ref = -1;
			}
		}

		if (g->scores[i].kills >= bg3_game_scores[g->values.score_target_id])
		{
			g->game_over = 1;
		}
	}
}

void bg3_game_update_render(bg3_base* base, bg3_menu* menu, float dt)
{
	if (base == NULL) return;
	bg3_game* g = &base->game;
	bg3_map* map = g->map;
	bg3_player* players = g->players;
	bg3_resources* resources = &base->resources;
	bg3_effects* effects = &base->effects;

	if (!g->paused)
	{
		bg3_update_tshells(g->tshells, dt);
		bg3_update_bullets(g->bullets, base, dt);
		bg3_update_missiles(g->missiles, base, dt);
		bg3_update_powerups(g->powerups, base, dt);
		xParticleSystemUpdate(effects->dirt_ps, dt);
		xParticleSystemUpdate(effects->missile_ps, dt);
		xParticleSystemUpdate(effects->expl_flash_ps, dt);
		xParticleSystemUpdate(effects->expl_flames_ps, dt);
		xParticleSystemUpdate(effects->expl_debris_ps, dt);
		xParticleSystemUpdate(effects->expl_sparks_ps, dt);
		xParticleSystemUpdate(effects->expl_smoke_ps, dt);
		xParticleSystemUpdate(effects->sparks_ps, dt);
		xParticleSystemUpdate(effects->smoke_ps, dt);
		xParticleSystemUpdate(effects->lasermark_ps, dt);
		xParticleSystemUpdate(effects->recharge_ps, dt);
		xParticleSystemUpdate(effects->dust_ps, dt);
		if (effects->wind_ps != NULL)
		{
			effects->wind_ps->pos = *(xVector3f*)&players[g->player].pos;
			xVec3Sub(&effects->wind_ps->pos, (xVector3f*)&players[g->player].pos, &effects->wind_ps->vel);
		}
		xParticleSystemUpdate(effects->wind_ps, dt);
		xParticleSystemUpdate(effects->laser_ps, dt);
		xParticleSystemUpdate(effects->muzzleflash_ps, dt);
		xParticleSystemUpdate(effects->powerup_ps, dt);
		xParticleSystemUpdate(effects->gun_smoke_ps, dt);
		xParticleSystemUpdate(effects->powerup_pickup_ps, dt);
	}


	xGuClear(g->fog_color);

	sceGumMatrixMode(GU_VIEW);
	ScePspFMatrix4 view;
	gumLoadIdentity(&view);
	{
		ScePspFVector3 major = {0.0f, 0.0f, 1.0f};
		ScePspFVector3 right, fwd, up;
		fwd = players[g->player].cam_look;
		x_crossproduct(&right, &fwd, &major);
		x_normalize(&right, 1.0f);
		x_crossproduct(&up, &right, &fwd);
		*(ScePspFVector3*)&view.x = right;
		*(ScePspFVector3*)&view.y = up;
		view.z.x = -fwd.x;
		view.z.y = -fwd.y;
		view.z.z = -fwd.z;
		*(ScePspFVector3*)&view.w = players[g->player].cam_orig;
		sceGumLoadMatrix(&view);
		sceGumFastInverse();

		xSound3dSetListener(&view, &players[g->player].vel);
		xSound3dUpdate();
	}

	sceGumMatrixMode(GU_MODEL);

	sceGuDisable(GU_LIGHTING);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuDisable(GU_BLEND);
	xGuTexFilter(X_BILINEAR);

	sceGuFog(g->fog_near, g->fog_far, g->fog_color);

#ifdef DRAW_HEIGHTMAP
	while (lod_time >= LOD_DELAY)
	{
		xHeightmapBuildLOD(&map->hmp_lod, &players[g->player].pos, 50*1.0f, 50*1.0f);
		lod_time -= LOD_DELAY;
	}
#endif

	sceGumLoadIdentity();
	xTexSetImage(map->terrain_tex);
	sceGuColor(0xffffffff);
#ifdef DRAW_HEIGHTMAP
	//xHeightmapDraw(&map->hmp);

	sceGuEnable(GU_FOG);
	xHeightmapDrawLOD(&map->hmp_lod);
	sceGuDisable(GU_FOG);

	//bg3_draw_terrain(&map->hmp_lod, &players[g->player].cam_orig);
#endif

	sceGuEnable(GU_LIGHTING);
	sceGuEnable(GU_TEXTURE_2D);
	xGuTexFilter(X_BILINEAR);


	int i;
#ifdef DRAW_TANKS
	for (i = 0; i < g->num_players; i++)
	{
		if (players[i].hp_armor > 0.0f)
		{
			if (bg3_check_visibility(&map->hmp, &players[g->player].cam_orig, (ScePspFVector3*)&players[i].e_pos) >= 1.0f)
			{
				sceGuEnable(GU_FOG);

				sceGuColor(0xffffffff);
				xTexSetImage(resources->tank_tex);
				sceGumLoadMatrix(&players[i].base_mat);
				xObjDraw(resources->tank_base, 0);
				sceGumLoadMatrix(&players[i].top_mat);
				sceGumPushMatrix();
				xObjTranslate(resources->tank_top);
				xObjDraw(resources->tank_top, 0);
				sceGumPopMatrix();
				xObjTranslate(resources->tank_turret);
				sceGumRotateX(players[i].turret_pitch);
				xObjDraw(resources->tank_turret, 0);

				sceGuDisable(GU_FOG);

	#if 1
				xVector3f vec;
				xVec3Sub(&vec, (xVector3f*)&players[i].base_mat.w, (xVector3f*)&view.w);
				float sq_dist = xVec3SqLength(&vec);

				if (sq_dist < SQR(2*LOD_DIST))
				{
					if (sq_dist < SQR(LOD_DIST))
					{
						//draw envmapped meshes

						bg3_envmap_pass_start(0.0f, 0.0f, 0.0f);
						xTexSetImage(map->sky_tex);
						sceGumLoadMatrix(&players[i].base_mat);
						xObjDraw(resources->tank_base, 0);
						sceGumLoadMatrix(&players[i].top_mat);
						sceGumPushMatrix();
						xObjTranslate(resources->tank_top);
						xObjDraw(resources->tank_top, 0);
						sceGumPopMatrix();
						xObjTranslate(resources->tank_turret);
						sceGumRotateX(players[i].turret_pitch);
						xObjDraw(resources->tank_turret, 0);
						bg3_envmap_pass_end();

						//draw shadowed meshes

						if (bg3_shadow_setrendertarget(effects->shadow, (ScePspFVector3*)&players[i].base_mat.w, &map->light_pos, SHADOW_FOV, SHADOW_DIST, 0.5f, 1) == 0)
						{
							//sceGumLoadMatrix(&players[i].base_mat);
							//xObjDraw(tank_base, 0);
							sceGumLoadMatrix(&players[i].top_mat);
							sceGumPushMatrix();
							xObjTranslate(resources->tank_top);
							xObjDraw(resources->tank_top, 0);
							sceGumPopMatrix();
							xObjTranslate(resources->tank_turret);
							sceGumRotateX(players[i].turret_pitch);
							xObjDraw(resources->tank_turret, 0);
							bg3_shadow_endrendertarget();
						}
						sceGumLoadMatrix(&players[i].base_mat);
						sceGumScale(&resources->tank_base->scale);
						if (bg3_shadowbuf_pass_start(effects->shadow) == 0)
						{
							ScePspFVector3 inv_scale = {1.0f/resources->tank_base->scale.x, 1.0f/resources->tank_base->scale.y, 1.0f/resources->tank_base->scale.z};
							sceGumScale(&inv_scale);
							xObjDraw(resources->tank_base, 0);
							bg3_shadow_pass_end();
						}

						if (bg3_shadow_setrendertarget(effects->shadow, (ScePspFVector3*)&players[i].base_mat.w, &map->light_pos, SHADOW_FOV, SHADOW_DIST, 0.5f, 0) == 0)
						{
							sceGumLoadMatrix(&players[i].base_mat);
							xObjDraw(resources->tank_base, 0);
							bg3_shadow_endrendertarget();
						}

						//draw detailed shadowed terrain

						sceGumLoadIdentity();
						if (bg3_shadowbuf_pass_start(effects->shadow) == 0)
						{
	#ifdef DRAW_HEIGHTMAP
							//xHeightmapDrawSection(&map->hmp, players[i].pos.x - 8*2.0f, players[i].pos.y - 8*2.0f, 16*2.0f, 16*2.0f);
							//xHeightmapDrawLOD(&map->hmp_lod);
							xHeightmapDrawPatchID(&map->hmp_lod, xHeightmapGetPatchID(&map->hmp_lod, players[i].pos.x, players[i].pos.y));
	#endif
							bg3_shadow_pass_end();
						}
					}
					else
					{
#if 0
						//draw low detail shadowed terrain

						ScePspFMatrix4 shadow_proj;
						bg3_shadow_projection(&shadow_proj, (ScePspFVector3*)&players[i].base_mat.w, &map->light_pos, SHADOW_FOV, SHADOW_DIST);
						xTexSetImage(resources->shadow_tex);
						sceGumLoadIdentity();
						if (bg3_shadow_pass_start(&shadow_proj) == 0)
						{
	#ifdef DRAW_HEIGHTMAP
							//xHeightmapDrawSection(&map->hmp, players[i].pos.x - 8*2.0f, players[i].pos.y - 8*2.0f, 16*2.0f, 16*2.0f);
							//xHeightmapDrawLOD(&map->hmp_lod);
							xHeightmapDrawPatchID(&map->hmp_lod, xHeightmapGetPatchID(&map->hmp_lod, players[i].pos.x, players[i].pos.y));
	#endif
							bg3_shadow_pass_end();
						}
#endif

#if 1
						if (bg3_shadow_setrendertarget(effects->shadow, (ScePspFVector3*)&players[i].base_mat.w, &map->light_pos, SHADOW_FOV, SHADOW_DIST, 0.5f, 1) == 0)
						{
							//sceGumLoadMatrix(&players[i].base_mat);
							//xObjDraw(tank_base, 0);
							sceGumLoadMatrix(&players[i].top_mat);
							sceGumPushMatrix();
							xObjTranslate(resources->tank_top);
							xObjDraw(resources->tank_top, 0);
							sceGumPopMatrix();
							xObjTranslate(resources->tank_turret);
							sceGumRotateX(players[i].turret_pitch);
							xObjDraw(resources->tank_turret, 0);
							bg3_shadow_endrendertarget();
						}
						sceGumLoadMatrix(&players[i].base_mat);
						sceGumScale(&resources->tank_base->scale);
						if (bg3_shadowbuf_pass_start(effects->shadow) == 0)
						{
							ScePspFVector3 inv_scale = {1.0f/resources->tank_base->scale.x, 1.0f/resources->tank_base->scale.y, 1.0f/resources->tank_base->scale.z};
							sceGumScale(&inv_scale);
							xObjDraw(resources->tank_base, 0);
							bg3_shadow_pass_end();
						}

						if (bg3_shadow_setrendertarget(effects->shadow, (ScePspFVector3*)&players[i].base_mat.w, &map->light_pos, SHADOW_FOV, SHADOW_DIST, 0.5f, 0) == 0)
						{
							sceGumLoadMatrix(&players[i].base_mat);
							xObjDraw(resources->tank_base, 0);
							bg3_shadow_endrendertarget();
						}

						//draw detailed shadowed terrain

						sceGumLoadIdentity();
						if (bg3_shadowbuf_pass_start(effects->shadow) == 0)
						{
#ifdef DRAW_HEIGHTMAP
							//xHeightmapDrawSection(&map->hmp, players[i].pos.x - 8*2.0f, players[i].pos.y - 8*2.0f, 16*2.0f, 16*2.0f);
							//xHeightmapDrawLOD(&map->hmp_lod);
							xHeightmapDrawPatchID(&map->hmp_lod, xHeightmapGetPatchID(&map->hmp_lod, players[i].pos.x, players[i].pos.y));
#endif
							bg3_shadow_pass_end();
						}
#else
#define BUF_WIDTH 32
						xBuffer buf;
						buf.data = effects->shadow->buf0->data;
						buf.width = BUF_WIDTH;
						buf.height = BUF_WIDTH;
						buf.buf_width = BUF_WIDTH;
						buf.pow2_height = BUF_WIDTH;
						buf.u_scale = 1.0f;
						buf.v_scale = 1.0f;
						bg3_shadow shadow;
						shadow.buf0 = &buf;
						if (bg3_shadow_setrendertarget(&shadow, (ScePspFVector3*)&players[i].base_mat.w, &map->light_pos, SHADOW_FOV, SHADOW_DIST, 0.5f, 1) == 0)
						{
							sceGumLoadMatrix(&players[i].base_mat);
							xObjDraw(resources->tank_base, 0);
							sceGumLoadMatrix(&players[i].top_mat);
							sceGumPushMatrix();
							xObjTranslate(resources->tank_top);
							xObjDraw(resources->tank_top, 0);
							sceGumPopMatrix();
							xObjTranslate(resources->tank_turret);
							sceGumRotateX(players[i].turret_pitch);
							xObjDraw(resources->tank_turret, 0);
							bg3_shadow_endrendertarget();
						}
						sceGumLoadIdentity();
						if (bg3_shadowbuf_pass_start(&shadow) == 0)
						{
#ifdef DRAW_HEIGHTMAP
							//xHeightmapDrawSection(&map->hmp, players[i].pos.x - 8*2.0f, players[i].pos.y - 8*2.0f, 16*2.0f, 16*2.0f);
							//xHeightmapDrawLOD(&map->hmp_lod);
							xHeightmapDrawPatchID(&map->hmp_lod, xHeightmapGetPatchID(&map->hmp_lod, players[i].pos.x, players[i].pos.y));
#endif
							bg3_shadow_pass_end();
						}
#endif

					}
				}
	#endif


			}
		}
	}
#endif

	sceGuColor(0xffffffff);

	sceGuEnable(GU_FOG);

	sceGuDisable(GU_BLEND);
	sceGuDisable(GU_LIGHTING);
	sceGuEnable(GU_TEXTURE_2D);
	xGuTexFilter(X_BILINEAR);

	sceGumLoadIdentity();
	sceGumTranslate((ScePspFVector3*)&view.w);
	xGumScale(500.0f, 500.0f, 500.0f);
	xTexSetImage(map->sky_tex);
	sceGuDepthMask(GU_TRUE);
	xObjDraw(resources->sky_obj, 0);
	sceGuDepthMask(GU_FALSE);

	//sceGuFog(g->fog_near, g->fog_far, 0x000000);

	sceGumLoadIdentity();

	sceGuEnable(GU_BLEND);
	sceGuEnable(GU_TEXTURE_2D);

	xTexSetImage(resources->bulletmark_tex);
	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_MUL_INV_COLOR, g->fog_color));
	bg3_draw_decals(effects->bullet_decals);

	xTexSetImage(resources->scorch_tex);
	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_MUL_COLOR, g->fog_color));
	bg3_draw_decals(effects->scorch_decals);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->lasermark_tex);
	xParticleSystemRender(effects->lasermark_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->dirt_particle_tex);
	xParticleSystemRender(effects->dirt_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->dirt_particle_tex);
	xParticleSystemRender(effects->dust_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->flash_tex);
	xParticleSystemRender(effects->powerup_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_AVERAGE_WITH_ALPHA, g->fog_color));
	bg3_draw_powerups(g->powerups, base, &view);

	sceGumLoadIdentity();

#ifdef DRAW_TANKS
	xTexSetImage(resources->shield_tex);
	sceGuEnable(GU_LIGHTING);
	//sceGuDisable(GU_TEXTURE_2D);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_BLEND);
	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	sceGuDepthMask(GU_TRUE);
	for (i = 0; i < g->num_players; i++)
	{
		if (players[i].shield_fade > 0.0f && players[i].hp_armor > 0.0f)
		{
			sceGuColor(GU_COLOR(0.0f, 1.0f, 1.0f, players[i].shield_fade/SHIELD_FADE_TIME));
			sceGumLoadMatrix(&players[i].base_mat);
			sceGumTranslate(&base->hit_ellipsoid_pos);
			sceGumScale(&base->hit_ellipsoid_radii);
			xGumScale(SHIELD_SCALE, SHIELD_SCALE, SHIELD_SCALE);
			xObjDraw(resources->sphere_obj, 0);
		}
	}
	sceGuDisable(GU_BLEND);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuDisable(GU_LIGHTING);
	sceGuDepthMask(GU_FALSE);
#endif

	sceGumLoadIdentity();

	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_BLEND);
	sceGuColor(0xffffffff);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	//xTexSetImage(bullet_tex);
	xTexSetImage(resources->tshell_tex);
	bg3_draw_bullets(g->bullets, &view, 5.0f, 0.3f, 0.3f, 0x00ffffff, 0xffffffff);

	xTexSetImage(resources->tshell_tex);
	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	bg3_draw_tshells(g->tshells, &view);

	xTexSetImage(resources->laser_tex);
	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_SIMPLE, g->fog_color));
	sceGuTexWrap(GU_REPEAT, GU_CLAMP);
	sceGuDepthMask(GU_TRUE);
	for (i = 0; i < g->num_players; i++)
	{
		if (players[i].firing && players[i].primary == BG3_LASER && players[i].hp_armor > 0.0f)
		{
			//firing laser
			sceGuTexScale(1.0f*xVec3Length((xVector3f*)&players[i].laser_len), 1.0f);
			sceGuTexOffset(-5.0f*players[i].laser_time, 0.0f);
			float h = 0.3f + 0.1f*x_sinf(40.0f*players[i].laser_time);
			bg3_draw_quad_billboard((xVector3f*)&view.w, (xVector3f*)&players[i].laser_start, (xVector3f*)&players[i].laser_len, h, h, 0xffffffff, 0xffffffff);
		}
	}
	sceGuDepthMask(GU_FALSE);
	sceGuTexWrap(GU_CLAMP, GU_CLAMP);
	sceGuTexScale(1.0f, 1.0f);
	sceGuTexOffset(0.0f, 0.0f);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->flash_tex);
	xParticleSystemRender(effects->laser_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->muzzleflash_tex);
	xParticleSystemRender(effects->muzzleflash_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_MUL_INV_COLOR, g->fog_color));
	xTexSetImage(resources->smoke_tex);
	xParticleSystemRender(effects->smoke_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_MUL_INV_COLOR, g->fog_color));
	xTexSetImage(resources->smoke_tex);
	xParticleSystemRender(effects->gun_smoke_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->flash_tex);
	bg3_draw_missiles(g->missiles, &view, 2.0f);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->smoke_tex);
	xParticleSystemRender(effects->missile_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_MUL_INV_COLOR, g->fog_color));
	xTexSetImage(resources->smoke_tex);
	xParticleSystemRender(effects->expl_smoke_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_MUL_INV_COLOR, g->fog_color));
	xTexSetImage(resources->dirt_particle_tex);
	xParticleSystemRender(effects->expl_debris_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->explosion_tex);
	xParticleSystemRender(effects->expl_flames_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->flash_tex);
	xParticleSystemRender(effects->expl_sparks_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->flash_tex);
	xParticleSystemRender(effects->expl_flash_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->flash_tex);
	xParticleSystemRender(effects->sparks_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->flash_tex);
	xParticleSystemRender(effects->recharge_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->flash_tex);
	xParticleSystemRender(effects->powerup_pickup_ps, &view);

	sceGuFog(g->fog_near, g->fog_far, bg3_set_blend(BLEND_ADD_WITH_ALPHA, g->fog_color));
	xTexSetImage(resources->dirt_particle_tex);
	xParticleSystemRender(effects->wind_ps, &view);


	sceGuDisable(GU_FOG);

	sceGuEnable(GU_BLEND);

	if (g->paused)
	{
		//paused

		sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

		bg3_draw_tex_center(base->logo_tex, X_SCREEN_WIDTH/2, 40);

#define MENU_BOX_WIDTH 250
#define MENU_BOX_HEIGHT 80
#define MENU_BOX_Y 100

		bg3_draw_box(X_SCREEN_WIDTH/2-MENU_BOX_WIDTH/2, MENU_BOX_Y, MENU_BOX_WIDTH, MENU_BOX_HEIGHT, 0x7f555555, 0xff000000);

#define MENU_START_X		(X_SCREEN_WIDTH/2-100)
#define MENU_START_Y		110
#define MENU_VALUE_X		(X_SCREEN_WIDTH/2+100)
#define MENU_TEXT_HEIGHT	15
#define MENU_SELECT_HEIGHT	16
#define MENU_NO_X			(X_SCREEN_WIDTH/2-40)
#define MENU_YES_X			(X_SCREEN_WIDTH/2+40)
#define MENU_YESNO_WIDTH	50

		int y;
		int width;

		switch (menu->screen)
		{
		case MENU_MAIN:
			y = MENU_START_Y + menu->selection*MENU_TEXT_HEIGHT;
			width = 100;
			bg3_draw_box(X_SCREEN_WIDTH/2-width/2, y, width, MENU_SELECT_HEIGHT, 0xff7f7f7f, 0xff000000);

			xTextSetAlign(X_ALIGN_CENTER);
			y = MENU_START_Y;
			bg3_print_text(X_SCREEN_WIDTH/2, y, "Resume");
			y += MENU_TEXT_HEIGHT;
			bg3_print_text(X_SCREEN_WIDTH/2, y, "Options");
			y += MENU_TEXT_HEIGHT;
			bg3_print_text(X_SCREEN_WIDTH/2, y, "Quit");
			break;
		case MENU_OPTIONS:
			y = MENU_START_Y + menu->selection*MENU_TEXT_HEIGHT;
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
		case MENU_EXIT:
			xTextSetAlign(X_ALIGN_CENTER);
			y = MENU_START_Y;
			bg3_print_text(X_SCREEN_WIDTH/2, y, "Are you sure?");
			y += MENU_TEXT_HEIGHT;
			if (menu->selection == 0)
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
	else
	{
		//unpaused

		if (xCtrlPress(PSP_CTRL_SELECT) || g->game_over)
		{
			//show scores

			sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

	#define SCORES_BOX_WIDTH 200
	#define SCORES_BOX_HEIGHT 125
	#define SCORES_BOX_Y 100

			bg3_draw_box(X_SCREEN_WIDTH/2-SCORES_BOX_WIDTH/2, SCORES_BOX_Y, SCORES_BOX_WIDTH, SCORES_BOX_HEIGHT, 0x7f555555, 0xff000000);

	#define SCORES_START_X		(X_SCREEN_WIDTH/2-75)
	#define SCORES_START_Y		110
	#define SCORES_VALUE_X		(X_SCREEN_WIDTH/2+75)
	#define SCORES_SELECT_WIDTH	50
	#define SCORES_TEXT_HEIGHT	15

			int y = SCORES_START_Y;
			xTextSetAlign(X_ALIGN_LEFT);
			bg3_print_text(SCORES_START_X, y, "Kills:");
			xTextSetAlign(X_ALIGN_RIGHT);
			bg3_print_text(SCORES_VALUE_X, y, "%i", g->scores[g->player].kills);
			y += SCORES_TEXT_HEIGHT;
			xTextSetAlign(X_ALIGN_LEFT);
			bg3_print_text(SCORES_START_X, y, "Assists:");
			xTextSetAlign(X_ALIGN_RIGHT);
			bg3_print_text(SCORES_VALUE_X, y, "%i", g->scores[g->player].assists);
			y += SCORES_TEXT_HEIGHT;
			xTextSetAlign(X_ALIGN_LEFT);
			bg3_print_text(SCORES_START_X, y, "Deaths:");
			xTextSetAlign(X_ALIGN_RIGHT);
			bg3_print_text(SCORES_VALUE_X, y, "%i", g->scores[g->player].deaths);
			y += SCORES_TEXT_HEIGHT;
			xTextSetAlign(X_ALIGN_LEFT);
			bg3_print_text(SCORES_START_X, y, "Suicides:");
			xTextSetAlign(X_ALIGN_RIGHT);
			bg3_print_text(SCORES_VALUE_X, y, "%i", g->scores[g->player].suicides);
			y += SCORES_TEXT_HEIGHT;
			xTextSetAlign(X_ALIGN_LEFT);
			bg3_print_text(SCORES_START_X, y, "Shots Fired:");
			xTextSetAlign(X_ALIGN_RIGHT);
			bg3_print_text(SCORES_VALUE_X, y, "%.2f", g->scores[g->player].shots_fired);
			y += SCORES_TEXT_HEIGHT;
			xTextSetAlign(X_ALIGN_LEFT);
			bg3_print_text(SCORES_START_X, y, "Shots Hit:");
			xTextSetAlign(X_ALIGN_RIGHT);
			bg3_print_text(SCORES_VALUE_X, y, "%.2f", g->scores[g->player].shots_hit);
			y += SCORES_TEXT_HEIGHT;
			xTextSetAlign(X_ALIGN_LEFT);
			bg3_print_text(SCORES_START_X, y, "Accuracy:");
			xTextSetAlign(X_ALIGN_RIGHT);
			if (g->scores[g->player].shots_fired <= 0.0f)
			{
				bg3_print_text(SCORES_VALUE_X, y, "NaN");
			}
			else
			{
				bg3_print_text(SCORES_VALUE_X, y, "%2.2f%%", g->scores[g->player].shots_hit/g->scores[g->player].shots_fired);
			}
			y += SCORES_TEXT_HEIGHT;

			if (g->game_over)
			{
				//draw "ok" box
				xTextSetAlign(X_ALIGN_CENTER);
				bg3_draw_box(X_SCREEN_WIDTH/2-SCORES_SELECT_WIDTH/2, y, SCORES_SELECT_WIDTH, MENU_SELECT_HEIGHT, 0xff7f7f7f, 0xff000000);
				bg3_print_text(X_SCREEN_WIDTH/2, y, "Finish");
			}

		}
		else if (players[g->player].hp_armor > 0.0f)
		{
			//not dead, draw hud

	#define ARMOR_START 37
	#define ARMOR_WIDTH 100
	#define SHIELDS_START 12
	#define SHIELDS_WIDTH 200
	#define HP_DISPLAY_X 2
	#define HP_DISPLAY_Y 2

			sceGuBlendFunc(GU_ADD, GU_FIX, GU_ONE_MINUS_SRC_COLOR, 0xffffffff, 0);
			bg3_draw_tex_center(resources->crosshair_tex, X_SCREEN_WIDTH/2, X_SCREEN_HEIGHT/2);

			sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
			/*
			bg3_draw_tex(resources->hp_hud_tex, HP_DISPLAY_X, HP_DISPLAY_Y);
			xTexDraw(
				resources->hp_armor_tex, HP_DISPLAY_X, HP_DISPLAY_Y,
				ARMOR_START + (int)(players[g->player].hp_armor*ARMOR_WIDTH),
				resources->hp_hud_tex->height,
				0, 0,
				ARMOR_START + (int)(players[g->player].hp_armor*ARMOR_WIDTH),
				resources->hp_hud_tex->height);
			sceGuBlendFunc(GU_ADD, GU_FIX, GU_SRC_COLOR, 0, 0);
			xTexDraw(
				resources->hp_shields_tex, HP_DISPLAY_X, HP_DISPLAY_Y,
				SHIELDS_START + (int)(players[g->player].hp_shields*SHIELDS_WIDTH),
				resources->hp_shields_tex->height,
				0, 0,
				SHIELDS_START + (int)(players[g->player].hp_shields*SHIELDS_WIDTH),
				resources->hp_shields_tex->height);
				*/
			bg3_draw_rect(HP_DISPLAY_X-1, HP_DISPLAY_Y-1, SHIELDS_WIDTH+1, 10, 0xafdddddd);
			bg3_draw_outline(HP_DISPLAY_X-1, HP_DISPLAY_Y-1, SHIELDS_WIDTH+1, 10, 0xff000000);
			bg3_draw_vert_grad(HP_DISPLAY_X, HP_DISPLAY_Y, (int)(players[g->player].hp_shields*SHIELDS_WIDTH), 9, 0xaf957000, 0xafffc000);
			bg3_draw_rect(HP_DISPLAY_X-1, HP_DISPLAY_Y+12-1, ARMOR_WIDTH+1, 10, 0xafdddddd);
			bg3_draw_outline(HP_DISPLAY_X-1, HP_DISPLAY_Y+12-1, ARMOR_WIDTH+1, 10, 0xff000000);
			bg3_draw_vert_grad(HP_DISPLAY_X, HP_DISPLAY_Y+12, (int)(players[g->player].hp_armor*ARMOR_WIDTH), 9, 0xaf0000af, 0xaf0000ef);
			sceGuDisable(GU_BLEND);

	#define ICON_START 350
	#define ICON_WIDTH 32
	#define ICON_FILL 0x7f7f7f7f
	#define ICON_COLOR0 0xffffffff
	#define ICON_COLOR1 0xff303030
	#define ICON_OUTLINE 0xff000000
	#define ICON_PRIMARY 0xff00c6ff

			sceGuEnable(GU_BLEND);
			sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

			int x = ICON_START;
			int y = 0;
			bg3_draw_rect(x, y, ICON_WIDTH, ICON_WIDTH, ICON_FILL);
			bg3_draw_outline(x-1, y-1, ICON_WIDTH, ICON_WIDTH+1, ICON_OUTLINE);
			if (players[g->player].weapons & (1<<BG3_MACHINE_GUN))
				sceGuColor(ICON_COLOR0);
			else
				sceGuColor(ICON_COLOR1);
			bg3_draw_tex2(resources->mgun_icon, x, y, ICON_WIDTH, ICON_WIDTH);
			x += ICON_WIDTH;
			bg3_draw_rect(x, y, ICON_WIDTH, ICON_WIDTH, ICON_FILL);
			bg3_draw_outline(x-1, y-1, ICON_WIDTH, ICON_WIDTH+1, ICON_OUTLINE);
			if (players[g->player].weapons & (1<<BG3_LASER))
				sceGuColor(ICON_COLOR0);
			else
				sceGuColor(ICON_COLOR1);
			bg3_draw_tex2(resources->laser_icon, x, y, ICON_WIDTH, ICON_WIDTH);
			x += ICON_WIDTH;
			bg3_draw_rect(x, y, ICON_WIDTH, ICON_WIDTH, ICON_FILL);
			bg3_draw_outline(x-1, y-1, ICON_WIDTH, ICON_WIDTH+1, ICON_OUTLINE);
			if (players[g->player].weapons & (1<<BG3_TANK_SHELL))
				sceGuColor(ICON_COLOR0);
			else
				sceGuColor(ICON_COLOR1);
			bg3_draw_tex2(resources->tshell_icon, x, y, ICON_WIDTH, ICON_WIDTH);
			x += ICON_WIDTH;
			bg3_draw_rect(x, y, ICON_WIDTH, ICON_WIDTH, ICON_FILL);
			bg3_draw_outline(x-1, y-1, ICON_WIDTH, ICON_WIDTH+1, ICON_OUTLINE);
			if (players[g->player].weapons & (1<<BG3_MISSILES))
				sceGuColor(ICON_COLOR0);
			else
				sceGuColor(ICON_COLOR1);
			bg3_draw_tex2(resources->missile_icon, x, y, ICON_WIDTH, ICON_WIDTH);

			sceGuColor(0xffffffff);

			switch (players[g->player].primary)
			{
			case BG3_MACHINE_GUN:
				x = ICON_START + 0*ICON_WIDTH;
				break;
			case BG3_LASER:
				x = ICON_START + 1*ICON_WIDTH;
				break;
			case BG3_TANK_SHELL:
				x = ICON_START + 2*ICON_WIDTH;
				break;
			case BG3_MISSILES:
				x = ICON_START + 3*ICON_WIDTH;
				break;
			}
			y = 0;
			bg3_draw_outline(x-1, y-1, ICON_WIDTH, ICON_WIDTH+1, ICON_PRIMARY);

			sceGuDisable(GU_BLEND);

			xTextSetAlign(X_ALIGN_RIGHT);
			x = X_SCREEN_WIDTH-1;
			y = ICON_WIDTH+1;
			switch (players[g->player].primary)
			{
			case BG3_MACHINE_GUN:
				bg3_print_text(x, y, "Machine Gun");
				bg3_print_text(x, y+15, "oo");
				break;
			case BG3_LASER:
				bg3_print_text(x, y, "Laser");
				bg3_print_text(x, y+15, "%.1f", players[g->player].laser_ammo);
				break;
			case BG3_TANK_SHELL:
				bg3_print_text(x, y, "Tank Shells");
				bg3_print_text(x, y+15, "%i", players[g->player].tshell_ammo);
				break;
			case BG3_MISSILES:
				bg3_print_text(x, y, "Missiles");
				bg3_print_text(x, y+15, "%i", players[g->player].missile_ammo);
				break;
			default:
				bg3_print_text(x, y, "Unknown");
				break;
			}

			xTextSetAlign(X_ALIGN_RIGHT);
			x = X_SCREEN_WIDTH-1;
			y = X_SCREEN_HEIGHT-2*MENU_TEXT_HEIGHT;
			int timeleft = bg3_game_times[g->values.game_time_id] - (int)g->time_elapsed;
			bg3_print_text(x, y, "%i:%02i", timeleft/60, timeleft%60);
			y += MENU_TEXT_HEIGHT;
			bg3_print_text(x, y, "%i/%i", g->scores[g->player].kills, bg3_game_scores[g->values.score_target_id]);
		}

		if (g->players[g->player].hp_armor <= 0.0f)
		{
			//dead, draw spawn timer
			xTextSetAlign(X_ALIGN_CENTER);
			bg3_print_text(X_SCREEN_WIDTH/2, 235, "Respawn in:");
			bg3_print_text(X_SCREEN_WIDTH/2, 250, "%.2f", bg3_game_spawns[g->values.spawn_time_id] - g->players[g->player].death_time);
		}

	}

	switch (base->transition)
	{
	case BG3_FADE_IN:
		base->fade -= dt/GAME_FADE_TIME;
		if (base->fade <= 0.0f)
		{
			base->transition = BG3_NO_TRANSITION;
			base->fade = 0.0f;
		}
		break;
	case BG3_FADE_OUT:
		base->fade += dt/GAME_FADE_TIME;
		if (base->fade >= 1.0f)
		{
			bg3_save_stats(&base->game.scores[base->game.player]);
			base->transition = BG3_NO_TRANSITION;
			base->fade = 1.0f;
			base->state = BG3_STATE_MENU;
		}
		break;
	}

	sceGuEnable(GU_BLEND);
	bg3_set_blend(BLEND_AVERAGE_WITH_ALPHA, 0);
	bg3_draw_rect(0, 0, X_SCREEN_WIDTH, X_SCREEN_HEIGHT, GU_COLOR(0.0f, 0.0f, 0.0f, base->fade));

#ifdef X_DEBUG
	xTextSetAlign(X_ALIGN_LEFT);
	bg3_print_text(1, X_SCREEN_HEIGHT - 15, "Ticks: %d", xTimeTickrate());
#endif



	//xBufferDrawA2B(xBufferFrameBuffer(), xBufferFrameBuffer()); //screenbuf);
	//xBuffer4x4Pcf(screenbuf, xBufferFrameBuffer());

	sceGuEnable(GU_TEXTURE_2D);
	//sceGuDisable(GU_BLEND);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	//bg3_draw_tex(resources->shield_icon, 0, 0);

	xGuFrameEnd();
}

#define BG3_GAME_FPS 60
#define BG3_MIN_FPS 10

void bg3_game_loop(bg3_base* base)
{
	if (base == NULL) return;
	base->transition = BG3_FADE_IN;
	base->fade = 1.0f;
	int i;
	for (i = 0; i < base->game.num_players; i++)
	{
		bg3_spawn_player(base, i);
	}

	xSoundPlay(base->resources.wind_sound);

	bg3_game* g = &base->game;

	bg3_menu menu;
	menu.screen = 0;
	menu.selection = 0;

	dust_time = 0.0f;
	lod_time = LOD_DELAY;
	float update_time = 0.0f;
	while(xRunning() && base->state == BG3_STATE_GAME)
	{
		xTimeUpdate();
		float dt = xTimeGetDeltaTime();
		//printf("dt: %f\n", dt);
		update_time += dt;
		float frame_time = 1.0f/BG3_GAME_FPS;
		int frames = 0;
		while (update_time >= frame_time)
		{
			xCtrlUpdate(frame_time);
			if (g->paused)
			{
				if (base->transition != BG3_FADE_OUT)
				{
					bg3_game_menu_input(base, &menu);
				}
			}
			else
			{
				g->time_elapsed += frame_time;
				if (g->time_elapsed >= (float)bg3_game_times[g->values.game_time_id])
				{
					g->game_over = 1;
				}
				dust_time += frame_time;
				lod_time += frame_time;
				bg3_game_reset_players_frame(base);
				bg3_game_player_input(base, &menu, frame_time);
				if (!g->game_over)
				{
					bg3_game_process_ai(base, frame_time);
				}
				bg3_game_update_players(base, frame_time);
				while (dust_time > 0.0f)
				{
					dust_time -= DUST_WAIT;
				}
			}
			update_time -= frame_time;
			frames += 1;
			if (frames >= BG3_MIN_FPS)
			{
				update_time = x_modf(update_time, BG3_MIN_FPS*frame_time);
				break;
			}
		}
		//printf("frames: %i\n", frames);
		//printf("update time: %f\n", update_time);
		bg3_game_update_render(base, &menu, dt);
	}

	xSoundSetStateAll(X_SOUND_STOP, 0);

	bg3_base_free_effects(base);
	bg3_base_free_resources(base);
	bg3_base_free_game(base);
}
