#ifndef __X_WAV_H__
#define __X_WAV_H__

#include "xconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/* play states */
#define X_WAV_STOP 0
#define X_WAV_PLAY 1
#define X_WAV_PAUSE 2

/* loop modes */
#define X_WAV_NO_LOOP 0
#define X_WAV_LOOP 1

/* pan modes */
#define X_WAV_SEPARATE 0
#define X_WAV_COMBINED 1

typedef struct xWav {
	u32 channels;
	u16 samplesize;
	u32 samplerate;
	u32 samplecount;
	void* data;
} xWav;

typedef struct xWav3dListener {
	xVector3f right;
	xVector3f pos;
	xVector3f vel;
} xWav3dListener;

typedef struct xWav3dEmitter {
	xVector3f pos;
	xVector3f vel;
	float radius;
} xWav3dEmitter;

int xWavInit(int num_sounds);

void xWavEnd();

void xWavGlobalVolume(float volume);

xWav* xWavLoad(char* filename);

void xWavFree(xWav* w);

/* play a wav file, returns slot used */
int xWavPlay(xWav* w);

/* set play state - X_WAV_STOP, X_WAV_PLAY, or X_WAV_PAUSE */
int xWavSetState(int slot, int state);

/* set state for all slots, does not effect stopped slots */
int xWavSetStateAll(int state);

/* set loop - X_WAV_NO_LOOP or X_WAV_LOOP */
void xWavSetLoop(int slot, int loop);

/* set pan mode - X_WAV_SEPARATE or X_WAV_COMBINED */
void xWavSetPanMode(int slot, int panmode);

/* volume should be a float from 0.0f (silent) to 1.0f (full volume) */
void xWavSetVolume(int slot, float volume);

/* pan should be a float from -1.0f (fully left) to 1.0f (fully right) */
void xWavSetPan(int slot, float pan);

/* pitch should be a float >= 0.0f, 1.0f = normal */
void xWavSetPitch(int slot, float pitch);

void xWav3dSetConstant(float speedOfSound);

int xWav3dPlay(xWav* w, xWav3dListener* l, xWav3dEmitter* e);

void xWav3dUpdateSounds();

#ifdef __cplusplus
}
#endif

#endif
