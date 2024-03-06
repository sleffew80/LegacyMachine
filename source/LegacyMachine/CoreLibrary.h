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
	void (*retro_set_controller_port_device)(unsigned port, unsigned device);
	void (*retro_reset)(void);
	void (*retro_run)(void);
	bool (*retro_load_game)(const struct retro_game_info* game);
	void (*retro_unload_game)(void);
	unsigned poll_type;
	bool initialized;
	bool running;
	bool supports_no_game;
}
CoreLibrary;

#endif