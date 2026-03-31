/*
* LegacyMachine - A Libretro implementation for creating simple lo-fi
* frontends intended to simulate the look and feel of the classic
* video gaming consoles, computers, and arcade machines being emulated.
*
* Copyright (C) 2022-2026 Steven Leffew
* All rights reserved
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
* */

#ifndef _AUDIO_MANAGER_H
#define _AUDIO_MANAGER_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include "LegacyMachine.h"
#include "../Common/Common.h"

/**************************************************************************************************
 * Audio Definitions
 *************************************************************************************************/

#define AUDIO_DRIVER_FIRST (AudioDriverID)0

/**************************************************************************************************
 * AudioDriver Enumeration
 *************************************************************************************************/

typedef enum
{
#if defined HAVE_SDL
	AUDIO_DRIVER_SDL,
#endif
	AUDIO_DRIVER_NONE,
	MAX_AUDIO_DRIVERS = AUDIO_DRIVER_NONE
}
AudioDriverID;

/**************************************************************************************************
 * AudioDriver Structure
 *************************************************************************************************/

typedef struct AudioDriver
{
	bool			(*cb_init)(int);						/* Initialize audio driver */
	size_t			(*cb_write)(const int16_t*, unsigned);	/* Write audio data */
	void			(*cb_pause)(void);						/* Pause audio playback */
	void			(*cb_resume)(void);						/* Resume audio playback */
	bool			(*cb_running)(void);					/* Check if audio is running */
	void			(*cb_deinit)(void);						/* Deinitialize audio driver */
	AudioDriverID	id;										/* Audio driver ID */
}
AudioDriver;

/**************************************************************************************************
 * AudioManager Structure
 *************************************************************************************************/

typedef struct AudioManager
{
	AudioInfo		output;			/* Current audio output settings. */
	bool			reinitialize;	/* Flag to reinitialize audio. */
	bool			initialized;	/* Flag to check if audio is initialized. */
}
AudioManager;

/**************************************************************************************************
 * AudioDriver Array
 *************************************************************************************************/

extern const AudioDriver* audio_drivers[];

/**************************************************************************************************
 * AudioDriver Contexts
 *************************************************************************************************/

extern AudioDriver sdl_audio_driver;

/**************************************************************************************************
 * Audio Prototypes
 *************************************************************************************************/

RETRO_BEGIN_DECLS

/* Audio Management Prototypes */

AudioManager* GetAudioManagerContext(void);
AudioDriverID GetAudioDriverID(void);

bool InitializeAudio(AudioDriverID driver_id, int frequency);
void ReinitializeAudio(void);
void DeinitializeAudio(void);

void WriteAudioSample(int16_t left, int16_t right);
size_t WriteAudioSampleBatch(const int16_t* data, size_t frames);

void PauseAudio(void);
void ResumeAudio(void);
bool IsAudioRunning(void);

AudioDriverID GetCurrentAudioDriverID(void);

void SetAudioLatency(unsigned latency);

RETRO_END_DECLS

#endif