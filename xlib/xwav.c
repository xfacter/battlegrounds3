#include <pspaudiolib.h>
#include <pspaudio.h>
#include "xmem.h"
#include "xmath.h"

#include "xwav.h"

#ifdef X_DEBUG
#include "xlog.h"
#define X_LOG(format, ... ) xLogPrintf("xWav: " format, __VA_ARGS__)
#else
#define X_LOG(format, ... ) do{}while(0)
#endif


typedef struct xWavSound {
	u32 playptr;
	u32 playptr_fraction;
	u32 rateratio;
	u16 playstate;
	u16 loop;
	u16 panmode;
	float pan;
	float volume;
	xWav* wav;
	xWav3dListener* listener;
	xWav3dEmitter* emitter;
} xWavSound;

static int x_wav_num_sounds = 0;
static xWavSound* x_wav_sounds = 0;

static float x_wav3d_speed_of_sound = 100.0f;

static void xWavSound_callback(void *buf, unsigned int reqn, void *pdata)
{
	if (!x_wav_sounds) return;
    s16* buffer = buf;
	int n;
	for (n = 0; n < reqn; n++)
	{
		int out_right = 0;
		int out_left = 0;
		int i;
		for (i = 0; i < x_wav_num_sounds; i++)
		{
			xWavSound* s = &x_wav_sounds[i];

			if (!s->wav || s->playstate != X_WAV_PLAY) continue;
			if (!s->wav->data) continue;
            
			u32 fraction = s->playptr_fraction + s->rateratio;
			s->playptr += fraction >> 16;
			s->playptr_fraction = fraction & 0xffff;
			if (s->volume <= 0.0f) continue;
			if (s->playptr >= s->wav->samplecount)
			{
				if (s->loop == X_WAV_LOOP)
				{
					s->playptr = 0;
					s->playptr_fraction = 0;
				}
				else
				{
					xWavSetState(i, X_WAV_STOP);
					continue;
				}
			}

			int index = s->wav->channels * s->playptr;
			if (s->wav->samplesize == 1)
			{
				s8* data8 = (s8*)s->wav->data;
				if (s->wav->channels == 1)
				{
					out_left  += (int)(data8[index]*256 * s->volume * (1.0f - s->pan));
					out_right += (int)(data8[index]*256 * s->volume * s->pan);
				}
				else
				{
					if (s->panmode == X_WAV_COMBINED)
					{
						out_left  += (int)((data8[index+0] + data8[index+1])*256*0.5f * s->volume * (1.0f - s->pan));
						out_right += (int)((data8[index+0] + data8[index+1])*256*0.5f * s->volume * s->pan);
					}
					else
					{
						out_left  += (int)(data8[index+0]*256 * s->volume * (1.0f - s->pan));
						out_right += (int)(data8[index+1]*256 * s->volume * s->pan);
					}
				}
			}
			else
			{
				s16* data16 = (s16*)s->wav->data;
				if (s->wav->channels == 1)
				{
					out_left  += (int)(data16[index] * s->volume * (1.0f - s->pan));
					out_right += (int)(data16[index] * s->volume * s->pan);
				}
				else
				{
					if (s->panmode == X_WAV_COMBINED)
					{
						out_left  += (int)((data16[index+0] + data16[index+1])*0.5f * s->volume * (1.0f - s->pan));
						out_right += (int)((data16[index+0] + data16[index+1])*0.5f * s->volume * s->pan);
					}
					else
					{
						out_left  += (int)(data16[index+0] * s->volume * (1.0f - s->pan));
						out_right += (int)(data16[index+1] * s->volume * s->pan);
					}
				}
			}
		}
		if (out_left < -32768) out_left = -32768;
		else if (out_left > 32767) out_left = 32767;
		if (out_right < -32768) out_right = -32768;
		else if (out_right > 32767) out_right = 32767;
		*(buffer++) = (s16)out_left;
		*(buffer++) = (s16)out_right;
	}
}

int xWavInit(int num_sounds)
{
	if (x_wav_sounds || num_sounds > 64 || num_sounds <= 0) return 1;
	x_wav_sounds = x_malloc(num_sounds*sizeof(xWavSound));
	if (!x_wav_sounds) return 1;
	x_wav_num_sounds = num_sounds;
	xWavSetStateAll(X_WAV_STOP);
    pspAudioInit();
    pspAudioSetChannelCallback(X_WAV_CHANNEL, xWavSound_callback, 0);
    pspAudioSetVolume(X_WAV_CHANNEL, PSP_VOLUME_MAX, PSP_VOLUME_MAX);
	return 0;
}

void xWavEnd()
{
	if (!x_wav_sounds) return;
	x_free(x_wav_sounds);
	x_wav_sounds = 0;
	x_wav_num_sounds = 0;
    pspAudioEnd();
}

void xWavGlobalVolume(float volume)
{
	if (volume < 0.0f) volume = 0.0f;
	if (volume > 1.0f) volume = 1.0f;
	int vol = (int)(volume*PSP_VOLUME_MAX);
	pspAudioSetVolume(X_WAV_CHANNEL, vol, vol);
}

