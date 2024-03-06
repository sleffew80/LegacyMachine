/*
* LegacyMachine - A libRetro implementation for creating simple lo-fi
* frontends intended to simulate the look and feel of the classic
* video gaming consoles, computers, and arcade machines being emulated.
*
* Copyright (C) 2022-2024 Steven Leffew
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

#include "../AudioDriver.h"
#include "../../MainEngine.h"
#include "../../Logging.h"

/**************************************************************************************************
 * SDL2 Audio Variables
 *************************************************************************************************/

static SDL_AudioDeviceID  audio_device_id = 0;

/**************************************************************************************************
 * SDL2 Audio Functions
 *************************************************************************************************/

/* Initialize audio device. */
static void SDL2_InitializeAudio(int frequency) {
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	SDL_zero(desired);
	SDL_zero(obtained);

	/* Set desired audio specs. */
	desired.format = AUDIO_S16;
	desired.freq = frequency;
	desired.channels = 2;
	desired.samples = 4096;

	/* Open an audio device requesting desired specs. */
	audio_device_id = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
	if (!audio_device_id)
	{
		lmc_trace(LMC_LOG_ERRORS, "Failed to open playback device: %s", SDL_GetError());
		LMC_SetLastError(LMC_ERR_FAIL_AUDIO_INIT);
	}

	/* Pause audio device until it is requested. */
	SDL_PauseAudioDevice(audio_device_id, 0);

	/* Let the core know that the audio device has been initialized. */
	if (legacy_machine->system->cb_audio.set_state) {
		legacy_machine->system->cb_audio.set_state(true);
	}

	legacy_machine->audio->initialized = true;
}

/* Close audio device. */
static void SDL2_CloseAudio(void) {
	SDL_CloseAudioDevice(audio_device_id);
}

/* Write audio to the audio device. */
static size_t SDL2_WriteAudio(const int16_t* buf, unsigned frames) {
	SDL_QueueAudio(audio_device_id, buf, sizeof(*buf) * frames * 2);
	return frames;
}

/**************************************************************************************************
 * SDL2 Audio Driver
 *************************************************************************************************/

AudioDriver sdl2_audio_driver = {
	SDL2_InitializeAudio,
	SDL2_WriteAudio,
	SDL2_CloseAudio,
	false
};