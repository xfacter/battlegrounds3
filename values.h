#ifndef __VALUES_H__
#define __VALUES_H__

#define TANK_RADIUS 0.75f

#define SHIELD_SCALE 1.5f
#define SHIELD_RECHARGE_PER_SEC 0.5f
#define SHIELD_FADE_TIME 1.0f
#define SHIELD_RECHARGE_WAIT 5.0f
#define POWERUP_EFFECT_TIME 0.5f

#define RESPAWN_WAIT 10.0f
#define DAMAGE_POINT1 0.5f
#define DMG_SMOKE_WAIT 0.1f
#define DMG_SPARKS_WAIT 2.5f
#define MISSILE_SMOKE_WAIT 0.05f

#define MGUN_FLASH_TIME 0.1f
#define MGUN_WARMUP_TIME 1.0f
#define MGUN_COOLDOWN_TIME 2.0f
#define MGUN_SPIN_SPEED 2*2*M_PI
#define MISSILE_VELOCITY 25.0f
#define MISSILE_SEEK_RADIUS 10.0f
#define MISSILE_SEEK_ANGLE M_PI/4
#define MISSILE_DMG_INNER_RADIUS 2.5f
#define MISSILE_DMG_OUTER_RADIUS 5.0f
#define TSHELL_DMG_INNER_RADIUS 2.5f
#define TSHELL_DMG_OUTER_RADIUS 5.0f

#define MGUN_WAIT		1/8.0f
#define TSHELL_WAIT		1.0f
//#define GAUSS_WAIT	1.0f
#define MISSILE_WAIT	1.0f

#define MGUN_SHIELD_DPS		0.35f
#define MGUN_ARMOR_DPS		0.35f
#define LASER_SHIELD_DPS	0.60f
#define LASER_ARMOR_DPS		0.15f
#define TSHELL_SHIELD_DPS	0.15f
#define TSHELL_ARMOR_DPS	0.60f
//#define GAUSS_SHIELD_DPS	0.40f
//#define GAUSS_ARMOR_DPS	0.40f
#define MISSILE_SHIELD_DPS	0.60f
#define MISSILE_ARMOR_DPS	0.60f

#define ARMOR_POWERUP_AMT	0.5f
#define LASER_POWERUP_AMT	10.0f
#define TSHELL_POWERUP_AMT	10
#define MISSILE_POWERUP_AMT	10

#define LASER_MAX_AMMO 20.0f
#define TSHELL_MAX_AMMO 20
#define MISSILE_MAX_AMMO 20

#define SOUND_RADIUS 50.0f

enum bg3_primary {
	BG3_MACHINE_GUN = 0,
	BG3_LASER,
	BG3_TANK_SHELL,
	//BG3_GAUSS,
	BG3_MISSILES,
	BG3_NUM_PRIMARY
};

enum bg3_state {
	BG3_MENU = 0,
	BG3_GAME_DM,
	//BG3_GAME_TDM,
	//BG3_GAME_CTF,
	BG3_EXIT
};

enum bg3_ai_neutral_state {
	BG3_AI_NEUTRAL_WANDER = 0,
	BG3_AI_NEUTRAL_SEEK_PASSIVE,
	BG3_AI_NEUTRAL_SEEK_ACTIVE,
	BG3_AI_NEUTRAL_SCAN,
	BG3_AI_NEUTRAL_STATES
};

enum bg3_ai_hostile_state {
	BG3_AI_HOSTILE_ATTACK = BG3_AI_NEUTRAL_STATES,
	BG3_AI_HOSTILE_FLEE,
	BG3_AI_HOSTILE_STATES
};

enum bg3_powerup_type {
	BG3_POWERUP_ARMOR = 0,
	BG3_POWERUP_LASER,
	BG3_POWERUP_TSHELL,
	BG3_POWERUP_MISSILE
};

#endif