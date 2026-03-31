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
#include <SDL.h>

#include <retro_math.h>

#include "../Common/SDL_Common.h"
#include "../AudioManager.h"
#include "../../MainEngine.h"
#include "../../Logging.h"

/**************************************************************************************************
 * SDL Audio Info Context
 *************************************************************************************************/

static SDL_AudioInfo* sdl_audio_info = NULL;

/**************************************************************************************************
 * SDL Audio Internal Functions
 *************************************************************************************************/

static uint16_t CalculateSamples(int frequency, unsigned latency)
{
	/* Calculate the number of audio samples based on frequency and latency. */
	int samples = (frequency * latency) / 1000;

	/* Return the next power of two for the calculated samples. */
	return (uint16_t)next_pow2(samples);
}

/**************************************************************************************************
 * SDL Audio Functions
 *************************************************************************************************/

/* Initialize audio device. */
static bool SDL_DriverInitializeAudio(int frequency)
{
	LMC_Engine legacy_machine = LMC_GetContext();
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;
	uint32_t subsystem_flags = SDL_WasInit(0);
	uint16_t desired_samples = 0;
	unsigned obtained_latency = 0;
	unsigned sample_check = 0;
	unsigned latency_check = 0;

	/* Close existing device if already initialized (Reinitializing) */
	if (sdl_audio_info)
	{
		lmc_trace(LMC_LOG_VERBOSE, "[Audio] [SDL]: Reinitializing audio...");
		SDL_CloseAudioDevice(sdl_audio_info->device);
		free(sdl_audio_info);
		sdl_audio_info = NULL;
	}
	else
	{
		lmc_trace(LMC_LOG_VERBOSE, "[Audio] [SDL]: Initializing audio...");
	}

	/* Initialize audio subsystem, if necessary. */
	if (subsystem_flags == 0)
	{
		if (SDL_Init(SDL_INIT_AUDIO) < 0)
		{
			lmc_trace(LMC_LOG_ERRORS, "[Audio] [SDL]: Failed to initialize audio subsystem: %s", SDL_GetError());
			LMC_SetLastError(LMC_ERR_FAIL_AUDIO_INIT);
			return false;
		}
	}
	else if ((subsystem_flags & SDL_INIT_AUDIO) == 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
		{
			lmc_trace(LMC_LOG_ERRORS, "[Audio] [SDL]: Failed to initialize audio subsystem: %s", SDL_GetError());
			LMC_SetLastError(LMC_ERR_FAIL_AUDIO_INIT);
			return false;
		}
	}

	SDL_zero(desired);
	SDL_zero(obtained);

	desired_samples = CalculateSamples(frequency, legacy_machine->audio->output.latency);

	/* SDL prefers power‑of‑two sample counts, which often results in a buffer size that produces
	   a latency higher than what is being supplied by LegacyMachine. The following checks should
	   reduce the chance of the desired samples being high enough to double the requested latency. */
	sample_check = desired_samples / 2;
	latency_check = (unsigned)round((double)(sample_check * 1000) / (double)frequency);

	if (latency_check >= legacy_machine->audio->output.latency)
		desired_samples = sample_check;

	/* Set desired audio specs. */
	desired.format = AUDIO_S16;
	desired.freq = frequency;
	desired.channels = 2;
	desired.samples = desired_samples;

	lmc_trace(LMC_LOG_VERBOSE, "[Audio] [SDL]: Requested audio specifications:");
	if (legacy_machine->log_level == LMC_LOG_VERBOSE)
	{
		printf("\tFormat: %u bit\n", SDL_AUDIO_BITSIZE(desired.format));
		printf("\tChannels: %u\n", desired.channels);
		printf("\tFrequency: %d Hz\n", desired.freq);
		printf("\tLatency: %u ms\n", legacy_machine->audio->output.latency);
		printf("\tBuffer Size: %u samples\n", desired.samples);
	}

	sdl_audio_info = (SDL_AudioInfo*)calloc(1, sizeof(SDL_AudioInfo));

	if (!sdl_audio_info)
	{
		SDL_AudioQuit();
		lmc_trace(LMC_LOG_ERRORS, "[Audio] [SDL]: Failed to allocate memory for audio info context");
		LMC_SetLastError(LMC_ERR_OUT_OF_MEMORY);
		return false;
	}

	/* Open an audio device requesting desired specs. */
	sdl_audio_info->device = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
	if (!sdl_audio_info->device)
	{
		free(sdl_audio_info);
		sdl_audio_info = NULL;
		SDL_AudioQuit();
		lmc_trace(LMC_LOG_ERRORS, "[Audio] [SDL]: Failed to open playback device: %s", SDL_GetError());
		LMC_SetLastError(LMC_ERR_FAIL_AUDIO_INIT);

		return false;
	}

	sdl_audio_info->channels = obtained.channels;
	sdl_audio_info->frequency = obtained.freq;
	sdl_audio_info->samples = obtained.samples;

	/* Get obtained latency in milliseconds (buffer size only) */
	obtained_latency = (unsigned)round((double)(obtained.samples * 1000) / (double)obtained.freq);

	lmc_trace(LMC_LOG_VERBOSE, "[Audio] [SDL]: Obtained audio specifications:");
	if (legacy_machine->log_level == LMC_LOG_VERBOSE)
	{
		printf("\tFormat: %u bit\n", SDL_AUDIO_BITSIZE(obtained.format));
		printf("\tChannels: %u\n", obtained.channels);
		printf("\tFrequency: %d Hz\n", obtained.freq);
		printf("\tLatency: %u ms\n", obtained_latency);
		printf("\tBuffer Size: %u samples\n", obtained.samples);
	}

	/* Update actual latency in the audio manager. */
	legacy_machine->audio->output.latency = obtained_latency;

	/* Start audio playback. */
	SDL_PauseAudioDevice(sdl_audio_info->device, 0);

	/* Let the core know that the audio device has been initialized. */
	if (legacy_machine->system->cb_audio.set_state)
	{
		legacy_machine->system->cb_audio.set_state(true);
	}

	lmc_trace(LMC_LOG_VERBOSE, "[Audio] [SDL]: Audio initialized successfully");
	LMC_SetLastError(LMC_ERR_OK);

	return true;
}