typedef struct {
	u32 ChunkID;
	u32 ChunkSize;
	u32 Format;
} riff_chunk;

typedef struct {
	u32 Subchunk1ID;
	u32 Subchunk1Size;
	u16 AudioFormat;
	u16 NumChannels;
	u32 SampleRate;
	u32 ByteRate;
	u16 BlockAlign;
	u16 BitsPerSample;
} wave_chunk;

typedef struct {
	u32 Subchunk2ID;
	u32 Subchunk2Size;
} data_chunk;

#define WAV_HEAD_RIFF   ('R'<<0|'I'<<8|'F'<<16|'F'<<24) /* "RIFF" (0x46464952) */
#define WAV_HEAD_FORMAT ('W'<<0|'A'<<8|'V'<<16|'E'<<24) /* "WAVE" (0x45564157) */
#define WAV_HEAD_SUB1ID ('f'<<0|'m'<<8|'t'<<16|' '<<24) /* "fmt " (0x20746d66) */
#define WAV_HEAD_SUB2ID ('d'<<0|'a'<<8|'t'<<16|'a'<<24) /* "data" (0x61746164) */

#define WAV_PCM (1)

xWav* xWavLoad(char* filename)
{
	xWav* w = x_malloc(sizeof(xWav));
	if (!w) return 0;
	w->data = 0;

    FILE* file = fopen(filename, "rb");
    if (!file)
	{
		xWavFree(w);
		return 0;
	}

	riff_chunk riff_c;
	fread(&riff_c, sizeof(riff_chunk), 1, file);

	wave_chunk wave_c;
	fread(&wave_c, sizeof(wave_chunk), 1, file);

	data_chunk data_c;
	fseek(file, wave_c.Subchunk1Size-16, SEEK_CUR);
	fread(&data_c, sizeof(data_chunk), 1, file);

	X_LOG("Loading WAV: %s, ChunkID: 0x%08x, ChunkSize: %u, Format: 0x%08x, Subchunk1ID: 0x%08x, Subchunk1Size: %u\r\n \
		  AudioFormat: %u, NumChannels: %u, SampleRate: %u, ByteRate: %u, BlockAlign: %u, BitsPerSample: %u, Subchunk2ID: 0x%08x, Subchunk2Size: %u",
		  filename, riff_c.ChunkID, riff_c.ChunkSize, riff_c.Format, wave_c.Subchunk1ID, wave_c.Subchunk1Size,
		  wave_c.AudioFormat, wave_c.NumChannels, wave_c.SampleRate, wave_c.ByteRate, wave_c.BlockAlign, wave_c.BitsPerSample, data_c.Subchunk2ID, data_c.Subchunk2Size);
    
    if (riff_c.ChunkID != WAV_HEAD_RIFF || riff_c.Format != WAV_HEAD_FORMAT ||
        wave_c.AudioFormat != WAV_PCM || wave_c.Subchunk1ID != WAV_HEAD_SUB1ID ||
        data_c.Subchunk2ID != WAV_HEAD_SUB2ID ||
		(wave_c.NumChannels != 1 && wave_c.NumChannels != 2) ||
		(wave_c.BitsPerSample != 8 && wave_c.BitsPerSample != 16))
	{
		X_LOG("WAV file not compatible with loader.", 0);
		fclose(file);
		xWavFree(w);
        return 0;
    }

	u32 datalength = data_c.Subchunk2Size;
    
	w->channels = wave_c.NumChannels;
	w->samplesize = wave_c.BitsPerSample/8;
    w->samplerate = wave_c.SampleRate;
    w->samplecount = datalength/(w->channels*w->samplesize);
    
    w->data = x_malloc(datalength);
    if (!w->data)
	{
		fclose(file);
		xWavFree(w);
		return 0;
	}
    fread(w->data, datalength, 1, file);
    fclose(file);
    
    return w;
}

void xWavFree(xWav* w)
{
    if (!w) return;
    if (w->data) x_free(w->data);
	x_free(w);
}

int xWavPlay(xWav* w)
{
    if (!w || !x_wav_sounds) return -1;
	if (!w->data) return -1;
	int i;
	for (i = 0; i < x_wav_num_sounds; i++)
	{
		if (x_wav_sounds[i].playstate == X_WAV_STOP)
		{
			xWavSound* s = &x_wav_sounds[i];
			s->playptr = 0;
			s->playptr_fraction = 0;
			s->rateratio = (w->samplerate*0x4000)/11025;
			s->loop = X_WAV_NO_LOOP;
			s->panmode = X_WAV_SEPARATE;
			s->pan = 0.5f;
			s->volume = 1.0f;
			s->wav = w;
			s->listener = 0;
			s->emitter = 0;
			s->playstate = X_WAV_PLAY;
			return i;
		}
	}
	return -1;
}

int xWavSetState(int slot, int state)
{
	if (!x_wav_sounds || slot < 0 || slot >= x_wav_num_sounds) return 1;
	if (state == X_WAV_STOP || x_wav_sounds[slot].playstate != X_WAV_STOP)
	{
		x_wav_sounds[slot].playstate = state;
	}
	return 0;
}

