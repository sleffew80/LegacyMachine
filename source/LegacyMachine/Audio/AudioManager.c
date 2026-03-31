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

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <string.h>

#include "AudioManager.h"
#include "../MainEngine.h"

/**************************************************************************************************
 * AudioManager Context
 *************************************************************************************************/

static AudioManager audio_manager = { 0 };

/**************************************************************************************************
 * AudioDriver Context Array
 *************************************************************************************************/

const AudioDriver* audio_drivers[] = {
#if defined HAVE_SDL
	&sdl_audio_driver,
#endif
	NULL
};

/**************************************************************************************************
 * AudioDriver Context
 *************************************************************************************************/

static AudioDriver* audio_driver = NULL;

/**************************************************************************************************
 * AudioDriver Functions
 *************************************************************************************************/

/* Verify that AudioDriverID enum values match their corresponding indices in the audio_drivers
   array. */
static void AssertAudioDriverEnumValues(void)
{
	for (unsigned i = 0; i < MAX_AUDIO_DRIVERS; ++i)
	{
		retro_assert((AudioDriverID)i == audio_drivers[i]->id);
	}
}

/* Initialize the audio driver. */
static AudioDriver* InitializeAudioDriver(AudioDriverID driver_id)
{
#if defined _DEBUG
	/* Verify AudioDriverID values when debugging. */
	AssertAudioDriverEnumValues();
#endif
	return (AudioDriver*)audio_drivers[driver_id];
}

/* Get the current audio driver context. */
static AudioDriver* GetAudioDriverContext(void)
{
	return audio_driver;
}

/**************************************************************************************************
 * AudioManager Functions
 *************************************************************************************************/

/* Get the current audio manager context. */
AudioManager* GetAudioManagerContext(void)
{
	return &audio_manager;
}

/* Get the current audio driver ID. */
AudioDriverID GetAudioDriverID(void)
{
	AudioDriver* driver = GetAudioDriverContext();
	return driver ? driver->id : AUDIO_DRIVER_NONE;
}

/* Initialize the audio manager and audio driver with given frequency. */
bool InitializeAudio(AudioDriverID driver_id, int frequency)
{
	AudioManager* audio = GetAudioManagerContext();
	AudioDriver* driver = InitializeAudioDriver(driver_id);

	if (audio->initialized)
		DeinitializeAudio();

	if (!driver || !driver->cb_init)
		return false;

	audio_driver = driver;

	audio->output.frequency = frequency;

	audio->initialized = driver->cb_init(frequency);

	return audio->initialized;
}

/* Reinitialize audio driver (used when changing latency settings). */
void ReinitializeAudio(void)
{
	AudioManager* audio = GetAudioManagerContext();
	AudioDriver* driver = GetAudioDriverContext();
	if (audio->initialized && driver && driver->cb_init)
	{
		driver->cb_init(audio->output.frequency);
	}

	audio->reinitialize = false;
}

/* Deinitialize the audio manager and its driver. */
void DeinitializeAudio(void)
{
	AudioManager* audio = GetAudioManagerContext();
	AudioDriver* driver = GetAudioDriverContext();

	if (driver && driver->cb_deinit)
	{
		driver->cb_deinit();
	}

	audio_driver = NULL;

	memset(audio, 0, sizeof(AudioManager));
}

/* Write a single audio sample (left and right channels). */
void WriteAudioSample(int16_t left, int16_t right)
{
	AudioDriver* driver = GetAudioDriverContext();
	AudioManager* audio = GetAudioManagerContext();

	retro_assert(driver);
	retro_assert(audio->initialized);

	int16_t buffer[2] = { left, right };

	driver->cb_write(buffer, 1);
}

/* Write a batch of audio samples. */
size_t WriteAudioSampleBatch(const int16_t* data, size_t frames)
{
	AudioDriver* driver = GetAudioDriverContext();
	AudioManager* audio = GetAudioManagerContext();

	retro_assert(driver);
	retro_assert(audio->initialized);

	return driver->cb_write(data, frames);
}

/* Pause audio playback. */
void PauseAudio(void)
{
	AudioDriver* driver = GetAudioDriverContext();

	if (driver && driver->cb_pause)
		driver->cb_pause();
}

/* Resume audio playback. */
void ResumeAudio(void)
{
	AudioDriver* driver = GetAudioDriverContext();

	if (driver && driver->cb_resume)
		driver->cb_resume();
}

/* Check if audio is currently running. */
bool IsAudioRunning(void)
{
	AudioDriver* driver = GetAudioDriverContext();

	if (!driver || !driver->cb_running)
		return false;

	return driver->cb_running();
}

/* Get the current audio driver ID. */
AudioDriverID GetCurrentAudioDriverID(void)
{
	AudioDriver* driver = GetAudioDriverContext();

	if (!driver)
		return AUDIO_DRIVER_NONE;

	return driver->id;
}

/* Set audio frequency in hertz. */
void SetAudioFrequency(unsigned frequency)
{
	AudioManager* audio = GetAudioManagerContext();

	audio->output.frequency = frequency;

	/* Mark audio driver for reinitialization. */
	if (audio->initialized)
		audio->reinitialize = true;
}

/* Set audio latency in milliseconds. */
void SetAudioLatency(unsigned latency)
{
	AudioManager* audio = GetAudioManagerContext();

	audio->output.latency = latency;

	/* Mark audio driver for reinitialization. */
	if (audio->initialized)
		audio->reinitialize = true;
}