/* Close audio device. */
static void SDL_DriverCloseAudio(void)
{
	lmc_trace(LMC_LOG_VERBOSE, "[Audio] [SDL]: Closing audio...");

	if (sdl_audio_info)
	{
		SDL_CloseAudioDevice(sdl_audio_info->device);
		free(sdl_audio_info);
		sdl_audio_info = NULL;
	}
}

/* Write audio to the audio device. */
static size_t SDL_DriverWriteAudio(const int16_t* buffer, unsigned frames)
{
	retro_assert(sdl_audio_info);

	SDL_QueueAudio(sdl_audio_info->device, buffer, sizeof(*buffer) * frames * 2);
	return frames;
}

/* Pause audio playback. */
static void SDL_DriverPauseAudio(void)
{
	retro_assert(sdl_audio_info);

	sdl_audio_info->paused = true;
	SDL_PauseAudioDevice(sdl_audio_info->device, 1);
}

/* Resume audio playback. */
static void SDL_DriverResumeAudio(void)
{
	retro_assert(sdl_audio_info);

	sdl_audio_info->paused = false;
	SDL_PauseAudioDevice(sdl_audio_info->device, 0);
}

/* Check if audio is currently running (not paused). */
static bool SDL_DriverIsAudioRunning(void)
{
	retro_assert(sdl_audio_info);

	return !sdl_audio_info->paused;
}

/**************************************************************************************************
 * SDL Audio Driver
 *************************************************************************************************/

AudioDriver sdl_audio_driver = {
	SDL_DriverInitializeAudio,	/* cb_init */
	SDL_DriverWriteAudio,		/* cb_write */
	SDL_DriverPauseAudio,		/* cb_pause */
	SDL_DriverResumeAudio,		/* cb_resume */
	SDL_DriverIsAudioRunning,	/* cb_running */
	SDL_DriverCloseAudio,		/* cb_deinit */
	AUDIO_DRIVER_SDL
};