int xWavSetStateAll(int state)
{
	if (!x_wav_sounds) return 0;
	int changed = 0;
	int i;
	for (i = 0; i < x_wav_num_sounds; i++)
	{
		if (xWavSetState(i, state) == 0)
		{
			changed += 1;
		}
	}
	return changed;
}

void xWavSetLoop(int slot, int loop)
{
	if (!x_wav_sounds || slot < 0 || slot >= x_wav_num_sounds) return;
	x_wav_sounds[slot].loop = loop;
}

void xWavSetPanMode(int slot, int panmode)
{
	if (!x_wav_sounds || slot < 0 || slot >= x_wav_num_sounds) return;
	x_wav_sounds[slot].panmode = panmode;
}

void xWavSetVolume(int slot, float volume)
{
	if (!x_wav_sounds || slot < 0 || slot >= x_wav_num_sounds) return;
	if (volume < 0.0f) volume = 0.0f;
	if (volume > 1.0f) volume = 1.0f;
	x_wav_sounds[slot].volume = volume;
}

void xWavSetPan(int slot, float pan)
{
	if (!x_wav_sounds || slot < 0 || slot >= x_wav_num_sounds) return;
	if (pan < -1.0f) pan = -1.0f;
	if (pan >  1.0f) pan =  1.0f;
	pan = (pan + 1.0f)*0.5f;
	x_wav_sounds[slot].pan = pan;
}

void xWavSetPitch(int slot, float pitch)
{
	if (!x_wav_sounds || slot < 0 || slot >= x_wav_num_sounds) return;
	if (pitch < 0.0f) pitch = 0.0f;
	xWav* w = x_wav_sounds[slot].wav;
	u32 rateratio = (u32)(pitch*((w->samplerate*0x4000)/11025));
	/*
	if (rateratio < (2000*0x4000)/11025) rateratio = (2000*0x4000)/11025;
	if (rateratio > (100000*0x4000)/11025) rateratio = (100000*0x4000)/11025;
	*/
	x_wav_sounds[slot].rateratio = rateratio;
}

void xWav3dSetConstant(float speedOfSound)
{
	x_wav3d_speed_of_sound = speedOfSound;
}

static void x_wav_update_sound(int slot)
{
	if (!x_wav_sounds || slot < 0 || slot >= x_wav_num_sounds) return;
	xWavSound* s = &x_wav_sounds[slot];
	if (s->listener && s->emitter)
	{
		xVector3f dir;
		xVec3Sub(&dir, &s->emitter->pos, &s->listener->pos);
		float dist = xVec3Length(&dir);
		if (dist >= s->emitter->radius)
		{
			xWavSetVolume(slot, 0.0f);
		}
		else
		{
			//volume
			xWavSetVolume(slot, 1.0f - dist/s->emitter->radius);
			//pan
			xVec3Normalize(&dir, &dir);
			xWavSetPan(slot, xVec3Dot(&dir, &s->listener->right));
			//pitch
			xVector3f rel_vel;
			xVec3Sub(&rel_vel, &s->emitter->vel, &s->listener->vel);
			if (rel_vel.x == 0.0f && rel_vel.y == 0.0f && rel_vel.z == 0.0f)
			{
				xWavSetPitch(slot, 1.0f);
			}
			else
			{
				/*
				xVector3f n_rel_vel;
				xVec3Normalize(&n_rel_vel, &rel_vel);
				xWavSetPitch(slot, x_wav3d_speed_of_sound / (x_wav3d_speed_of_sound + xVec3Length(&rel_vel)*xVec3Dot(&dir, &n_rel_vel)));
				*/
				xWavSetPitch(slot, x_wav3d_speed_of_sound / (x_wav3d_speed_of_sound + xVec3Dot(&dir, &rel_vel)));
			}
		}
	}
}

int xWav3dPlay(xWav* w, xWav3dListener* l, xWav3dEmitter* e)
{
	if (!x_wav_sounds || !w || !l || !e) return -1;
	if (!w->data) return -1;
	int i;
	for (i = 0; i < x_wav_num_sounds; i++)
	{
		if (x_wav_sounds[i].playstate == X_WAV_STOP)
		{
			xWavSound* s = &x_wav_sounds[i];
			s->playptr = 0;
			s->playptr_fraction = 0;
			s->rateratio = (w->samplerate*0x4000)/11025;
			s->loop = X_WAV_NO_LOOP;
			s->panmode = X_WAV_SEPARATE;
			s->pan = 0.5f;
			s->volume = 1.0f;
			s->wav = w;
			s->listener = l;
			s->emitter = e;
			x_wav_update_sound(i);
			s->playstate = X_WAV_PLAY;
			return i;
		}
	}
	return -1;
}

void xWav3dUpdateSounds()
{
	if (!x_wav_sounds) return;
	int i;
	for (i = 0; i < x_wav_num_sounds; i++)
	{
		if (x_wav_sounds[i].playstate == X_WAV_PLAY)
		{
			x_wav_update_sound(i);
		}
	}
}
