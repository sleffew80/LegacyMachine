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

#ifndef _CORE_LIBRARY_H
#define _CORE_LIBRARY_H

/**************************************************************************************************
 * Includes
 *************************************************************************************************/
#include <dynamic/dylib.h>

#include "LegacyMachine.h"

/**************************************************************************************************
 * CoreLibrary Structure
 *************************************************************************************************/

typedef struct
{
	dylib_t handle;

	void (*retro_init)(void);
	void (*retro_deinit)(void);
	unsigned (*retro_api_version)(void);
	void (*retro_get_system_info)(struct retro_system_info* info);
	void (*retro_get_system_av_info)(struct retro_system_av_info* info);
	void (*retro_set_environment)(retro_environment_t environment);
	void (*retro_set_video_refresh)(retro_video_refresh_t refresh);
	void (*retro_set_audio_sample)(retro_audio_sample_t sample);
	void (*retro_set_audio_sample_batch)(retro_audio_sample_batch_t sample_batch);
	void (*retro_set_input_poll)(retro_input_poll_t poll);
	void (*retro_set_input_state)(retro_input_state_t state);
	void (*retro_set_controller_port_device)(unsigned port, unsigned device);
	void (*retro_reset)(void);
	void (*retro_run)(void);
	size_t(*retro_serialize_size)(void);
	bool (*retro_serialize)(void* data, size_t size);
	bool (*retro_unserialize)(const void* data, size_t size);
	void (*retro_cheat_reset)(void);
	void (*retro_cheat_set)(unsigned index, bool enabled, const char* code);
	bool (*retro_load_game)(const struct retro_game_info* game);
	bool (*retro_load_game_special)(unsigned game_type, const struct retro_game_info* info, size_t num_info);
	void (*retro_unload_game)(void);
	unsigned (*retro_get_region)(void);
	void* (*retro_get_memory_data)(unsigned id);
	size_t(*retro_get_memory_size)(unsigned id);

	bool supports_no_game;
	bool initialized;
}
CoreLibrary;

#endif